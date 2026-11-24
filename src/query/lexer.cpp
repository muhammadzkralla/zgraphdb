#include "zgraphdb/query/lexer.hpp"

#include <cctype>
#include <optional>
#include <stdexcept>
#include <string>

namespace zgraphdb {

// Helper functions for checking token types
namespace {

bool isAlpha(char character) {
    return std::isalpha(static_cast<unsigned char>(character)) != 0;
}

bool isDigit(char character) {
    return std::isdigit(static_cast<unsigned char>(character)) != 0;
}

bool isNameChar(char character) {
    return isAlpha(character) || isDigit(character) || character == '_' || character == '-';
}

class Scanner {
public:
    // Constructor to initialize private `text_` field which holds the actual query
    explicit Scanner(const std::string& text) : text_(text) {}

    // Scans the next token in the query
    std::optional<Token> next() {
        skipIgnored();
        if (atEnd()) {
            return std::nullopt;
        }

        const std::size_t startLine = line_;
        const std::size_t startColumn = column_;
        const char character = text_[position_];

        // Parse the token type according to what character it starts with
        if (isAlpha(character)) {
            return scanWord(startLine, startColumn);
        }
        if (character == '?') {
            return scanVariable(startLine, startColumn);
        }
        if (character == '<') {
            return scanIri(startLine, startColumn);
        }
        return scanPunctuation(startLine, startColumn);
    }

    std::size_t line() const {
        return line_;
    }

    std::size_t column() const {
        return column_;
    }

private:
    // The actual query value and some cursors
    // NOTE: `position_` is the absolute index of a character in the current query
    // while `column_` is the index of a character relative to the current line
    const std::string& text_;
    std::size_t position_ = 0;
    std::size_t line_ = 1;
    std::size_t column_ = 1;

    bool atEnd() const {
        return position_ >= text_.size();
    }

    char current() const {
        return text_[position_];
    }

    // Advances the cursors to scan the next token
    void advance() {
        if (text_[position_] == '\n') {
            ++line_;
            column_ = 1;
        } else {
            ++column_;
        }
        ++position_;
    }

    // Skip white spaces and comments to avoid scanning them
    void skipIgnored() {
        while (!atEnd()) {
            if (std::isspace(static_cast<unsigned char>(current())) != 0) {
                advance();
                continue;
            }
            if (current() == '#') {
                while (!atEnd() && current() != '\n') {
                    advance();
                }
                continue;
            }
            break;
        }
    }

    // Decides which keyword type is this token
    std::optional<TokenKind> keywordKind(const std::string& upper) const {
        if (upper == "PREFIX") {
            return TokenKind::PrefixKeyword;
        }
        if (upper == "BASE") {
            return TokenKind::BaseKeyword;
        }
        if (upper == "SELECT") {
            return TokenKind::SelectKeyword;
        }
        if (upper == "ASK") {
            return TokenKind::AskKeyword;
        }
        if (upper == "WHERE") {
            return TokenKind::WhereKeyword;
        }
        return std::nullopt;
    }

    std::runtime_error fail(std::size_t line, std::size_t column, const std::string& message) const {
        return std::runtime_error("line " + std::to_string(line) + ", column " + std::to_string(column) + ": " + message);
    }

    // Assembles the token
    Token makeToken(TokenKind kind, const std::string& value, std::size_t line, std::size_t column) const {
        return {kind, value, line, column};
    }


    // Assembles the current name
    std::string readName() {
        std::string word;
        while (!atEnd() && isNameChar(current())) {
            word.push_back(current());
            advance();
        }
        return word;
    }

    // Tokenizes a prefixed name
    Token scanPrefixedName(const std::string& prefix, std::size_t line, std::size_t column) {
        advance();
        std::string local;
        while (!atEnd() && isNameChar(current())) {
            local.push_back(current());
            advance();
        }
        return makeToken(TokenKind::PrefixedName, prefix + ":" + local, line, column);
    }

    // Tokenizes a keyword and decides which type is this keyword
    Token scanKeyword(const std::string& word, std::size_t line, std::size_t column) {
        std::string upper = word;
        for (char& character : upper) {
            character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
        }

        const auto kind = keywordKind(upper);
        if (!kind.has_value()) {
            throw fail(line, column, "unknown keyword '" + word + "'");
        }
        return makeToken(*kind, upper, line, column);
    }

    // Tokenizes any kind of word and decides whether it's a keyword or a prefix
    Token scanWord(std::size_t line, std::size_t column) {
        const std::string word = readName();
        if (!atEnd() && current() == ':') {
            return scanPrefixedName(word, line, column);
        }
        return scanKeyword(word, line, column);
    }

    // Tokenizes a variable
    Token scanVariable(std::size_t line, std::size_t column) {
        // Advance the cursors first because we don't want the starting "?" character
        advance();

        // Assemble the variable name
        std::string name;
        while (!atEnd() && isNameChar(current())) {
            name.push_back(current());
            advance();
        }
        if (name.empty()) {
            throw fail(line, column, "empty variable name");
        }
        return makeToken(TokenKind::Variable, name, line, column);
    }

    // Tokenizes an IRI term
    Token scanIri(std::size_t line, std::size_t column) {
        // Advance the cursors first because we don't want the starting "<" character
        advance();
        std::string value;
        while (!atEnd() && current() != '>') {
            if (current() == '\n' || current() == '\r') {
                throw fail(line, column, "unterminated IRI");
            }
            value.push_back(current());
            advance();
        }
        if (atEnd()) {
            throw fail(line, column, "unterminated IRI");
        }
        advance();
        return makeToken(TokenKind::Iri, value, line, column);
    }

    // Tokenizes different kinds of punctuations
    Token scanPunctuation(std::size_t line, std::size_t column) {
        const char character = current();
        TokenKind kind;
        if (character == '{') {
            kind = TokenKind::LBrace;
        } else if (character == '}') {
            kind = TokenKind::RBrace;
        } else if (character == '.') {
            kind = TokenKind::Dot;
        } else if (character == ';') {
            kind = TokenKind::Semicolon;
        } else if (character == ',') {
            kind = TokenKind::Comma;
        } else {
            throw fail(line, column, std::string("unexpected character '") + character + "'");
        }
        advance();
        return makeToken(kind, std::string(1, character), line, column);
    }
};

}

// Takes a query, parse it, and tokenize it to return a vector of extracted tokens
std::vector<Token> tokenize(const std::string& text) {
    Scanner scanner(text);
    std::vector<Token> tokens;
    while (const auto token = scanner.next()) {
        tokens.push_back(*token);
    }
    tokens.push_back({TokenKind::End, "", scanner.line(), scanner.column()});
    return tokens;
}

}
