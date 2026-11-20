#pragma once

#include <cstdint>
#include "zgraphdb/model/term.hpp"

namespace zgraphdb {

using TermId = std::uint32_t;

struct Triple {
    Term subject;
    Iri predicate;
    Term object;
};

struct EncodedTriple {
    TermId subject;
    TermId predicate;
    TermId object;
};

}
