#include "zgraphdb/model/term.hpp"

namespace zgraphdb {

TermKind kindOf(const Term& term) {
    if (std::holds_alternative<Iri>(term)) {
        return TermKind::Iri;
    }
    if (std::holds_alternative<BlankNode>(term)) {
        return TermKind::BlankNode;
    }
    return TermKind::Literal;
}

bool operator==(const Iri& lhs, const Iri& rhs) {
    return lhs.value == rhs.value;
}

bool operator==(const BlankNode& lhs, const BlankNode& rhs) {
    return lhs.id == rhs.id;
}

bool operator==(const Literal& lhs, const Literal& rhs) {
    return lhs.value == rhs.value && lhs.language == rhs.language && lhs.datatype == rhs.datatype;
}

}
