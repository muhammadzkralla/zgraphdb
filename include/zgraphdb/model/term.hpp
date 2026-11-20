#pragma once

#include <string>
#include <variant>

namespace zgraphdb {

struct Iri {
    std::string value;
};

struct BlankNode {
    std::string id;
};

struct Literal {
    std::string value;
    std::string language;
    std::string datatype;
};

// NOTE: A triple is constructed like that: (subject, predicate, object)
// Each one of those three is called a term.
// A subject could be either an IRI or a BlankNode
// A predicate MUST ALWAYS be an IRI
// An object could be an IRI, a BlankNode, or a Literal
using Term = std::variant<Iri, BlankNode, Literal>;

enum class TermKind { Iri, BlankNode, Literal };

TermKind kindOf(const Term& term);

bool operator==(const Iri& lhs, const Iri& rhs);
bool operator==(const BlankNode& lhs, const BlankNode& rhs);
bool operator==(const Literal& lhs, const Literal& rhs);

}
