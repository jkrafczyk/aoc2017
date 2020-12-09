#include "jit.hpp"
#include <stdexcept>
#include <sys/mman.h>

using std::runtime_error;
using std::vector;

using namespace day25;

struct Jit::SymbolRef {
    const bool absolute;
    const uint32_t offset;
    const std::string symbol;
    const uint8_t replacement_length;
};

namespace {
uint8_t rex(uint8_t w, uint8_t r, uint8_t x, uint8_t b) {
    return (0b0100'0000 | ((w & 1) << 3) | ((r & 1) << 2) | ((x & 1) << 1) |
            ((b & 1)));
}

//    const uint8_t rexw = rex(1,0,0,0);

uint8_t ModR(uint8_t mod, uint8_t rm, Register reg) {
    return (mod & 0b11) << 6 | (rm & 0b111) << 3 | ((uint8_t)reg & 0b111);
}

uint8_t ModR(uint8_t mod, Register rm, Register reg) {
    return ModR(mod, (uint8_t)rm, reg);
}

uint8_t SIB(uint8_t scale, uint8_t index, Register base) {
    return (scale & 0b11) << 6 | (index & 0b111) << 3 | ((uint8_t)base & 0b111);
}

uint8_t SIB(uint8_t scale, Register index, Register base) {
    return SIB(scale, (uint8_t)index, base);
}

uint8_t register_pair(Register reg1, Register reg2) {
    uint8_t result =
        0xc0 | (((uint8_t)reg1 & 0x7) << 3) | (((uint8_t)reg2) & 0x7);
    return result;
}
} // namespace

Jit::Jit() : m_code_size(16384), m_offset(0), m_code_finalized(false) {
    if (sizeof(void *) != 8) {
        throw runtime_error("JIT is only available on 64-bit systems.");
    }
    int prot = PROT_READ | PROT_WRITE;
    int flags = MAP_ANONYMOUS | MAP_PRIVATE;
    m_code = (uint8_t *)mmap(nullptr, m_code_size, prot, flags, -1, 0);
    if (!m_code) {
        throw runtime_error("Could not create code buffer for JIT.");
    }
}

Jit::~Jit() {
    if (m_code) {
        munmap(m_code, m_code_size);
    }
}

void Jit::finalize_code() {
    if (m_code_finalized) {
        return;
    }

    for (auto ref : m_symbol_refs) {
        if (m_symbols.count(ref.symbol) == 0) {
            throw runtime_error("Reference to undefined symbol " + ref.symbol);
        }
        void *ref_addr = m_symbols.at(ref.symbol);
        void *here = m_code + ref.offset;
        int64_t distance =
            (uint8_t *)ref_addr - ref.replacement_length - (uint8_t *)here;
        if (ref.absolute) {
            memcpy(here, &ref_addr, sizeof(void *));
        } else if (!ref.absolute && ref.replacement_length == 1) {
            if (distance < -128 || distance > 127) {
                throw runtime_error(
                    "Relative reference to symbol too far away.");
            }
            int8_t i = distance;
            memcpy(here, &i, 1);
        } else if (!ref.absolute && ref.replacement_length == 2) {
            if (distance < -32768 || distance > 32767) {
                throw runtime_error(
                    "Relative reference to symbol too far away.");
            }
            int16_t i = distance;
            memcpy(here, &i, 2);
        } else if (!ref.absolute && ref.replacement_length == 4) {
            if (distance < -2147483648 || distance > 2147483647) {
                throw runtime_error(
                    "Relative reference to symbol too far away.");
            }
            int32_t i = distance;
            memcpy(here, &i, 4);
        } else {
            throw runtime_error(
                "Invalid symbol reference: Relative reference with "
                "replacement size that is not 1, 2 or 4 bytes.");
        }
    }

    // TODO: Do relocations, fix jumps, calls and moves, etc. based on symbol
    // table.
    if (mprotect(m_code, m_code_size, PROT_READ | PROT_EXEC)) {
        throw runtime_error("Could not mark code as executable.");
    }
    m_code_finalized = true;
}

void Jit::emit_symbol(const std::string &name) {
    emit_symbol(name, m_code + m_offset);
}

void Jit::emit_symbol(const std::string &name, void *location) {
    if (m_symbols.count(name)) {
        throw runtime_error("Re-defined symbol " + name);
    }
    m_symbols[name] = location;
}

void Jit::emit_symbol_ref(const std::string &name) {
    m_symbol_refs.push_back(SymbolRef{.absolute = true,
                                      .offset = m_offset,
                                      .symbol = name,
                                      .replacement_length = sizeof(void *)});
}

void Jit::emit_symbol_relative_ref(const std::string &name,
                                   uint8_t ref_length) {
    m_symbol_refs.push_back(SymbolRef{.absolute = false,
                                      .offset = m_offset,
                                      .symbol = name,
                                      .replacement_length = ref_length});
}

void Jit::emit(uint32_t length, const void *bytes) {
    if (length + m_offset >= m_code_size) {
        throw runtime_error("Generated code too large.");
    }
    memcpy(m_code + m_offset, bytes, length);
    m_offset += length;
}

void Jit::emit_ret() { emit((uint8_t)0xc3); }

void Jit::emit_mov(Register reg, uint64_t val) {
    uint8_t move = 0xb8;
    move |= (uint8_t)reg;
    emit(rex(1, 0, 0, reg >= Register::R8));
    emit(move);
    emit(val);
}

void Jit::emit_mov(Register reg, Symbol val) {
    uint8_t move = 0xb8;
    move |= (uint8_t)reg;
    emit(rex(1, 0, 0, reg >= Register::R8));
    emit(move);
    emit_symbol_ref(val.name);
    emit((uint64_t)0xdeadbeef1badf00d);
}

void Jit::emit_mov(Register dest, Register src) {
    // MOV r/m64,r64
    // Encoding: REX.W + 89 /r
    //'/r' = bits '11', followed by src reg, followed dest reg.
    // TODO: This will explode for R8..R15 in any slot!
    uint8_t reg = 0xc0;
    reg |= ((uint8_t)src) << 3;
    reg |= ((uint8_t)dest);
    emit((uint8_t)0x48);
    emit((uint8_t)0x89);
    emit(reg);
}

void Jit::emit_mov(Register dest, Indirect src) {
    // mov r64, r/m64
    emit(rex(1, 0, 0, dest >= Register::R8));
    emit((uint8_t)0x8b);

    if (src.reg == Register::RBP) {
        throw std::runtime_error("Invalid operand combination.");
    } else if (src.reg == Register::RSP | src.offset_reg != Register::NONE) {
        // Request SIB prefix by using 'RSP' as register:
        emit(ModR(0, dest, Register::RSP));
        if (src.offset_reg == Register::NONE) {
            // Disable offset by using 'RSP'
            emit(SIB(0, Register::RSP, src.reg));
        } else {
            emit(SIB(0, src.offset_reg, src.reg));
        }
    } else {
        uint8_t reg = 0x00;
        reg |= ((uint8_t)src.reg);
        reg |= ((uint8_t)dest) << 3;
        emit(reg);
    }
}

void Jit::emit_mov(Indirect dest, Register src) {
    // mov r/m64, r64
    emit(rex(1, 0, 0, dest.reg >= Register::R8));
    emit((uint8_t)0x89);

    if (dest.reg == Register::RBP) {
        throw std::runtime_error("Invalid operand combination.");
    } else if (dest.reg == Register::RSP | dest.offset_reg != Register::NONE) {
        // Request SIB prefix by using 'RSP' as register:
        emit(ModR(0, src, Register::RSP));
        if (dest.offset_reg == Register::NONE) {
            // Disable offset by using 'RSP'
            emit(SIB(0, Register::RSP, dest.reg));
        } else {
            emit(SIB(0, dest.offset_reg, dest.reg));
        }
    } else {
        uint8_t reg = 0x00;
        reg |= ((uint8_t)dest.reg);
        reg |= ((uint8_t)src) << 3;
        emit(reg);
    }
}

void Jit::emit_push(Register reg) {
    if (reg >= Register::R8) {
        emit(rex(0, 0, 0, 1));
    }
    emit((uint8_t)(0x50 | (((uint8_t)reg) & 0x7)));
}
void Jit::emit_pop(Register reg) {
    if (reg >= Register::R8) {
        emit(rex(0, 0, 0, 1));
    }
    emit((uint8_t)(0x58 | (((uint8_t)reg) & 0x7)));
}

void Jit::emit_inc(Register reg) {
    emit(rex(1, 0, 0, reg >= Register::R8));
    emit((uint8_t)0xff);
    emit((uint8_t)(0xC0 | (uint8_t)reg));
}

void Jit::emit_dec(Register reg) {
    emit(rex(1, 0, 0, reg >= Register::R8));
    emit((uint8_t)0xff);
    emit((uint8_t)(0xC8 | (uint8_t)reg));
}

void Jit::emit_jmp(Register reg) {
    if (reg >= Register::R8) {
        emit(rex(0, 0, 0, 1));
        emit((uint8_t)0xFF);
        uint8_t encoded_reg = 0b0010'0000;
        encoded_reg |= ((uint8_t)reg) & 0x7;
        emit(encoded_reg);
    } else {
        emit((uint8_t)0xFF);
        emit((uint8_t)(0xE0 | (uint8_t)reg));
    }
}

void Jit::emit_jmp(int32_t displacement) {
    emit((uint8_t)0xe9);
    emit(displacement);
}

void Jit::emit_jmp(Symbol target) {
    emit((uint8_t)0xe9);
    emit_symbol_relative_ref(target.name, sizeof(uint32_t));
    emit((uint32_t)0xdeadbeef);
}

void Jit::emit_jcc(Condition condition, int32_t displacement) {
    emit((uint8_t)0x0f);
    emit((uint8_t)condition);
    emit(displacement);
}

void Jit::emit_jcc(Condition condition, Symbol target) {
    emit((uint8_t)0x0f);
    emit((uint8_t)condition);
    emit_symbol_relative_ref(target.name, sizeof(uint32_t));
    emit((uint32_t)0xdeadbeef);
}

void Jit::emit_lea(Register reg, Indirect addr) {
    emit(rex(1, reg >= Register::R8,
             addr.offset_reg >= Register::R8 &&
                 addr.offset_reg <= Register::NONE,
             addr.reg >= Register::R8));
    emit((uint8_t)0x8d);
    emit(ModR(0, reg, Register::RSP));
    auto index =
        (uint8_t)((addr.offset_reg == Register::NONE) ? Register::RSP
                                                      : addr.offset_reg) &
        0x7;

    emit(SIB(0, index, addr.reg));
}

void Jit::emit_lea(Register reg, int32_t displacement) {
    emit(rex(1, 0, 0, reg >= Register::R8));
    emit((uint8_t)0x8d);
    emit(ModR(0, reg, Register::RBP));
    emit(displacement);
}

void Jit::emit_lea(Register reg, Symbol target) {
    emit(rex(1, 0, 0, reg >= Register::R8));
    emit((uint8_t)0x8d);
    emit(ModR(0, reg, Register::RBP));
    emit_symbol_relative_ref(target.name, sizeof(uint32_t));
    emit((uint32_t)0xdeadbeef);
}

void Jit::emit_add(Register target, int8_t imm_addend) {
    emit(rex(1, 0, 0, target >= Register::R8));
    emit((uint8_t)0x83);
    emit(register_pair(Register::RAX, target));
    emit(imm_addend);
}

void Jit::emit_add(Register target, int32_t imm_addend) {
    if (imm_addend >= -128 && imm_addend <= 127) {
        return emit_add(target, (int8_t)imm_addend);
    }
    emit(rex(1, 0, 0, target >= Register::R8));
    emit((uint8_t)0x81);
    emit(register_pair(Register::RAX, target));
    emit(imm_addend);
}

void Jit::emit_add(Register target, Register addend) {
    emit(rex(1, addend >= Register::R8, 0, target >= Register::R8));
    emit((uint8_t)0x01);
    emit(register_pair(addend, target));
}

void Jit::emit_add(Register target, Symbol addend) {
    emit(rex(1, 0, 0, target >= Register::R8));
    emit((uint8_t)0x03);
    emit(ModR(0, target, Register::RBP));
    emit_symbol_relative_ref(addend.name, sizeof(uint32_t));
    emit(0xdeadbeef);
}

void Jit::emit_sub(Register target, int8_t imm_subtrahend) {
    emit(rex(1, 0, 0, target >= Register::R8));
    emit((uint8_t)0x83);
    emit(register_pair(Register::RBP, target));
    emit(imm_subtrahend);
}

void Jit::emit_sub(Register target, int32_t imm_subtrahend) {
    if (imm_subtrahend >= -128 && imm_subtrahend <= 127) {
        return emit_sub(target, (int8_t)imm_subtrahend);
    }
    emit(rex(1, 0, 0, target >= Register::R8));
    emit((uint8_t)0x81);
    emit(register_pair(Register::RBP, target));
    emit(imm_subtrahend);
}

void Jit::emit_sub(Register target, Register subtrahend) {
    emit(rex(1, subtrahend >= Register::R8, 0, target >= Register::R8));
    emit((uint8_t)0x29);
    emit(register_pair(subtrahend, target));
}

void Jit::emit_sub(Register target, Symbol subtrahend) {
    emit(rex(1, 0, 0, target >= Register::R8));
    emit((uint8_t)0x2B);
    emit(ModR(0, target, Register::RBP));
    emit_symbol_relative_ref(subtrahend.name, sizeof(uint32_t));
    emit(0xdeadbeef);
}

void Jit::emit_mul(Register arg) {
    emit(rex(1, 0, 0, arg >= Register::R8));
    emit((uint8_t)0xF7);
    emit(register_pair(Register::RSP, arg));
}

void Jit::emit_div(Register arg) {
    emit(rex(1, 0, 0, arg >= Register::R8));
    emit((uint8_t)0xF7);
    emit(register_pair(Register::RSI, arg));
}

void Jit::emit_cmp(Register r1, int8_t imm) {
    emit(rex(1, 0, 0, r1 >= Register::R8));
    emit((uint8_t)0x83);
    uint8_t reg = 0xf8;
    reg |= ((uint8_t)r1) & 0x7;
    emit(reg);
    emit(imm);
}
void Jit::emit_cmp(Register r1, int32_t imm) {
    if (imm >= -128 && imm <= 127) {
        return emit_cmp(r1, (int8_t)imm);
    }
    emit(rex(1, 0, 0, r1 >= Register::R8));
    emit((uint8_t)0x81);
    uint8_t reg = 0xf8;
    reg |= ((uint8_t)r1) & 0x7;
    emit(reg);
    emit(imm);
}
void Jit::emit_cmp(Register r1, Register r2) {
    emit(rex(1, r2 >= Register::R8, 0, r1 >= Register::R8));
    emit((uint8_t)0x39);
    emit(register_pair(r2, r1));
}
void Jit::emit_cmp(Register r1, Symbol s) {
    emit(rex(1, r1 >= Register::R8, 0, 0));
    emit((uint8_t)0x3B);
    emit(ModR(0, r1, Register::RBP));
    emit_symbol_relative_ref(s.name, sizeof(uint32_t));
    emit(0xdeadbeef);
}

uint64_t Jit::call(uint64_t arg) { return call(m_code, arg); }

uint64_t Jit::call(const std::string &entrypoint, uint64_t arg) {
    return call(m_symbols.at(entrypoint), arg);
}

uint64_t Jit::call(void *location, uint64_t arg) {
    if (location < m_code || location >= (m_code + m_code_size)) {
        throw runtime_error("Trying to call pointer outside of jit code area");
    }
    if (!m_code_finalized) {
        throw runtime_error("Trying to call jit before calling finalize()");
    }

    typedef uint64_t (*func)(uint64_t);
    func f = (func)location;
    return f(arg);
}

vector<uint8_t> Jit::dump_memory() const {
    vector<uint8_t> result;
    result.resize(m_offset);
    for (uint32_t i = 0; i < m_offset; i++) {
        result[i] = m_code[i];
    }
    return result;
}

Symbol Jit::symbol(const std::string &name) const {
    if (m_symbols.count(name) == 0) {
        return Symbol(name);
    }
    void *loc = m_symbols.at(name);
    int32_t offset = 0;
    if (loc >= m_code && loc < m_code + m_code_size) {
        offset = (uint8_t *)loc - m_code;
    } else {
        offset = -1;
    }
    return Symbol(name, offset, loc);
}