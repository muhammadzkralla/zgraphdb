#include "zgraphdb/api/db.hpp"

namespace zgraphdb {

void Database::loadNTriples(const std::string& path) {
    (void)path;
}

std::size_t Database::size() const {
    return store_.size();
}

}
