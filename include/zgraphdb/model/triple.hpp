#pragma once

#include <cstdint>
#include "zgraphdb/model/term.hpp"

namespace zgraphdb {

// NOTE: A single term id could refer to 4.3 billion distinct terms.
// I still don't know why an id is not unique to just one term.
using TermId = std::uint32_t;

struct Triple {
    // TODO: subject should be validated to ensure it's not a literal term type.
    Term subject;
    // MUST BE ALWAYS AN IRI
    Iri predicate;
    // Could be any term type
    Term object;
};

struct EncodedTriple {
    TermId subject;
    TermId predicate;
    TermId object;
};

}
