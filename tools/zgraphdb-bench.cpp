#include <chrono>
#include <cstddef>
#include <iostream>
#include <optional>
#include <vector>

#include "zgraphdb/storage/indexes.hpp"

namespace {

using Clock = std::chrono::steady_clock;

std::vector<zgraphdb::EncodedTriple> fullScan(
    const std::vector<zgraphdb::EncodedTriple>& triples,
    const zgraphdb::TriplePattern& pattern) {
    std::vector<zgraphdb::EncodedTriple> result;
    for (const auto& triple : triples) {
        if (pattern[0].has_value() && triple.subject != *pattern[0]) {
            continue;
        }
        if (pattern[1].has_value() && triple.predicate != *pattern[1]) {
            continue;
        }
        if (pattern[2].has_value() && triple.object != *pattern[2]) {
            continue;
        }
        result.push_back(triple);
    }
    return result;
}

}

int main() {
    constexpr std::size_t tripleCount = 100000;
    constexpr std::size_t repeats = 20;
    constexpr zgraphdb::TermId targetPredicate = 7;

    std::vector<zgraphdb::EncodedTriple> triples;
    triples.reserve(tripleCount);
    for (std::size_t i = 0; i < tripleCount; ++i) {
        triples.push_back({
            static_cast<zgraphdb::TermId>(i),
            static_cast<zgraphdb::TermId>(i % 10),
            static_cast<zgraphdb::TermId>((i * 7) % 1000),
        });
    }

    zgraphdb::Indexes indexes;
    indexes.rebuild(triples);

    const zgraphdb::TriplePattern pattern{std::nullopt, targetPredicate, std::nullopt};

    std::size_t scanSink = 0;
    const auto scanStart = Clock::now();
    for (std::size_t i = 0; i < repeats; ++i) {
        scanSink += fullScan(triples, pattern).size();
    }
    const auto scanEnd = Clock::now();

    std::size_t indexSink = 0;
    const auto indexStart = Clock::now();
    for (std::size_t i = 0; i < repeats; ++i) {
        indexSink += indexes.lookup(pattern).size();
    }
    const auto indexEnd = Clock::now();

    const auto scanMs = std::chrono::duration<double, std::milli>(scanEnd - scanStart).count();
    const auto indexMs = std::chrono::duration<double, std::milli>(indexEnd - indexStart).count();
    const std::size_t scanMatches = scanSink / repeats;
    const std::size_t indexMatches = indexSink / repeats;

    std::cout << "triples: " << tripleCount << "\n";
    std::cout << "pattern: ?s <p" << targetPredicate << "> ?o\n";
    std::cout << "full scan: " << scanMs << " ms (" << scanMatches << " matches)\n";
    std::cout << "index:     " << indexMs << " ms (" << indexMatches << " matches)\n";
    if (indexMs > 0.0) {
        std::cout << "speedup:   " << scanMs / indexMs << "x\n";
    }

    return scanMatches == indexMatches ? 0 : 1;
}
