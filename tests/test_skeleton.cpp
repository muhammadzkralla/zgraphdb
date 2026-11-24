#include <cassert>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "zgraphdb/api/db.hpp"
#include "zgraphdb/query/lexer.hpp"
#include "zgraphdb/storage/dictionary.hpp"
#include "zgraphdb/storage/indexes.hpp"
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
    assert(loaded.indexes().size() == loaded.size());

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

    zgraphdb::Indexes indexes;
    const std::vector<zgraphdb::EncodedTriple> sample = {
        {1, 2, 3},
        {1, 4, 3},
        {5, 2, 6},
    };
    indexes.rebuild(sample);

    const auto all = indexes.lookup(zgraphdb::TriplePattern{});
    assert(all.size() == 3);

    const auto bySubject = indexes.lookup({1, std::nullopt, std::nullopt});
    assert(bySubject.size() == 2);
    assert(bySubject[0].subject == 1);
    assert(bySubject[0].predicate == 2);
    assert(bySubject[0].object == 3);

    const auto byPredicate = indexes.lookup({std::nullopt, 2, std::nullopt});
    assert(byPredicate.size() == 2);
    assert(byPredicate[0].subject == 1);
    assert(byPredicate[0].predicate == 2);
    assert(byPredicate[0].object == 3);
    assert(byPredicate[1].subject == 5);
    assert(byPredicate[1].predicate == 2);
    assert(byPredicate[1].object == 6);

    const auto byObject = indexes.lookup({std::nullopt, std::nullopt, 3});
    assert(byObject.size() == 2);
    assert(byObject[0].subject == 1);
    assert(byObject[0].predicate == 2);
    assert(byObject[0].object == 3);
    assert(byObject[1].subject == 1);
    assert(byObject[1].predicate == 4);
    assert(byObject[1].object == 3);

    const auto bySubjectPredicate = indexes.lookup({1, 2, std::nullopt});
    assert(bySubjectPredicate.size() == 1);

    const auto bySubjectObject = indexes.lookup({1, std::nullopt, 3});
    assert(bySubjectObject.size() == 2);

    const auto byPredicateObject = indexes.lookup({std::nullopt, 2, 6});
    assert(byPredicateObject.size() == 1);
    assert(byPredicateObject[0].subject == 5);
    assert(byPredicateObject[0].predicate == 2);
    assert(byPredicateObject[0].object == 6);

    const auto exact = indexes.lookup({5, 2, 6});
    assert(exact.size() == 1);
    assert(exact[0].subject == 5);
    assert(exact[0].predicate == 2);
    assert(exact[0].object == 6);

    const std::string query =
        "PREFIX foaf: <http://xmlns.com/foaf/0.1/>\n"
        "SELECT ?name WHERE {\n"
        "  <http://example.org/alice> foaf:knows ?friend .\n"
        "  ?friend foaf:name ?name .\n"
        "}\n";
    const auto tokens = zgraphdb::tokenize(query);
    const std::vector<zgraphdb::TokenKind> expected = {
        zgraphdb::TokenKind::PrefixKeyword,
        zgraphdb::TokenKind::PrefixedName,
        zgraphdb::TokenKind::Iri,
        zgraphdb::TokenKind::SelectKeyword,
        zgraphdb::TokenKind::Variable,
        zgraphdb::TokenKind::WhereKeyword,
        zgraphdb::TokenKind::LBrace,
        zgraphdb::TokenKind::Iri,
        zgraphdb::TokenKind::PrefixedName,
        zgraphdb::TokenKind::Variable,
        zgraphdb::TokenKind::Dot,
        zgraphdb::TokenKind::Variable,
        zgraphdb::TokenKind::PrefixedName,
        zgraphdb::TokenKind::Variable,
        zgraphdb::TokenKind::Dot,
        zgraphdb::TokenKind::RBrace,
        zgraphdb::TokenKind::End,
    };
    assert(tokens.size() == expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i) {
        assert(tokens[i].kind == expected[i]);
    }
    assert(tokens[0].value == "PREFIX");
    assert(tokens[1].value == "foaf:");
    assert(tokens[2].value == "http://xmlns.com/foaf/0.1/");
    assert(tokens[4].value == "name");
    assert(tokens[8].value == "foaf:knows");
    assert(tokens[9].value == "friend");
    assert(tokens[16].value.empty());
    assert(tokens[4].line == 2);
    assert(tokens[8].line == 3);

    bool lexerThrew = false;
    try {
        zgraphdb::tokenize("SELECT <oops");
    } catch (const std::runtime_error&) {
        lexerThrew = true;
    }
    assert(lexerThrew);

    return 0;
}
