#pragma once

#include <cstddef>
#include <string>
#include "zgraphdb/storage/dictionary.hpp"
#include "zgraphdb/storage/triple_store.hpp"

namespace zgraphdb {

class Database {
public:
    void loadNTriples(const std::string& path);
    std::size_t size() const;

private:
    Dictionary dictionary_;
    TripleStore store_;
};

}
