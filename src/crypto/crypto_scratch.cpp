// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file crypto_scratch.cpp
 * @brief Definition of the shared crypto scratch buffer (see crypto_scratch.h). The secure wipe is a
 *        static inline in the header, so a translation unit that only wipes needs no link against this file;
 *        only users of crypto_work do.
 */

#include "crypto/crypto_scratch.h"

alignas(16) uint8_t crypto_work[DWS_CRYPTO_WORK_SIZE];
