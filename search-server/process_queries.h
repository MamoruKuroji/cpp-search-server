#pragma once

#include <vector>
#include <string>
#include <execution>
#include <algorithm>
#include <numeric>
#include <functional>
#include <list>

#include "document.h"
#include "search_server.h"

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries);

std::list<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries); 