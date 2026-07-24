// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file sb_modbus.h
 * @brief Modbus-master southbound driver adapter (DWS_ENABLE_SOUTHBOUND && DWS_ENABLE_MODBUS_MASTER).
 *
 * Binds the transport-agnostic Modbus TCP master codec (services/modbus/modbus_master) into the
 * southbound driver framework (services/southbound), so an app addresses a Modbus slave the same way
 * as any other field device: register the driver, then read *points* (register addresses) by name
 * through the one facade. A point id is a register address; the block (matrix) path is the atomic
 * multi-register read a single Modbus request can satisfy (up to 125 registers).
 *
 * The shipped master codec is read-only (a register scanner: build a read request, parse the reply),
 * so this adapter binds @ref SouthboundDriver::read and @ref SouthboundDriver::read_block; write /
 * write_block stay unbound (the framework reports Sb::SB_ERR_UNSUPPORTED). A future master write
 * builder (FC 0x06 / 0x10) would complete the matrix write path.
 *
 * The app owns the transport: it supplies a @ref DwsSbModbusTxn seam that sends a request ADU and
 * receives the reply (over dws_client for Modbus TCP, or a serial gateway). Pure otherwise - no heap,
 * no sockets, host-testable with a mock transaction routed straight into the slave codec.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_SB_MODBUS_H
#define DETERMINISTICESPASYNCWEBSERVER_SB_MODBUS_H

#include "ServerConfig.h"
#include "services/modbus/modbus.h"         // ModbusFunction, MODBUS_ADU_MAX
#include "services/southbound/southbound.h" // SouthboundDriver, Sb
#include <stddef.h>
#include <stdint.h>

#if DWS_ENABLE_SOUTHBOUND && DWS_ENABLE_MODBUS_MASTER

/**
 * @brief Request/response transport seam.
 *
 * Send the @p req_len request ADU, receive the reply into @p resp (capacity @p resp_cap).
 * @return the reply length in bytes (> 0), or a negative transport error - the negative is propagated
 *         through the SouthboundDriver read call unchanged, so the app can tell a transport failure
 *         from a Modbus-level one (see DWS_SB_MODBUS_EXCEPTION).
 */
typedef int (*DwsSbModbusTxn)(void *io, const uint8_t *req, size_t req_len, uint8_t *resp, size_t resp_cap);

/** @brief A Modbus-level exception reply (not a transport error); the raw code is in ctx->last_exception. */
static constexpr int DWS_SB_MODBUS_EXCEPTION = -100;

/**
 * @brief One Modbus-master southbound driver instance (borrowed by the registry for its lifetime).
 *
 * Fill it with dws_sb_modbus_init(), then build a SouthboundDriver over it with dws_sb_modbus_driver().
 */
struct DwsSbModbusCtx
{
    DwsSbModbusTxn txn;     ///< app transport seam (send request, receive reply).
    void *io;               ///< opaque transport context passed to @ref txn.
    ModbusFunction fc;      ///< MODBUS_FC_READ_HOLDING_REGS (0x03) or MODBUS_FC_READ_INPUT_REGS (0x04).
    uint8_t unit;           ///< Modbus unit / slave id.
    uint16_t txid;          ///< rolling transaction id, incremented per request.
    uint8_t last_exception; ///< raw Modbus exception code from the last read (0 = none).
};

/**
 * @brief Initialize a driver context.
 * @param ctx   the instance to fill.
 * @param txn   the transport seam (must be non-null).
 * @param io    opaque context passed to @p txn on each request (may be null).
 * @param fc    ModbusFunction::MODBUS_FC_READ_HOLDING_REGS or ::MODBUS_FC_READ_INPUT_REGS.
 * @param unit  Modbus unit / slave id.
 * @return Sb::SB_OK, or Sb::SB_ERR_ARG on a null ctx/txn or an fc that is not a read function code.
 */
int dws_sb_modbus_init(DwsSbModbusCtx *ctx, DwsSbModbusTxn txn, void *io, ModbusFunction fc, uint8_t unit);

/**
 * @brief Fill @p drv_out with a SouthboundDriver bound to @p ctx (read + read_block only).
 * @param drv_out  the driver vtable to fill (borrowed by the registry; must outlive it, as must @p ctx).
 * @param name     the driver's unique registry name (borrowed).
 * @param ctx      an initialized context (see dws_sb_modbus_init).
 * @return Sb::SB_OK, or Sb::SB_ERR_ARG on a null / uninitialized argument.
 */
int dws_sb_modbus_driver(SouthboundDriver *drv_out, const char *name, DwsSbModbusCtx *ctx);

#endif // DWS_ENABLE_SOUTHBOUND && DWS_ENABLE_MODBUS_MASTER
#endif // DETERMINISTICESPASYNCWEBSERVER_SB_MODBUS_H
