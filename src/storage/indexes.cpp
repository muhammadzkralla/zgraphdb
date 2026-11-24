#include "zgraphdb/storage/indexes.hpp"

#include <algorithm>
#include <cstddef>

namespace zgraphdb {
namespace {


// A key is just another form of an `EncodedTriple`
// The reason why we didn't use an `EncodedTriple`
// is that the order of terms is not fixed in
// the indexes, it could be {spo}/{pos}/{osp}
// unlike in an `EncodedTriple`: {SPO} only
using Key = std::array<TermId, 3>;

// A triple pattern is used to represent some sort of a query,
// something like {?s p o} or {?s p ?o} and so on, any term could be required
using Pattern = TriplePattern;

// Some mapper functions to help in mapping encoded triples to keys:

Key toSpo(const EncodedTriple& triple) {
    return {triple.subject, triple.predicate, triple.object};
}

Key toPos(const EncodedTriple& triple) {
    return {triple.predicate, triple.object, triple.subject};
}

Key toOsp(const EncodedTriple& triple) {
    return {triple.object, triple.subject, triple.predicate};
}

EncodedTriple fromSpo(const Key& key) {
    return {key[0], key[1], key[2]};
}

EncodedTriple fromPos(const Key& key) {
    return {key[2], key[0], key[1]};
}

EncodedTriple fromOsp(const Key& key) {
    return {key[1], key[2], key[0]};
}

// Helper comparator functions to be used in the binary search functions
// `lower_bound` and `upper_bound`, used to check if the current key
// is less than or greater than the required group of triples
struct PrefixLess {
    bool operator()(const Key& key, const Pattern& prefix) const {
        for (std::size_t i = 0; i < 3; ++i) {
            if (!prefix[i].has_value()) {
                continue;
            }
            if (key[i] != *prefix[i]) {
                return key[i] < *prefix[i];
            }
        }
        return false;
    }

    bool operator()(const Pattern& prefix, const Key& key) const {
        for (std::size_t i = 0; i < 3; ++i) {
            if (!prefix[i].has_value()) {
                continue;
            }
            if (key[i] != *prefix[i]) {
                return *prefix[i] < key[i];
            }
        }
        return false;
    }
};

// Returns a vector of encoded triples from the passed `index` vector
// matching the passed `prefix` query
// after decoding them from a key to an encoded triple using the passed `decode` function
std::vector<EncodedTriple> collect(const std::vector<Key>& index,
                                   const Pattern& prefix,
                                   EncodedTriple (*decode)(const Key&)) {
    const auto first = std::lower_bound(index.begin(), index.end(), prefix, PrefixLess{});
    const auto last = std::upper_bound(index.begin(), index.end(), prefix, PrefixLess{});
    std::vector<EncodedTriple> result;
    for (auto it = first; it != last; ++it) {
        result.push_back(decode(*it));
    }
    return result;
}

}

// Rebuilds all the indexes in our database, called after triples are loaded
void Indexes::rebuild(const std::vector<EncodedTriple>& triples) {
    spo_.clear();
    pos_.clear();
    osp_.clear();

    spo_.reserve(triples.size());
    pos_.reserve(triples.size());
    osp_.reserve(triples.size());

    // Push all the triples inside our database to our
    // index vectors after mapping each encoded triple to a key
    // matching the order of terms of each index vector
    for (const auto& triple : triples) {
        spo_.push_back(toSpo(triple));
        pos_.push_back(toPos(triple));
        osp_.push_back(toOsp(triple));
    }

    // Sort all the indexes to enable binary searching them in O(LOG n + k)
    std::sort(spo_.begin(), spo_.end());
    std::sort(pos_.begin(), pos_.end());
    std::sort(osp_.begin(), osp_.end());
}

// Performs the actual querying process by looking up the optimal index for the query
std::vector<EncodedTriple> Indexes::lookup(const TriplePattern& pattern) const {
    const bool hasSubject = pattern[0].has_value();
    const bool hasPredicate = pattern[1].has_value();
    const bool hasObject = pattern[2].has_value();

    // Choose the optimal index to perform the query
    if (hasObject && !hasPredicate) {
        Pattern prefix{pattern[2], pattern[0], pattern[1]};
        return collect(osp_, prefix, fromOsp);
    }
    if (hasPredicate && !hasSubject) {
        Pattern prefix{pattern[1], pattern[2], pattern[0]};
        return collect(pos_, prefix, fromPos);
    }
    return collect(spo_, pattern, fromSpo);
}

// Returns the `spo` index's vector size, `pos` and `osp`
// indexes are guaranteed to have the same size
std::size_t Indexes::size() const {
    return spo_.size();
}

}
