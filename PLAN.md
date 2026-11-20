# zgraphdb — Project Plan

## Goal
Build a small C++ graph database engine to learn graph-database and graph-lakehouse
concepts. The engine implements an RDF triple store with a SPARQL 1.1 query subset,
mirroring the architecture of Siemens' RapidMiner Graph Lakehouse (formerly
AnzoGraph / Altair Graph Lakehouse, written in C++, RDF-native, SPARQL 1.1 primary).

## Confirmed decisions
- Language: C++ (matches the real Siemens engine)
- Data model: RDF (IRI / blank node / literal terms; triples; named graphs)
- Query language: SPARQL 1.1 subset only
- Architecture: native engine first; lakehouse projection added later
- Scope: single-node learning toy — correctness and understanding over performance

## Background (RDF + SPARQL in brief)
- RDF expresses facts as triples: (subject, predicate, object), e.g.
  `<http://example.org/alice> <http://xmlns.com/foaf/0.1/name> "Alice" .`
- Terms: IRI = globally unique name (looks like a URL but is just an ID; `<...>`
  are delimiters, not XML tags); literal = value (`"Alice"`, `42`, typed values);
  blank node = anonymous ID (`_:b1`). No separate "node properties" — everything
  is a triple. Predicates are always IRIs.
- A set of triples = an RDF graph. SPARQL queries it by matching triple patterns
  containing variables.
- N-Triples / Turtle are the serialization formats we'll use. RDF/XML is skipped.
- Graph lakehouse = data stays in tables; a mapping exposes it as virtual RDF
  queried in place with pushdown (Siemens calls its mapping/ingestion layer GDI).

## Core components
1. RDF model — term types (IRI, blank node, literal with optional datatype/lang),
   triple, dataset with named graphs.
2. Storage — dictionary encoding (term -> 32-bit ID), in-memory triple vectors,
   N-Triples/Turtle loader.
3. Indexes — SPO / POS / OSP permutations for lookups with a variable in any
   position; basic selectivity counts.
4. SPARQL parser — hand-written lexer + parser -> AST: PREFIX/BASE, SELECT, ASK,
   WHERE, triple patterns, variables, FILTER, OPTIONAL, UNION, DISTINCT, ORDER BY,
   LIMIT, OFFSET.
5. Query evaluator — Basic Graph Pattern (BGP) evaluation via index lookups and
   hash joins; expression evaluation for FILTERs; solution modifiers.
6. Optimizer — BGP triple-pattern reordering by selectivity; join ordering.
7. Updates & durability — SPARQL 1.1 Update (INSERT DATA, DELETE DATA), write-ahead
   log, snapshot reload.
8. Semantic extras — named graphs, simple SHACL-style validation, graph algorithms
   (BFS, shortest path, PageRank) as SPARQL extension functions.
9. Lakehouse layer — CSV/Parquet reader + R2RML-like mapping rules -> virtual RDF
   graph; translate triple patterns into table scans with pushdown.
10. Interfaces — CLI REPL for SPARQL, programmatic C++ API, optional HTTP SPARQL
    Protocol endpoint.

## Target SPARQL subset
- PREFIX / BASE, SELECT (variables and *), ASK
- WHERE: triple patterns, FILTER (=, !=, <, >, BOUND, REGEX, &&, ||), OPTIONAL, UNION
- Modifiers: DISTINCT, ORDER BY, LIMIT, OFFSET
- Updates (Phase 5): INSERT DATA, DELETE DATA

## Project structure sketch
zgraphdb/
  CMakeLists.txt
  include/zgraphdb/
    model/       term.hpp, triple.hpp, dataset.hpp
    storage/     dictionary.hpp, triple_store.hpp, indexes.hpp
    query/       lexer.hpp, parser.hpp, ast.hpp, evaluator.hpp, optimizer.hpp
    update/      update.hpp
    lakehouse/   mapping.hpp, csv_reader.hpp
    api/         db.hpp
  src/           matching implementation
  tests/         unit + query tests
  examples/      social.ntriples, queries.rq
  tools/         zgraphdb-cli.cpp

## Roadmap and checkpoints
Phase 0 — Foundations
  Work: RDF/SPARQL primer, CMake project skeleton, lock the supported subset.
  Checkpoint: write a few SPARQL queries by hand against a sample graph.

Phase 1 — RDF store core
  Work: term types, dictionary encoding, in-memory triple store, N-Triples loader.
  Checkpoint: load a small RDF file and print all triples.

Phase 2 — Indexes
  Work: SPO/POS/OSP permutations, indexed pattern lookup, selectivity counts.
  Checkpoint: demonstrate indexed lookup beating a full scan.

Phase 3 — SPARQL read queries
  Work: lexer/parser/AST, BGP evaluation with joins.
  Checkpoint: run `SELECT ?name WHERE { ?p <knows> ?q . ?q <name> ?name }`.

Phase 4 — Richer SPARQL
  Work: FILTER, OPTIONAL, UNION, DISTINCT, ORDER BY, LIMIT; BGP optimization.
  Checkpoint: complex multi-pattern queries return correct results.

Phase 5 — Updates and durability
  Work: SPARQL Update, WAL, snapshot reload.
  Checkpoint: restart the process and confirm data survived.

Phase 6 — Semantic extras
  Work: named graphs, SHACL-style validation, graph algorithms as SPARQL functions.
  Checkpoint: run a shortest-path query via SPARQL.

Phase 7 — Lakehouse projection
  Work: table reader + mapping config -> virtual graph, pushdown of triple patterns.
  Checkpoint: query tables as RDF with no ingestion step.

Phase 8 — Polish
  Work: CLI/HTTP endpoint, tests, benchmarks, docs.
  Checkpoint: full test suite green; simple README with usage examples.

Each phase ends at a checkpoint small enough to implement yourself or delegate.

## Learning references
- W3C RDF 1.1 Primer and SPARQL 1.1 specification — the standards being implemented.
- "Learning SPARQL" by Bob DuCharme — gentle query-language introduction.
- Hexastore / RDF-3X papers — theory behind triple index permutations.
- Siemens RapidMiner Graph Lakehouse docs — real architecture for comparison.
