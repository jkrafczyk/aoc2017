#include "parser.hpp"

using std::string;

namespace day25 {
    Parser::Parser(Tokenizer &source): m_source(source) {}

    const ParserState &Parser::parse() {
        bool initial_state_seen = false;
        bool checksum_delay_seen = false;

        while (true) {
            auto token = m_source.next();

            if (token.type == Token::END_OF_STREAM) {
                if (!initial_state_seen) {
                    return error(token, "Initial state not defined.");
                }
                if (!checksum_delay_seen) {
                    return error(token, "Delay until checksum not defined.");
                }
                return finalize_program(token);

            } else if (token.type == Token::ERROR) {
                return error(token, "Invalid token.");

            } else if (token.type == Token::INITIAL_STATE) {
                if (initial_state_seen) {
                    return error(token, "Multiple initial state declarations.");
                } else {
                    initial_state_seen = true;
                    m_program.initial_state = token.arg;
                }

            } else if (token.type == Token::CHECKSUM_DELAY) {
                if (checksum_delay_seen) {
                    return error(token, "Multiple checksum declarations.");
                } else {
                    checksum_delay_seen = true;
                    m_program.checksum_delay = std::stoul(token.arg);
                }

            } else if (token.type == Token::STATE_DECLARATION) {
                auto result = parse_state(token);
                if (result.error) {
                    return m_last_state;
                }
            }

            else {
                return error(token, "Syntax error. Expected 'In State...' block.");
            }
        }
    }

    const ParserState& Parser::parse_state(const Token &state_declaration) {
        const std::string &name = state_declaration.arg;
        if (m_program.states.count(name)) {
            return error(state_declaration, "Multiple definitions encountered for state " + name);
        }
        m_program.states[name] = State{
                .name = name
        };

        for (int i=0; i<=1; i++) {
            auto requirement = m_source.next();
            if (requirement.type != Token::STATE_REQUIREMENT) {
                return error(requirement, "Expected exactly two 'If the current value is...' blocks after state declaration.");
            }
            auto write = m_source.next();
            if (write.type != Token::STATE_WRITE) {
                return error(write, "Expected '- Write the value...' as first line in action block.");
            }
            auto move = m_source.next();
            if (move.type != Token::STATE_MOVEMENT) {
                return error(move, "Expected '- Move one slot...' as second line in action block.");
            }
            auto next = m_source.next();
            if (next.type != Token::STATE_NEXT) {
                return error(next, "Expected '- Continue with state...' as third line in action block.");
            }
            m_program.states[name].actions[i].slot_condition = i;
            m_program.states[name].actions[i].write_value = std::stoi(write.arg);
            m_program.states[name].actions[i].move_direction = move.arg == "right" ? 1 : -1;
            m_program.states[name].actions[i].next_state = next.arg;
        }

        return ok(m_source.current());
    }

    const ParserState &Parser::finalize_program(const Token &eof_token) {
        for (auto state: m_program.states) {
            for (auto action: state.second.actions) {
                if (!m_program.states.count(action.second.next_state)) {
                    return error(eof_token, "Actions for state " + state.first + " refer to state " + action.second.next_state + ", which is undefined");
                }
            }
        }
        if (!m_program.states.count(m_program.initial_state)) {
            return error(eof_token, "Program specifies initial state" + m_program.initial_state + ", which does not exist.");
        }
        return eof(eof_token);
    }

    Program Parser::program() {
        if (m_last_state.eof && !m_last_state.error) {
            return m_program;
        } else {
            return Program{};
        }
    }

    const ParserState &Parser::error(const Token &token, const string &message) {
        return m_last_state = ParserState {
                .error = true,
                .eof = false,
                .error_message = message,
                .token = token,
        };
    }

    const ParserState &Parser::eof(const Token &token) {
        return m_last_state = ParserState {
                .error = false,
                .eof = true,
                .error_message = "",
                .token = token,
        };
    }

    const ParserState &Parser::ok(const Token &token) {
        return m_last_state = ParserState {
                .error = false,
                .eof = false,
                .error_message = "",
                .token = token,
        };
    }
}