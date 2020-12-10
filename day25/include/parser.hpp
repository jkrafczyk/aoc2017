#pragma once
#include "program.hpp"
#include "tokenizer.hpp"

namespace day25 {
    /**
     * Describes the success or failure of a parsing operation.
     * \ingroup parsing
     */
struct ParserState {
    /**
     * Indicates if an error has occurred.
     * If true, error_message will be set and no attempt should be made to continue processing the input.
     */
    bool error;
    /**
     * Indicates if EOF was encountered, i.e. the file was processed in full.
     */
    bool eof;
    std::string error_message;
    //! Last seen token. This <i>may</i> be related to error_message, but doesn't have to.
    Token token;
};

/**
 * Processes a sequence of \ref Token "Tokens" into a \ref Program.
 * \ingroup parsing
 */
class Parser {
  public:
    Parser(Tokenizer &source);

    //! Read input and return a \ref ParserState indicating either success or failure.
    const ParserState &parse();

    //! Returns the \ref Program generated during \ref parse.
    Program program();

  private:
    Tokenizer &m_source;
    Program m_program;
    ParserState m_last_state;

    const ParserState &parse_state(const Token &state_declaration);

    const ParserState &finalize_program(const Token &eof_token);

    const ParserState &error(const Token &token, const std::string &message);

    const ParserState &ok(const Token &token);

    const ParserState &eof(const Token &token);
};
} // namespace day25