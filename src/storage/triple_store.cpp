#include "zgraphdb/storage/triple_store.hpp"

namespace zgraphdb {

// Appends one encoded triple to the database.
void TripleStore::add(const EncodedTriple& triple) {
    triples_.push_back(triple);
}

// Returns a read-only instance of the vector without copying.
const std::vector<EncodedTriple>& TripleStore::triples() const {
    return triples_;
}

// Returns the number of distinct encoded triples in our database
std::size_t TripleStore::size() const {
    return triples_.size();
}

}
