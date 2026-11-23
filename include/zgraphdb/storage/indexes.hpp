#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <vector>
#include "zgraphdb/model/triple.hpp"

namespace zgraphdb {

// A triple pattern is used to represent some sort of a query,
// something like {?s p o} or {?s p ?o} and so on, any term could be required
using TriplePattern = std::array<std::optional<TermId>, 3>;

class Indexes {
public:
    void rebuild(const std::vector<EncodedTriple>& triples);
    std::vector<EncodedTriple> lookup(const TriplePattern& pattern) const;
    std::size_t size() const;

private:
    // A key is just another form of an `EncodedTriple`
    // The reason why we didn't use an `EncodedTriple`
    // is that the order of terms is not fixed in
    // the indexes, it could be {spo}/{pos}/{osp}
    // unlike in an `EncodedTriple`: {SPO} only
    using Key = std::array<TermId, 3>;

    // Our three indexes for now
    // TODO: Introduce `SOP`, `PSO`, and `OPS` indexes later
    std::vector<Key> spo_;
    std::vector<Key> pos_;
    std::vector<Key> osp_;
};

}
