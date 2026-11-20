#include <cassert>

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

    return 0;
}
