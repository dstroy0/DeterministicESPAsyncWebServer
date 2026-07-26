// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file esp_crypto_hal.h
 * @brief Single owner of the ESP32 RSA/MPI accelerator's MODMULT, behind a variant-stable API.
 *
 * The library's own big-field crypto (the GF(2^255-19) layer in @ref fe25519.h for X25519 / Ed25519, and the
 * NIST P-256 field/scalar layer in @ref ecdsa.cpp) drives one primitive on the RSA accelerator: a single-shot
 * 256-bit **modular multiply** `Z = X*Y mod M`. That is the ONLY place we touch the accelerator's registers.
 *
 * We support a growing set of dies (S3, P4, and more to come), and Espressif renames these registers between
 * silicon generations - the same peripheral is "hw_ver1" on the S3 (`RSA_MOD_MULT_START_REG`,
 * `RSA_QUERY_INTERRUPT_REG`, `RSA_MEM_*_BLOCK_BASE`, ...) and "hw_ver3" on the P4 and newer
 * (`RSA_SET_START_MODMULT_REG`, `RSA_QUERY_IDLE_REG`, `RSA_*_MEM`, ...). Rather than scatter those names (and
 * that per-die `#if`) across every crypto file - where a vendor header change would break the crypto in many
 * places - this HAL is the **one** file that names a vendor register. Consumers call @ref dws_rsa_modmul and
 * never see a register or a `soc/` macro, so an upstream rename touches only the shim below (and fails loud via
 * the `#error`, never silently). The MODMULT sequence and the Montgomery convention are identical across dies;
 * only the register names and the done-bit's name (INTERRUPT vs IDLE - both "poll until it reads 1") differ.
 * Reference: ESP-IDF `components/mbedtls/port/bignum/bignum_alt.c` `esp_mpi_mul_mpi_mod_hw_op` + `hal/mpi_hal.c`.
 *
 * `DWS_RSA_MODMUL_HW` is defined when the target die has a single-shot MODMULT. The classic ESP32 does not (it
 * needs two MULT passes), so it - and native / host builds - leave it undefined and the callers fall back to
 * their software field layer. Header-only `static inline`: the modmul is the expensive op, so the call is free
 * relative to it, and the whole HAL stays one source of truth.
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_CRYPTO_ESP_CRYPTO_HAL_H
#define DETERMINISTICESPASYNCWEBSERVER_CRYPTO_ESP_CRYPTO_HAL_H

#include <stdint.h>

#ifdef ARDUINO
#include "sdkconfig.h" // CONFIG_IDF_TARGET_* selects the die
#endif

// Enabled per-variant on every die with a single-shot hardware MODMULT. Add a die here once its rig has passed
// the on-device RFC KAT (fe25519/ecdsa via main_cryptobench). Classic ESP32 has no single-shot MODMULT. This
// die detection MUST precede the soc-register include below so the header pulls in its own vendor dependency
// (rather than relying on some other TU including soc/hwcrypto_reg.h first).
#if defined(ARDUINO) && ((defined(CONFIG_IDF_TARGET_ESP32S3) && CONFIG_IDF_TARGET_ESP32S3) ||                          \
                         (defined(CONFIG_IDF_TARGET_ESP32P4) && CONFIG_IDF_TARGET_ESP32P4))
#define DWS_RSA_MODMUL_HW 1
#endif
#if defined(DWS_RSA_MODMUL_HW)
#include "soc/hwcrypto_reg.h" // RSA/MPI accelerator register map (the ONLY vendor register dependency)
#include "soc/soc.h"          // DR_REG_RSA_BASE
#endif

#ifdef DWS_RSA_MODMUL_HW

extern "C"
{
    void esp_mpi_enable_hardware_hw_op(void);  // mbedTLS port: acquire the MPI lock + clock/power the peripheral
    void esp_mpi_disable_hardware_hw_op(void); // release the lock + power down (also waits for its memory-init)
}

#define DWS_RSA_REG(a) (*(volatile uint32_t *)(a))

// --- the ONE per-silicon register shim (see the file comment) --------------------------------------------
#if defined(RSA_SET_START_MODMULT_REG) // hw_ver3: ESP32-P4 and newer
#define DWS_RSA_MEM_M RSA_M_MEM
#define DWS_RSA_MEM_X RSA_X_MEM
#define DWS_RSA_MEM_Y RSA_Y_MEM
#define DWS_RSA_MEM_Z RSA_Z_MEM
#define DWS_RSA_MODE RSA_MODE_REG      // operand length in words, minus 1
#define DWS_RSA_MPRIME RSA_M_PRIME_REG // Montgomery m' (mod 2^32)
#define DWS_RSA_START RSA_SET_START_MODMULT_REG
#define DWS_RSA_DONE RSA_QUERY_IDLE_REG // reads 1 once the accelerator is idle (op complete)
#define DWS_RSA_INTCLR RSA_INT_CLR_REG
#define DWS_RSA_INTENA RSA_INT_ENA_REG
#elif defined(RSA_MOD_MULT_START_REG) // hw_ver1: ESP32-S3 / S2
#define DWS_RSA_MEM_M RSA_MEM_M_BLOCK_BASE
#define DWS_RSA_MEM_X RSA_MEM_X_BLOCK_BASE
#define DWS_RSA_MEM_Y RSA_MEM_Y_BLOCK_BASE
#define DWS_RSA_MEM_Z RSA_MEM_Z_BLOCK_BASE
#define DWS_RSA_MODE RSA_LENGTH_REG
#define DWS_RSA_MPRIME RSA_M_DASH_REG
#define DWS_RSA_START RSA_MOD_MULT_START_REG
#define DWS_RSA_DONE RSA_QUERY_INTERRUPT_REG // reads 1 once the op raises its completion bit
#define DWS_RSA_INTCLR RSA_CLEAR_INTERRUPT_REG
#define DWS_RSA_INTENA RSA_INTERRUPT_REG
#else
#error "esp_crypto_hal: DWS_RSA_MODMUL_HW is on but no known RSA MODMULT register set - add this die's flavor"
#endif

/**
 * @brief Acquire the RSA accelerator for a run of modular multiplies (lock + clock/power), poll-only.
 * @note  Bracket every batch of @ref dws_rsa_modmul with acquire/release; the lock is shared with mbedTLS
 *        RSA/DH, so a scalar-mult holds it for its whole run rather than toggling per multiply.
 */
static inline void dws_rsa_hw_acquire(void)
{
    esp_mpi_enable_hardware_hw_op(); // lock + clock/power the peripheral (waits for its memory-init)
    DWS_RSA_REG(DWS_RSA_INTENA) = 0; // poll only, no completion IRQ
}

/** @brief Release the RSA accelerator (drop the lock, power down). */
static inline void dws_rsa_hw_release(void)
{
    esp_mpi_disable_hardware_hw_op();
}

/**
 * @brief `z = x*y mod m` (@p words limbs) on the RSA accelerator. Requires @ref dws_rsa_hw_acquire first.
 * @param z      result, @p words little-endian limbs (safe to alias @p x or @p y).
 * @param x,y    operands, canonical (< m).
 * @param m      the modulus (canonical @p words limbs).
 * @param mprime Montgomery m' = -m^-1 mod 2^32.
 * @param rinv   R^2 mod m; preloaded into the result block so MODMULT returns the plain residue x*y mod m
 *               rather than a Montgomery form (the esp_mpi_mul_mpi_mod convention). Output is canonical (< m).
 */
static inline void dws_rsa_modmul(uint32_t *z, const uint32_t *x, const uint32_t *y, const uint32_t *m, uint32_t mprime,
                                  const uint32_t *rinv, unsigned words)
{
    volatile uint32_t *M = (volatile uint32_t *)DWS_RSA_MEM_M;
    volatile uint32_t *X = (volatile uint32_t *)DWS_RSA_MEM_X;
    volatile uint32_t *Y = (volatile uint32_t *)DWS_RSA_MEM_Y;
    volatile uint32_t *Z = (volatile uint32_t *)DWS_RSA_MEM_Z;
    DWS_RSA_REG(DWS_RSA_MODE) = words - 1; // mode = words - 1
    DWS_RSA_REG(DWS_RSA_MPRIME) = mprime;
    for (unsigned i = 0; i < words; i++)
    {
        M[i] = m[i];
        X[i] = x[i];
        Y[i] = y[i];
        Z[i] = rinv[i]; // R^2 mod m in the result block -> plain (non-Montgomery) output
    }
    DWS_RSA_REG(DWS_RSA_INTCLR) = 1; // clear any stale done flag before starting
    DWS_RSA_REG(DWS_RSA_START) = 1;
    while (DWS_RSA_REG(DWS_RSA_DONE) == 0) // wait until the done/idle bit reads 1
        ;
    DWS_RSA_REG(DWS_RSA_INTCLR) = 1;
    for (unsigned i = 0; i < words; i++)
        z[i] = Z[i];
}

#endif // DWS_RSA_MODMUL_HW
#endif // DETERMINISTICESPASYNCWEBSERVER_CRYPTO_ESP_CRYPTO_HAL_H
