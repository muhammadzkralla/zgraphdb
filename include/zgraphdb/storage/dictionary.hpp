#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "zgraphdb/model/term.hpp"

namespace zgraphdb {

class Dictionary {
public:
    std::uint32_t intern(const Term& term);
    std::optional<std::uint32_t> find(const Term& term) const;
    std::optional<Term> lookup(std::uint32_t id) const;
    std::size_t size() const;

private:
    // A hashmap of {term key->id} AVG O(1) lookup
    std::unordered_map<std::string, std::uint32_t> idByKey_;
    // Direct indexing by position, {id->term}, ids are vector indices
    std::vector<Term> terms_;
};

}
