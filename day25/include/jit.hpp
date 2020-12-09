#pragma once
#include <iostream>
#include <list>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <functional>

namespace day25 {
    //! Names a general-purpose register.
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
};

//! Condition codes for Jcc instructions.
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

/** Describes some forms of indirect addressing like in `lea [rax]`.
 *
 * Only `[register]` and `[register+register]` variants are supported.
 */
struct Indirect {
    Indirect(Register r, Register offset) : reg(r), offset_reg(offset) {}
    Indirect(Register r) : reg(r), offset_reg(Register::NONE) {}

    const Register reg;
    const Register offset_reg;
};

/** Represents a reference to a named symbol (i.e. a memory address).
 */
struct Symbol {
    Symbol(const std::string &name)
        : name(name), offset(-1), address(nullptr) {}
    Symbol(const std::string &name, uint32_t offset, void *address)
        : name(name), offset(offset), address(address) {}
    const std::string name;
    const int32_t offset;
    const void *address;
};

/**
 * Code generator for just-in-time compilation.
 *
 * Use the `emit` methods to write instructions into a memory buffer, then call \ref finalize_code,
 * then use the `call` methods to call functions within the generated code.
 *
 * Example:
 * \code
 * Jit jit;
 * jit.emit_symbol("my-function");
 * jit.emit_mov(Register::RAX, 42);
 * jit.emit_ret();
 * jit.finalize_code();
 * int fourty_two = jit.call("my-function", 0);
 * assert(fourty_two == 42);
 * \endcode
 */
class Jit {
  public:
    Jit();
    ~Jit();

    // Requirements to actually execute code:
    //! Mark the code buffer as executable and resolve all symbols used in instructions.
    void finalize_code();
    //! Call the function at offset 0 in the buffer, passing `arg` as the first argument.
    uint64_t call(uint64_t arg);
    //! Call the function at symbol with name `entrypoint`, passing `arg` as the first argument.
    uint64_t call(const std::string &entrypoint, uint64_t arg);

    // Mark locations with symbols that can be referred to by other ops:
    //! Mark the current location in the buffer with the name `name`.
    void emit_symbol(const std::string &name);
    //! Mark the location `location` with the name `name`. `location` may refer to memory outside of the buffer.
    void emit_symbol(const std::string &name, void *location);

    // Mark the current location to be replaced by a symbol pointer later on:
    //! Replace the uint64 starting at the current offset with the address of the symbol `name` during finalization.
    void emit_symbol_ref(const std::string &name);
    /** Mark the next `ref_length` bytes as a relative reference to the symbol `name`.
     *
     * During finalization, the bytes starting at this location will be replaced by the offset between here and the referred symbol.
     * This can be used for RIP-relative addressing.
     */
    void emit_symbol_relative_ref(const std::string &name, uint8_t ref_length);

    // Generic byte-sequence builders
    //! Copy `length` bytes from `bytes` into the current buffer location.
    void emit(uint32_t length, const void *bytes);
    /** Write any primitive value into the current buffer location.
     * \warning
     * Ensure to call this with the correct type when trying to write single bytes!
     **/
    template <class T> void emit(const T &v) { emit(sizeof(T), &v); }

    // Generate common instructions:
    //! Write a `ret` (return) instruction.
    void emit_ret();
    //! Write a `mov r64, imm64` instruction (e.g. `mov rax, 0xdeadc0de`)
    void emit_mov(Register reg, uint64_t val);
    //! Write a `mov r64, r64` instruction (e.g. `mov rax, r15`)
    void emit_mov(Register dest, Register src);
    //! Write a `mov r64, r/m64` instruction (e.g. `mov rax, [rbx+rcx])
    void emit_mov(Register dest, Indirect src);
    //! Write a `mov r/m64, r64` instruction (e.g. `mov [rbx+rcx], rax`)
    void emit_mov(Indirect dest, Register src);
    //! Write a `mov r64, imm64` instruction loading the address of symbol (e.g. `mov rax, printf`)
    void emit_mov(Register reg, Symbol val);

    void emit_push(Register reg);
    void emit_pop(Register reg);

    void emit_inc(Register reg);
    void emit_dec(Register reg);

    //! Write a RIP-relative near jump with fixed 32 bit displacement.
    void emit_jmp(int32_t displacement);
    //! Write a RIP-relative near jump to the address of a symbol.
    void emit_jmp(Symbol target);
    //! Write a near jump to the 64-bit address in the given register. (e.g. `jmp [rax]`)
    void emit_jmp(Register reg);
    //! Write a conditional RIP-relative near jump with fixed 32 bit displacement.
    void emit_jcc(Condition condition, int32_t displacement);
    //! Write a conditional RIP-relative near jump to the address of a symbol.
    void emit_jcc(Condition condition, Symbol target);

    //! Write a lea instruction in the form `lea reg, [reg1]` or `lea reg, [reg1+reg2]`
    void emit_lea(Register reg, Indirect addr);
    //! Write a RIP-relative lea instruction with 32bit displacement.
    void emit_lea(Register reg, int32_t displacement);
    //! Write a RIP-relative lea instruction referring to the address of a symbol.
    void emit_lea(Register reg, Symbol target);

    void emit_add(Register target, int8_t imm_addend);
    void emit_add(Register target, int32_t imm_addend);
    void emit_add(Register target, Register addend);
    void emit_add(Register target, Symbol addend);

    void emit_sub(Register target, int8_t imm_subtrahend);
    void emit_sub(Register target, int32_t imm_subtrahend);
    void emit_sub(Register target, Register subtrahend);
    void emit_sub(Register target, Symbol subtrahend);

    void emit_mul(Register arg);
    void emit_div(Register arg);

    void emit_cmp(Register r1, int8_t imm);
    void emit_cmp(Register r1, int32_t imm);
    void emit_cmp(Register r1, Register r2);
    void emit_cmp(Register r1, Symbol s);

    void emit_call(Register target);
    void emit_call(Indirect target);
    void emit_call(Symbol target);

    //Abstract / high-level stuff:
    typedef std::function<void(Jit *jit, const std::string &name, const std::string &return_label)> FunctionDefiner;

    /**
     * Write a function in SysV AMD64 ABI.
     *
     * The \ref Jit will:
     * * write a suitable function prelude
     * * define a symbol for the function name,
     * * call the \ref FunctionDefiner to emit the function body
     * * write the necessary instructions to tear down the stack frame.
     */
    void emit_function(const std::string &name, uint8_t n_locals, FunctionDefiner contents);

    /**
     * Write a function call in SysV AMD64 ABI.
     *
     * Called functions need to accept either no arugments, or up to 6 int64 arguments.
     *
     * Example:
     * \code
     * //Make the exit() function available to the jit:
     * jit.emit_symbol("exit", (void*)exit);
     * //Call exit(23) to terminate the process:
     * jit.emit_function_call(
     *   jit.symbol("exit"),  //name of the function to call
     *   1,                   //argument count
     *   23                   //first argument
     * );
     * \endcode
     */
    void emit_function_call(Symbol function_name, uint64_t argc, ...);

    //Non-executable memory management:
    void add_constant(const std::string &name, const std::string &value);
    void add_buffer(const std::string &name, uint64_t size);

    // Utility and inspection:
    std::vector<uint8_t> dump_memory() const;

    //! Return an object that can be used to refer to symbols.
    Symbol symbol(const std::string &name) const;

  private:
    struct SymbolRef;
    struct Buffer;
    uint32_t m_code_size;
    uint32_t m_offset;
    bool m_code_finalized;
    uint8_t *m_code;
    std::map<std::string, Buffer> m_buffers;
    std::map<std::string, void *> m_symbols;
    std::list<SymbolRef> m_symbol_refs;

    uint64_t call(void *location, uint64_t arg);
};

struct Jit::SymbolRef {
    const bool absolute;
    const uint32_t offset;
    const std::string symbol;
    const uint8_t replacement_length;
};

struct Jit::Buffer {
    std::string name;
    uint64_t size;
    uint8_t *address;
};
} // namespace day25