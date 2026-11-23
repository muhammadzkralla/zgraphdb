#pragma once

#include <cstdint>
#include "zgraphdb/model/term.hpp"

namespace zgraphdb {

// NOTE: Using a uint32_t means that we could have up to 4.3 billion unique term in our database
using TermId = std::uint32_t;

// A human-readable format of a triple in the database
struct Triple {
    // Might be an IRI or a BlankNode, can't be a Literal
    Term subject;
    // MUST BE ALWAYS AN IRI
    Iri predicate;
    // Could be any term type
    Term object;
};

// An encoded format of the tripe to store in the database
struct EncodedTriple {
    TermId subject;
    TermId predicate;
    TermId object;
};

}
