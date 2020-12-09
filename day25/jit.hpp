#pragma once
#include <iostream>
#include <list>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace day25 {
enum class Register : uint8_t {
    RAX = 0,
    RCX,
    RDX,
    RBX,
    RSP,
    RBP,
    RSI,
    RDI,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
    NONE = 0xff,
    // Don't support R08-R15.
};

// Condition codes for Jcc instructions.
enum class Condition : uint8_t {
    ABOVE = 0x87,
    ABOVE_EQUAL = 0x83,
    BELOW = 0x82,
    BELOW_EQUAL = 0x86,
    CARRY = 0x82,
    EQUAL = 0x84,
    GREATER = 0x8f,
    GREATER_EQUAL = 0x8d,
    LESS = 0x8c,
    LESS_EQUAL = 0x8e,
    NOT_ABOVE = 0x86,
    NOT_ABOVE_EQUAL = 0x82,
    NOT_BELOW = 0x83,
    NOT_BELOW_EQUAL = 0x87,
    NOT_CARRY = 0x83,
    NOT_EQUAL = 0x85,
    NOT_GREATER = 0x8e,
    NOT_GREATER_EQUAL = 0x8c,
    NOT_LESS = 0x8d,
    NOT_LESS_EQUAL = 0x8f,
    NOT_OVERFLOW = 0x81,
    NOT_PARITY = 0x8b,
    NOT_SIGN = 0x89,
    NOT_ZERO = 0x85,
    OVERFLOW = 0x80,
    PARITY = 0x8a,
    PARITY_EVEN = 0x8a,
    PARITY_ODD = 0x8b,
    SIGN = 0x88,
    ZERO = 0x84
};

struct Indirect {
    Indirect(Register r, Register offset) : reg(r), offset_reg(offset) {}
    Indirect(Register r) : reg(r), offset_reg(Register::NONE) {}

    const Register reg;
    const Register offset_reg;
};

struct Symbol {
    Symbol(const std::string &name)
        : name(name), offset(-1), address(nullptr) {}
    Symbol(const std::string &name, uint32_t offset, void *address)
        : name(name), offset(offset), address(address) {}
    const std::string name;
    const int32_t offset;
    const void *address;
};

class Jit {
  public:
    Jit();
    ~Jit();

    // Requirements to actually execute code:
    void finalize_code();
    uint64_t call(uint64_t arg);
    uint64_t call(const std::string &entrypoint, uint64_t arg);

    // Mark locations with symbols that can be referred to by other ops:
    void emit_symbol(const std::string &name);
    void emit_symbol(const std::string &name, void *location);

    // Mark the current location to be replaced by a symbol pointer later on:
    void emit_symbol_ref(const std::string &name);
    void emit_symbol_relative_ref(const std::string &name, uint8_t ref_length);

    // Generic byte-sequence builders
    void emit(uint32_t length, const void *bytes);
    template <class T> void emit(const T &v) { emit(sizeof(T), &v); }

    // Generate common instructions:
    void emit_ret();
    void emit_mov(Register reg, uint64_t val);
    void emit_mov(Register dest, Register src);
    void emit_mov(Register dest, Indirect src);
    void emit_mov(Indirect dest, Register src);
    void emit_mov(Register reg, Symbol val);

    void emit_push(Register reg);
    void emit_pop(Register reg);

    void emit_inc(Register reg);
    void emit_dec(Register reg);

    void emit_jmp(int32_t displacement);
    void emit_jmp(Symbol target);
    void emit_jmp(Register reg);
    // void emit_jmp(Indirect reg);
    void emit_jcc(Condition condition, int32_t displacement);
    void emit_jcc(Condition condition, Symbol target);

    void emit_lea(Register reg, Indirect addr);
    void emit_lea(Register reg, int32_t displacement);
    void emit_lea(Register reg, Symbol target);

    void emit_add(Register target, int8_t imm_addend);
    void emit_add(Register target, int32_t imm_addend);
    void emit_add(Register target, Register addend);
    void emit_add(Register target, Symbol addend);

    void emit_sub(Register target, int8_t imm_subtrahend);
    void emit_sub(Register target, int32_t imm_subtrahend);
    void emit_sub(Register target, Register subtrahend);
    void emit_sub(Register target, Symbol subtrahend);

    void emit_mul(Register arg); // RDX:RAX = RAX * arg
    void emit_div(Register arg); // RAX = RDX:RAX / arg

    void emit_cmp(Register r1, int8_t imm);
    void emit_cmp(Register r1, int32_t imm);
    void emit_cmp(Register r1, Register r2);
    void emit_cmp(Register r1, Symbol s);

    void emit_call(Register target);
    void emit_call(Indirect target);
    void emit_call(Symbol target);

    //Abstract / high-level stuff:
    typedef std::function<void(Jit *jit, const std::string &name, const std::string &return_label)> FunctionDefiner;
    void emit_function(const std::string &name, uint8_t n_locals, FunctionDefiner contents);
    void emit_function_call(Symbol function_name, uint64_t argc, ...);


    // Utility and inspection:
    std::vector<uint8_t> dump_memory() const;
    Symbol symbol(const std::string &name) const;

  private:
    struct SymbolRef;
    uint32_t m_code_size;
    uint32_t m_offset;
    bool m_code_finalized;
    uint8_t *m_code;
    std::map<std::string, void *> m_symbols;
    std::list<SymbolRef> m_symbol_refs;

    uint64_t call(void *location, uint64_t arg);
};
} // namespace day25