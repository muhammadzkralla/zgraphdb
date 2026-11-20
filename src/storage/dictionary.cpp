#include "zgraphdb/storage/dictionary.hpp"

#include <sstream>
#include <utility>

namespace zgraphdb {
namespace {

// A private helper to this file.
// If the term is an IRI: returns "I:value".
// If the term is a BlankNode: returns "B:id".
// If the term is a literal: returns "L:value|language|datatype"
std::string keyOf(const Term& term) {
    std::ostringstream out;
    if (const auto* iri = std::get_if<Iri>(&term)) {
        out << "I:" << iri->value;
    } else if (const auto* blank = std::get_if<BlankNode>(&term)) {
        out << "B:" << blank->id;
    } else {
        //TODO: Note that a literal containing a '|' character will break this encoding algorithm.
        const auto* literal = std::get_if<Literal>(&term);
        out << "L:" << literal->value << "|" << literal->language << "|" << literal->datatype;
    }
    return out.str();
}

}

// Maps the term to a unique identifier that is a uint32_t
// This is because storing the actual term as a std::string
// would waste much heap allocated memory.
std::uint32_t Dictionary::intern(const Term& term) {
    auto key = keyOf(term);
    auto found = idByKey_.find(key);
    // If the term already has an id, return it
    if (found != idByKey_.end()) {
        return found->second;
    }
    auto id = static_cast<std::uint32_t>(terms_.size());
    // NOTE: std:move() moves the ownership of the key to the map instead of copying it
    idByKey_.emplace(std::move(key), id);
    terms_.push_back(term);
    return id;
}

// Finds the term with that id if exists, might not exist, so optional.
std::optional<Term> Dictionary::lookup(std::uint32_t id) const {
    if (id >= terms_.size()) {
        return std::nullopt;
    }
    return terms_[id];
}

// Returns the number of distinct terms in our database
std::size_t Dictionary::size() const {
    return terms_.size();
}

}
