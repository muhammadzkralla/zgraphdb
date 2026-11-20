#pragma once

#include <cstddef>
#include <vector>
#include "zgraphdb/model/triple.hpp"

namespace zgraphdb {

class TripleStore {
public:
    void add(const EncodedTriple& triple);
    const std::vector<EncodedTriple>& triples() const;
    std::size_t size() const;

private:
    std::vector<EncodedTriple> triples_;
};

}
