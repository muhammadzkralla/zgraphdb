#pragma once

#include <string>
#include <variant>
#include <vector>

namespace zgraphdb {

// TODO: Full SPARQL still needs AST nodes for:
// - literal, blank-node, numeric, and boolean terms in QueryTerm
// - prologue/dataset declarations (PREFIX, BASE, FROM, FROM NAMED)
// - richer graph patterns (FILTER, OPTIONAL, UNION, MINUS, GRAPH, BIND, VALUES, subqueries)
// - FILTER/expression nodes (operators, functions, aggregates)
// - solution modifiers (DISTINCT, REDUCED, ORDER BY, LIMIT, OFFSET)
// - CONSTRUCT and DESCRIBE query forms, plus SELECT expressions and aggregates
struct Variable {
    std::string name;
};

struct IriRef {
    std::string value;
};

using QueryTerm = std::variant<Variable, IriRef>;

struct QueryTriplePattern {
    QueryTerm subject;
    QueryTerm predicate;
    QueryTerm object;
};

struct SelectQuery {
    std::vector<Variable> variables;
    std::vector<QueryTriplePattern> patterns;
};

struct AskQuery {
    std::vector<QueryTriplePattern> patterns;
};

using Query = std::variant<SelectQuery, AskQuery>;

struct QueryDocument {
    Query query;
};

}
