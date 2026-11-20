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
    std::optional<Term> lookup(std::uint32_t id) const;
    std::size_t size() const;

private:
    std::unordered_map<std::string, std::uint32_t> idByKey_;
    std::vector<Term> terms_;
};

}
