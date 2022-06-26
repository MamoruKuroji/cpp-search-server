#include "string_processing.h"

#include <algorithm>
#include <execution>

std::vector<std::string> SplitIntoWords(const std::string_view text) {
    std::vector<std::string> words;
    words.reserve(500);
    std::string word;
    word.reserve(11);
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    
    return words;
}

std::vector<std::string_view> SplitIntoWordsView(std::string_view text) {
    std::vector<std::string_view> result;
    result.reserve(500);
    int64_t pos = text.find_first_not_of(" ");
    const int64_t pos_end = text.npos;
    //int count_space = 0;
    
    //text.remove_prefix(count_space);
    while (pos != pos_end) {
        int64_t space = text.find(' ', pos);
        result.push_back(space == pos_end ? text.substr(pos) : text.substr(pos, space - pos));
        pos = text.find_first_not_of(" ", space);
    }
    
    return result;
}