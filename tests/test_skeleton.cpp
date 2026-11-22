#include <cassert>
#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>

#include "zgraphdb/api/db.hpp"
#include "zgraphdb/storage/dictionary.hpp"
#include "zgraphdb/storage/triple_store.hpp"

int main() {
    zgraphdb::Dictionary dictionary;
    auto alice = dictionary.intern(zgraphdb::Iri{"http://example.org/alice"});
    auto aliceAgain = dictionary.intern(zgraphdb::Iri{"http://example.org/alice"});
    assert(alice == aliceAgain);
    assert(dictionary.size() == 1);

    auto name = dictionary.intern(zgraphdb::Iri{"http://xmlns.com/foaf/0.1/name"});
    assert(name != alice);
    assert(dictionary.lookup(alice).has_value());

    zgraphdb::TripleStore store;
    store.add({alice, name, aliceAgain});
    assert(store.size() == 1);

    zgraphdb::Database db;
    assert(db.size() == 0);

    zgraphdb::Database loaded;
    loaded.loadNTriples(std::string(ZGRAPHDB_EXAMPLES_DIR) + "/social.ntriples");
    assert(loaded.size() == 5);

    const std::string duplicatePath = "test_duplicates.nt";
    {
        std::ofstream out(duplicatePath);
        out << "<http://example.org/a> <http://example.org/p> \"v\" .\n";
        out << "<http://example.org/a> <http://example.org/p> \"v\" .\n";
    }
    zgraphdb::Database duplicates;
    duplicates.loadNTriples(duplicatePath);
    assert(duplicates.size() == 1);
    std::remove(duplicatePath.c_str());

    const std::string badPath = "test_bad.nt";
    {
        std::ofstream out(badPath);
        out << "<http://example.org/a> <http://example.org/p> \"v\" .\n";
        out << "this is not a triple\n";
    }
    bool threw = false;
    std::string message;
    try {
        zgraphdb::Database bad;
        bad.loadNTriples(badPath);
    } catch (const std::runtime_error& error) {
        threw = true;
        message = error.what();
    }
    assert(threw);
    assert(message.find("line 2") != std::string::npos);
    std::remove(badPath.c_str());

    return 0;
}
