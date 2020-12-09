#pragma once
#include <cstdint>
#include <iostream>
#include <string>

namespace day25 {
struct Token {
    enum Type {
        INITIAL_STATE,
        CHECKSUM_DELAY,
        STATE_DECLARATION,
        STATE_REQUIREMENT,
        STATE_WRITE,
        STATE_MOVEMENT,
        STATE_NEXT,
        ERROR,
        END_OF_STREAM,
    } type;
    uint16_t line_number;
    std::string raw_text;
    std::string arg;
};

class Tokenizer {
  public:
    Tokenizer(std::istream &source);

    const Token &next();

    const Token &current() const;

  private:
    std::istream &m_source;
    uint16_t m_line_number;
    Token m_current;
};
} // namespace day25