#include "zgraphdb/api/db.hpp"

#include <array>
#include <cctype>
#include <fstream>
#include <set>
#include <stdexcept>

namespace zgraphdb {
namespace {

// Increment the `pos` index of the current `line` if it's currently on a white space
void skipWhitespace(const std::string& line, std::size_t& pos) {
    while (pos < line.size() &&
           std::isspace(static_cast<unsigned char>(line[pos])) != 0) {
        ++pos;
    }
}

// Check if the sequence of characters starting at `pos` of the current `line` matches `prefix`
bool startsWith(const std::string& line, std::size_t pos, const char* prefix) {
    for (std::size_t i = 0; prefix[i] != '\0'; ++i) {
        if (pos + i >= line.size() || line[pos + i] != prefix[i]) {
            return false;
        }
    }
    return true;
}

// Extract the IRI found at `pos` of the current `line` and parse it as a term
Term parseIri(const std::string& line, std::size_t& pos) {
    if (pos >= line.size() || line[pos] != '<') {
        throw std::runtime_error("expected IRI");
    }
    auto end = line.find('>', pos + 1);
    if (end == std::string::npos) {
        throw std::runtime_error("unterminated IRI");
    }
    std::string value = line.substr(pos + 1, end - pos - 1);
    pos = end + 1;
    return Iri{value};
}

// Extract the BlankNode found at `pos` of the current `line` and parse it as a term
Term parseBlankNode(const std::string& line, std::size_t& pos) {
    if (!startsWith(line, pos, "_:")) {
        throw std::runtime_error("expected blank node");
    }
    std::size_t start = pos + 2;
    std::size_t end = start;
    while (end < line.size() &&
           std::isspace(static_cast<unsigned char>(line[end])) == 0) {
        ++end;
    }
    std::string id = line.substr(start, end - start);
    if (id.empty()) {
        throw std::runtime_error("empty blank node id");
    }
    pos = end;
    return BlankNode{id};
}

// Extract the Literal found at `pos` of the current `line` and parse it as a term
Term parseLiteral(const std::string& line, std::size_t& pos) {
    if (pos >= line.size() || line[pos] != '"') {
        throw std::runtime_error("expected literal");
    }
    auto end = line.find('"', pos + 1);
    if (end == std::string::npos) {
        throw std::runtime_error("unterminated literal");
    }
    Literal literal;
    literal.value = line.substr(pos + 1, end - pos - 1);
    pos = end + 1;
    if (pos < line.size() && line[pos] == '@') {
        ++pos;
        std::size_t start = pos;
        while (pos < line.size() &&
               std::isspace(static_cast<unsigned char>(line[pos])) == 0) {
            ++pos;
        }
        literal.language = line.substr(start, pos - start);
    } else if (startsWith(line, pos, "^^")) {
        pos += 2;
        Term datatype = parseIri(line, pos);
        literal.datatype = std::get<Iri>(datatype).value;
    }
    return literal;
}

// Parse the current triple's term which starts at the `pos` index of the current `line`
Term parseTerm(const std::string& line, std::size_t& pos) {
    skipWhitespace(line, pos);
    if (pos >= line.size()) {
        throw std::runtime_error("unexpected end of line");
    }
    if (line[pos] == '<') {
        return parseIri(line, pos);
    }
    if (line[pos] == '"') {
        return parseLiteral(line, pos);
    }

    // A `BlankNode` starts with two characters so it needs a special function
    if (startsWith(line, pos, "_:")) {
        return parseBlankNode(line, pos);
    }
    throw std::runtime_error("expected term");
}

}

void Database::loadNTriples(const std::string& path) {
    // Open the file containing the triples
    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("cannot open file: " + path);
    }

    // Set of triples to track seen triples before
    std::set<std::array<TermId, 3>> seen;
    std::string line;
    std::size_t lineNumber = 0;

    while (std::getline(input, line)) {
        // Increment the lines index and parse the triple
        ++lineNumber;
        std::size_t pos = 0;
        skipWhitespace(line, pos);

        // Avoid reading overflowing indices and comments
        if (pos >= line.size() || line[pos] == '#') {
            continue;
        }

        // Try to parse the three terms of the current triple with validation against the RDF rules
        try {
            Term subject = parseTerm(line, pos);
            if (std::holds_alternative<Literal>(subject)) {
                throw std::runtime_error("subject cannot be a literal");
            }

            Term predicate = parseTerm(line, pos);
            if (!std::holds_alternative<Iri>(predicate)) {
                throw std::runtime_error("predicate must be an IRI");
            }

            Term object = parseTerm(line, pos);

            skipWhitespace(line, pos);
            if (pos >= line.size() || line[pos] != '.') {
                throw std::runtime_error("expected '.'");
            }
            ++pos;
            skipWhitespace(line, pos);
            if (pos < line.size()) {
                throw std::runtime_error("unexpected content after '.'");
            }

            // Parse the current triple as an encoded triple and store it inside our
            // triple store database if it's not seen before
            TermId subjectId = dictionary_.intern(subject);
            TermId predicateId = dictionary_.intern(predicate);
            TermId objectId = dictionary_.intern(object);
            std::array<TermId, 3> key{subjectId, predicateId, objectId};
            if (seen.insert(key).second) {
                store_.add(EncodedTriple{subjectId, predicateId, objectId});
            }
        } catch (const std::exception& error) {
            throw std::runtime_error("line " + std::to_string(lineNumber) + ": " + error.what());
        }
    }

    // Rebuild the indexes in our database after initializing it
    indexes_.rebuild(store_.triples());
}

std::size_t Database::size() const {
    return store_.size();
}

// Returns the three indexes we have in our database for faster retrieval
const Indexes& Database::indexes() const {
    return indexes_;
}

}
