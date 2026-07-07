// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file modbus_master.h
 * @brief Modbus TCP master codec + register scanner (DETWS_ENABLE_MODBUS_MASTER).
 *
 * The master/client side of Modbus: build a read-request ADU (MBAP header + PDU)
 * and parse the slave's response into register values, so an application can poll
 * or auto-discover a slave's registers. Pure - no sockets, no heap - so it is
 * host-tested as a full round-trip against the slave codec (modbus_process_adu).
 * The app supplies the transport (send the ADU, receive the reply).
 *
 * Auto-discovery pattern: walk the address space one read at a time; a register
 * exists where the response parses without a Modbus exception.
 *
 * @author  Douglas Quigg (dstroy0)
 * @date    2026
 */

#ifndef DETERMINISTICESPASYNCWEBSERVER_MODBUS_MASTER_H
#define DETERMINISTICESPASYNCWEBSERVER_MODBUS_MASTER_H

#include "ServerConfig.h"
#include <stddef.h>
#include <stdint.h>

#if DETWS_ENABLE_MODBUS_MASTER

/**
 * @brief Build a read-request ADU (FC 0x03 holding or 0x04 input registers).
 *
 * @param fc     MODBUS_FC_READ_HOLDING_REGS (0x03) or MODBUS_FC_READ_INPUT_REGS (0x04).
 * @param txid   transaction id echoed by the slave (caller's correlation token).
 * @param unit   unit / slave id.
 * @param start  first register address.
 * @param count  number of registers (1..125).
 * @param out    destination buffer.
 * @param cap    destination capacity (>= 12).
 * @return bytes written (12), or 0 on a bad argument / too-small buffer.
 */
size_t modbus_build_read(uint8_t fc, uint16_t txid, uint8_t unit, uint16_t start, uint16_t count, uint8_t *out,
                         size_t cap);

/**
 * @brief Parse a read-response ADU into register values.
 *
 * @param adu       response bytes (MBAP + PDU).
 * @param len       response length.
 * @param regs_out  destination for parsed 16-bit register values.
 * @param max_regs  capacity of @p regs_out.
 * @param exception_out  set to the Modbus exception code if the slave returned one
 *                       (then the function returns 0 registers); 0 otherwise.
 * @return number of registers parsed (>= 0), or -1 on a malformed/short frame.
 */
int modbus_parse_response(const uint8_t *adu, size_t len, uint16_t *regs_out, size_t max_regs, uint8_t *exception_out);

#endif // DETWS_ENABLE_MODBUS_MASTER
#endif // DETERMINISTICESPASYNCWEBSERVER_MODBUS_MASTER_H
