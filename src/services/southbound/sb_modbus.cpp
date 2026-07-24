// Copyright (C) 2026 Douglas Quigg (dstroy0) <dquigg123@gmail.com>
// SPDX-License-Identifier: AGPL-3.0-or-later

/**
 * @file sb_modbus.cpp
 * @brief Modbus-master southbound driver adapter (see sb_modbus.h).
 */

#include "services/southbound/sb_modbus.h"

#if DWS_ENABLE_SOUTHBOUND && DWS_ENABLE_MODBUS_MASTER

#include "services/modbus/modbus_master.h"

namespace
{
// Read a contiguous span of `n` registers (1..125) at `first` in one Modbus request; write the parsed
// values to `out` as int32. Shared by the single-point and block reads. Returns the register count
// (>= 0), a negative transport error (propagated from txn), DWS_SB_MODBUS_EXCEPTION on a Modbus
// exception reply, or Sb::SB_ERR_ARG on a bad argument / malformed reply.
int sb_modbus_read_span(DwsSbModbusCtx *c, uint32_t first, int32_t *out, size_t n)
{
    // A Modbus register address is 16-bit and a single request reads at most 125 registers.
    if (n == 0 || n > 125 || first > 0xFFFFu || first + n > 0x10000u)
        return Sb::SB_ERR_ARG;

    uint8_t req[12];
    size_t rn =
        dws_modbus_build_read((uint8_t)c->fc, c->txid++, c->unit, (uint16_t)first, (uint16_t)n, req, sizeof(req));
    if (rn == 0)
        return Sb::SB_ERR_ARG;

    uint8_t resp[MODBUS_ADU_MAX];
    int pn = c->txn(c->io, req, rn, resp, sizeof(resp));
    if (pn < 0)
        return pn; // transport error, propagated unchanged

    uint16_t regs[125];
    uint8_t ex = 0;
    int got = dws_modbus_parse_response(resp, (size_t)pn, regs, n, &ex);
    if (got < 0)
        return Sb::SB_ERR_ARG; // malformed / short frame
    c->last_exception = ex;
    if (ex)
        return DWS_SB_MODBUS_EXCEPTION;
    for (int i = 0; i < got; i++)
        out[i] = (int32_t)regs[i];
    return got;
}

int sb_modbus_read(void *vctx, uint32_t point, int32_t *value_out)
{
    DwsSbModbusCtx *c = (DwsSbModbusCtx *)vctx;
    int got = sb_modbus_read_span(c, point, value_out, 1);
    if (got < 0)
        return got;
    return (got == 1) ? Sb::SB_OK : Sb::SB_ERR_ARG; // a valid reply always carries the one register
}

int sb_modbus_read_block(void *vctx, uint32_t first, int32_t *out, size_t n)
{
    return sb_modbus_read_span((DwsSbModbusCtx *)vctx, first, out, n);
}
} // namespace

int dws_sb_modbus_init(DwsSbModbusCtx *ctx, DwsSbModbusTxn txn, void *io, ModbusFunction fc, uint8_t unit)
{
    if (!ctx || !txn)
        return Sb::SB_ERR_ARG;
    if (fc != ModbusFunction::MODBUS_FC_READ_HOLDING_REGS && fc != ModbusFunction::MODBUS_FC_READ_INPUT_REGS)
        return Sb::SB_ERR_ARG;
    ctx->txn = txn;
    ctx->io = io;
    ctx->fc = fc;
    ctx->unit = unit;
    ctx->txid = 0;
    ctx->last_exception = 0;
    return Sb::SB_OK;
}

int dws_sb_modbus_driver(SouthboundDriver *drv_out, const char *name, DwsSbModbusCtx *ctx)
{
    if (!drv_out || !name || !ctx || !ctx->txn)
        return Sb::SB_ERR_ARG;
    drv_out->name = name;
    drv_out->read = sb_modbus_read;
    drv_out->write = nullptr; // the master codec is read-only (a register scanner)
    drv_out->read_block = sb_modbus_read_block;
    drv_out->write_block = nullptr;
    drv_out->ctx = ctx;
    return Sb::SB_OK;
}

#endif // DWS_ENABLE_SOUTHBOUND && DWS_ENABLE_MODBUS_MASTER
