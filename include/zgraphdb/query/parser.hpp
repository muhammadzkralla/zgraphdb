#pragma once

#include <vector>
#include "zgraphdb/query/ast.hpp"
#include "zgraphdb/query/lexer.hpp"

namespace zgraphdb {

QueryDocument parseQuery(const std::vector<Token>& tokens);

}
