#pragma once

#include <cstddef>
#include <string>
#include "zgraphdb/storage/dictionary.hpp"
#include "zgraphdb/storage/triple_store.hpp"
#include "zgraphdb/storage/indexes.hpp"

namespace zgraphdb {

class Database {
public:
    void loadNTriples(const std::string& path);
    std::size_t size() const;
    const Indexes& indexes() const;

private:
    // Responsible for managing the terms in our database 
    Dictionary dictionary_;
    // Responsible for managing the triples in our database
    TripleStore store_;
    // Responsible for managing the indexes in our database
    Indexes indexes_;
};

}
