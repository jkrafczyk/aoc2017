#pragma once
#include "program.hpp"
#include "tokenizer.hpp"

namespace day25 {
struct ParserState {
    bool error;
    bool eof;
    std::string error_message;
    Token token;

    operator bool() const { return !eof && !error; }
};

class Parser {
  public:
    Parser(Tokenizer &source);

    const ParserState &parse();

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