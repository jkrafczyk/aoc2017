#pragma once
#include <cstdint>
#include <iostream>
#include <string>

namespace day25 {
    /**
     * Represents one line of input in a program.
     * \ingroup parsing
     */
struct Token {
    enum Type {
        //! The name of the turing machines starting state.
        INITIAL_STATE,
        //! The number of steps to execute before calculating the checksum.
        CHECKSUM_DELAY,
        //! Begin of a block describing a possible machine state.
        STATE_DECLARATION,
        //! Begin of a block describing what to do in a state for a specific value in the current tape slot.
        STATE_REQUIREMENT,
        //! Instruction to write a value to the tape.
        STATE_WRITE,
        //! Instruction to move to a different slot on the tape.
        STATE_MOVEMENT,
        //! Instruction to move to a different state.
        STATE_NEXT,
        //! The line was non-empty, but invalid.
        ERROR,
        //! End of file was encountered.
        END_OF_STREAM,
    } type;
    //! Line number where this token was encountered
    uint16_t line_number;
    //! Original text encountered in the source file
    std::string raw_text;
    //! Value of the token. e.g. for INITIAL_STATE the name of the turing machines start state.
    std::string arg;
};

/**
 * Splits the lines of a file into \ref Token objects.
 * \ingroup parsing
 */
class Tokenizer {
  public:
    Tokenizer(std::istream &source);

    //! Read one token and return it.
    const Token &next();

    /** Return the last read token.
    * \warning Result is undefined if \ref next() has not been called at least once before.
    */
    const Token &current() const;

  private:
    std::istream &m_source;
    uint16_t m_line_number;
    Token m_current;
};
} // namespace day25