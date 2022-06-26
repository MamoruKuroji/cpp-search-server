#pragma once
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>

#include "document.h"

using std::ostream;

std::vector<std::string> SplitIntoWords(const std::string_view text);

std::vector<std::string_view> SplitIntoWordsView(std::string_view text);

template <typename StringContainer>
std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    std::set<std::string> non_empty_strings;
    for (const std::string& str : strings) {
        if (!str.empty()) {
            non_empty_strings.insert(str);
        }
    }
    return non_empty_strings;
}