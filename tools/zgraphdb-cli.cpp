#include <iostream>

#include "zgraphdb/api/db.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: zgraphdb-cli <ntriples-file>\n";
        return 1;
    }

    zgraphdb::Database db;
    db.loadNTriples(argv[1]);
    std::cout << "zgraphdb skeleton\n";
    std::cout << "triples: " << db.size() << "\n";
    return 0;
}
