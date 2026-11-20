#include "zgraphdb/storage/dictionary.hpp"

#include <sstream>
#include <utility>

namespace zgraphdb {
namespace {

std::string keyOf(const Term& term) {
    std::ostringstream out;
    if (const auto* iri = std::get_if<Iri>(&term)) {
        out << "I:" << iri->value;
    } else if (const auto* blank = std::get_if<BlankNode>(&term)) {
        out << "B:" << blank->id;
    } else {
        const auto* literal = std::get_if<Literal>(&term);
        out << "L:" << literal->value << "|" << literal->language << "|" << literal->datatype;
    }
    return out.str();
}

}

std::uint32_t Dictionary::intern(const Term& term) {
    auto key = keyOf(term);
    auto found = idByKey_.find(key);
    if (found != idByKey_.end()) {
        return found->second;
    }
    auto id = static_cast<std::uint32_t>(terms_.size());
    idByKey_.emplace(std::move(key), id);
    terms_.push_back(term);
    return id;
}

std::optional<Term> Dictionary::lookup(std::uint32_t id) const {
    if (id >= terms_.size()) {
        return std::nullopt;
    }
    return terms_[id];
}

std::size_t Dictionary::size() const {
    return terms_.size();
}

}
