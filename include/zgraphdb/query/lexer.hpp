#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace zgraphdb {

// TODO: Defer `$variables`, literals, numeric tokens, FILTER operators,
// and OPTIONAL/UNION keywords until those phases are implemented.
enum class TokenKind {
    PrefixKeyword,
    BaseKeyword,
    SelectKeyword,
    AskKeyword,
    WhereKeyword,
    Variable,
    Iri,
    PrefixedName,
    LBrace,
    RBrace,
    Dot,
    Semicolon,
    Comma,
    End
};

// A token represents a keyword, a variable, or other kinds of text in a query
struct Token {
    TokenKind kind;
    std::string value;
    std::size_t line;
    std::size_t column;
};

std::vector<Token> tokenize(const std::string& text);

}
