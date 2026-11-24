#include "zgraphdb/query/parser.hpp"

#include <stdexcept>
#include <string>
#include <unordered_map>

namespace zgraphdb {
namespace {

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens) : tokens_(tokens) {}

    // Takes a vector of tokens representing a query and converts them into a structured AST
    QueryDocument parse() {
        if (tokens_.empty()) {
            throw std::runtime_error("empty token stream");
        }

        parsePrologue();
        Query query = parseQuery();
        if (!atEnd()) {
            throw fail(peek(), "unexpected content after query");
        }
        return QueryDocument{query};
    }

private:
    // The tokens vector and a cursor
    const std::vector<Token>& tokens_;
    std::size_t position_ = 0;
    // A local prefixes map to resolve prefixed names during parse time
    std::unordered_map<std::string, std::string> prefixes_;

    // Peeks on the current token
    const Token& peek() const {
        return tokens_[position_];
    }

    bool atEnd() const {
        return peek().kind == TokenKind::End;
    }

    // Consumes the current token and advances the cursor
    const Token& advance() {
        return tokens_[position_++];
    }

    std::runtime_error fail(const Token& token, const std::string& message) const {
        return std::runtime_error("line " + std::to_string(token.line) + ", column " + std::to_string(token.column) + ": " + message);
    }

    // Consumes the current token and advances the cursor if the expected kind is matching its king
    const Token& expect(TokenKind kind, const char* description) {
        const Token& token = peek();
        if (token.kind != kind) {
            throw fail(token, std::string("expected ") + description);
        }
        ++position_;
        return token;
    }

    // Parses all the tokens related to the prefixes and store their value in the local prefixes map
    void parsePrologue() {
        while (peek().kind == TokenKind::PrefixKeyword) {
            ++position_;
            const Token label = expect(TokenKind::PrefixedName, "prefix name");
            const auto colon = label.value.find(':');
            if (colon == std::string::npos || colon == 0 || colon + 1 != label.value.size()) {
                throw fail(label, "invalid prefix declaration");
            }
            const Token iri = expect(TokenKind::Iri, "prefix IRI");
            prefixes_[label.value.substr(0, colon)] = iri.value;
        }

        // TODO: Implement `BASE` keyword later
        if (peek().kind == TokenKind::BaseKeyword) {
            throw fail(peek(), "BASE is not supported yet");
        }
    }

    // Parses all the tokens related to the query after the prefix tokens
    // TODO: Currently only `SELECT` and `ASK` keywords are parsed
    Query parseQuery() {
        if (peek().kind == TokenKind::SelectKeyword) {
            return parseSelect();
        }
        if (peek().kind == TokenKind::AskKeyword) {
            return parseAsk();
        }
        throw fail(peek(), "expected SELECT or ASK");
    }

    // Parses the current `SELECT` query token and constructs the `SelectQuery` tree node
    SelectQuery parseSelect() {
        ++position_;
        SelectQuery query;
        
        // Process the variables in this `SELECT` query
        while (peek().kind == TokenKind::Variable) {
            query.variables.push_back(Variable{advance().value});
        }

        // A `SELECT` query MUST have at least one variable
        if (query.variables.empty()) {
            throw fail(peek(), "expected at least one SELECT variable");
        }

        // A `WHERE` token MUST be expected after processing the variables of the `SELECT` token
        expect(TokenKind::WhereKeyword, "WHERE");
        query.patterns = parseGroup();
        return query;
    }

    // Parses the current `ASK` query token and constructs the `AskQuery` tree node
    AskQuery parseAsk() {
        ++position_;
        expect(TokenKind::WhereKeyword, "WHERE");
        AskQuery query;
        query.patterns = parseGroup();
        return query;
    }

    // Parses a group of query triple patterns inside a `SELECT` or an `ASK` query
    std::vector<QueryTriplePattern> parseGroup() {
        // A open left brace token MUST be expected before any query triple pattern
        expect(TokenKind::LBrace, "{");
        std::vector<QueryTriplePattern> patterns;
        while (peek().kind != TokenKind::RBrace) {
            if (atEnd()) {
                throw fail(peek(), "expected }");
            }
            patterns.push_back(parseTriple());

            // A dot token MUST be expected between all query triple patterns
            expect(TokenKind::Dot, ".");
        }

        // A closed right brace token MUST be expected after all query triple patterns
        expect(TokenKind::RBrace, "}");
        return patterns;
    }

    // Parses a query triple pattern and constructs the `QueryTriplePattern` tree node
    QueryTriplePattern parseTriple() {
        QueryTriplePattern pattern;
        pattern.subject = parseTerm();
        pattern.predicate = parseTerm();
        pattern.object = parseTerm();
        return pattern;
    }

    // Parses a single term inside a query triple pattern which could be a variable, IRI, or a PrefixedName
    QueryTerm parseTerm() {
        const Token token = peek();
        if (token.kind == TokenKind::Variable) {
            ++position_;
            return Variable{token.value};
        }
        if (token.kind == TokenKind::Iri) {
            ++position_;
            return IriRef{token.value};
        }
        if (token.kind == TokenKind::PrefixedName) {
            ++position_;
            return IriRef{resolvePrefixedName(token)};
        }
        throw fail(token, "expected variable, IRI, or prefixed name");
    }

    // Resolves the value of the passed token by looking it up in the 
    // local prefixes map constructed while processing the tokens
    std::string resolvePrefixedName(const Token& token) const {
        const auto colon = token.value.find(':');
        if (colon == std::string::npos || colon == 0 || colon + 1 == token.value.size()) {
            throw fail(token, "invalid prefixed name");
        }
        const std::string prefix = token.value.substr(0, colon);
        const std::string local = token.value.substr(colon + 1);
        const auto found = prefixes_.find(prefix);
        if (found == prefixes_.end()) {
            throw fail(token, "unknown prefix '" + prefix + "'");
        }

        // Replace the prefix name in the query with the prefix value
        // for example: "foaf:knows" -> "http://xmlns.com/foaf/0.1/knows"
        return found->second + local;
    }
};

}

// Takes a vector of tokens and constructs the AST
QueryDocument parseQuery(const std::vector<Token>& tokens) {
    Parser parser(tokens);
    return parser.parse();
}

}
