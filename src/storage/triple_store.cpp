#include "zgraphdb/storage/triple_store.hpp"

namespace zgraphdb {

void TripleStore::add(const EncodedTriple& triple) {
    triples_.push_back(triple);
}

const std::vector<EncodedTriple>& TripleStore::triples() const {
    return triples_;
}

std::size_t TripleStore::size() const {
    return triples_.size();
}

}
