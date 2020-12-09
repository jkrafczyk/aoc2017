#include "tokenizer.hpp"
#include <regex>
#include <list>

using std::list;
using std::istream;
using std::string;
using std::regex;
using std::regex_match;
using std::endl;

namespace {
    using day25::Token;

    static regex EMPTY_LINE("^\\s*$");

    static regex INITIAL_STATE_RE("Begin in state ([a-zA-Z0-9]+)\\.");
    static regex CHECKSUM_DELAY_RE("Perform a diagnostic checksum after ([0-9]+) steps\\.");
    static regex STATE_DECLARATION_RE("In state ([a-zA-Z0-9]+):");
    static regex STATE_REQUIREMENT_RE("\\s*If the current value is ([01]):");
    static regex STATE_WRITE_RE("\\s*- Write the value ([01])\\.");
    static regex STATE_MOVEMENT_RE("\\s*- Move one slot to the (left|right)\\.");
    static regex STATE_NEXT_RE("\\s*- Continue with state ([a-zA-Z0-9]+)\\.");

    struct TokenExpression {
        const regex &regex;
        const Token::Type type;
    };
    static list<TokenExpression> TOKEN_EXPRESSIONS = {
            {INITIAL_STATE_RE, Token::INITIAL_STATE},
            {CHECKSUM_DELAY_RE, Token::CHECKSUM_DELAY},
            {STATE_DECLARATION_RE, Token::STATE_DECLARATION},
            {STATE_REQUIREMENT_RE, Token::STATE_REQUIREMENT},
            {STATE_WRITE_RE, Token::STATE_WRITE},
            {STATE_MOVEMENT_RE, Token::STATE_MOVEMENT},
            {STATE_NEXT_RE, Token::STATE_NEXT},
    };
}

namespace day25 {
    Tokenizer::Tokenizer(istream &source): m_source(source), m_line_number(0) {}

    const Token &Tokenizer::next() {
        Token result = {
                .line_number = m_line_number,
                .type = Token::ERROR,
                .arg = "",
                .raw_text = "",
        };

        string line;
        while (line == "" || regex_match(line, EMPTY_LINE)) {
            if (m_source.eof()) {
                result.type = Token::END_OF_STREAM;
                return m_current = result;
            }
            getline(m_source, line);
            m_line_number++;
        }
        result.line_number = m_line_number;
        result.raw_text = line;

        std::smatch match;
        for (auto it: TOKEN_EXPRESSIONS) {
            if (regex_match(line, match, it.regex)) {
                result.type = it.type;
                break;
            }
        }

        if (result.type != Token::ERROR && match.size() > 0) {
            result.arg = match[1];
        }

        m_current = result;
        return m_current;
    }

    const Token &Tokenizer::current() const {
        return m_current;
    }
}