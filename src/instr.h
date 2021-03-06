/**
 * This file is part of Rellume.
 *
 * (c) 2020, Alexis Engelke <alexis.engelke@googlemail.com>
 *
 * Rellume is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License (LGPL)
 * as published by the Free Software Foundation, either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Rellume is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Rellume.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file
 **/

#ifndef RELLUME_INSTR_H
#define RELLUME_INSTR_H

#include <fadec.h>

#include <cstdbool>
#include <cstdint>


namespace rellume {

class Instr : public FdInstr {
public:
    using Type = FdInstrType;

    struct Reg {
        uint16_t rt;
        uint16_t ri;
        Reg(unsigned rt, unsigned ri)
            : rt(static_cast<uint16_t>(rt)), ri(static_cast<uint16_t>(ri)) {}
        explicit operator bool() const { return ri != FD_REG_NONE; }
    };

    class Op {
        const Instr* inst;
        unsigned idx;
    public:
        constexpr Op(const Instr* inst, unsigned idx) : inst(inst), idx(idx) {}
        explicit operator bool() const {
            return idx < 4 && FD_OP_TYPE(inst, idx) != FD_OT_NONE;
        }
        unsigned size() const { return FD_OP_SIZE(inst, idx); }
        unsigned bits() const { return size() * 8; }

        bool is_reg() const { return FD_OP_TYPE(inst, idx) == FD_OT_REG; }
        const Reg reg() const {
            assert(is_reg());
            return Reg(FD_OP_REG_TYPE(inst, idx), FD_OP_REG(inst, idx));
        }

        bool is_imm() const { return FD_OP_TYPE(inst, idx) == FD_OT_IMM; }
        int64_t imm() const { assert(is_imm()); return FD_OP_IMM(inst, idx); }

        bool is_pcrel() const { return FD_OP_TYPE(inst, idx) == FD_OT_OFF; }
        int64_t pcrel() const { assert(is_pcrel()); return FD_OP_IMM(inst, idx); }

        bool is_mem() const { return FD_OP_TYPE(inst, idx) == FD_OT_MEM; }
        const Reg base() const {
            assert(is_mem());
            return Reg(FD_RT_GPL, FD_OP_BASE(inst, idx));
        }
        const Reg index() const {
            assert(is_mem());
            return Reg(FD_RT_GPL, FD_OP_INDEX(inst, idx));
        }
        unsigned scale() const {
            assert(is_mem());
            if (FD_OP_INDEX(inst, idx) != FD_REG_NONE)
                return 1 << FD_OP_SCALE(inst, idx);
            return 0;
        }
        int64_t off() const { assert(is_mem()); return FD_OP_DISP(inst, idx); }
        unsigned seg() const { assert(is_mem()); return FD_SEGMENT(inst); }
        unsigned addrsz() const { assert(is_mem()); return inst->addrsz(); }
    };

    size_t len() const { return FD_SIZE(fdi()); }
    uintptr_t start() const { return FD_ADDRESS(fdi()); }
    uintptr_t end() const { return start() + len(); }
    Type type() const { return FD_TYPE(fdi()); }
    unsigned addrsz() const { return FD_ADDRSIZE(fdi()); }
    unsigned opsz() const { return FD_OPSIZE(fdi()); }
    const Op op(unsigned idx) const { return Op{this, idx}; }
    bool has_rep() const { return FD_HAS_REP(fdi()); }
    bool has_repnz() const { return FD_HAS_REPNZ(fdi()); }

private:
    const FdInstr* fdi() const {return static_cast<const FdInstr*>(this); }
};

} // namespace

#endif
