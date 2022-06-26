#include "search_server.h"

    
SearchServer::SearchServer(const std::string& stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text)) {
}
    

void SearchServer::AddDocument(int document_id, 
                               const std::string_view document, 
                               DocumentStatus status, 
                               const std::vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw std::invalid_argument("Invalid document_id"s);
    }
    std::string str(document);
    const auto words = SplitIntoWordsNoStop(str);
    const double inv_word_count = 1.0 / words.size();
    for (const std::string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        ids_word_freqs_[document_id][word] += 0;
    }    
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_ids_.insert(document_id);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(
    const std::string_view
    raw_query, int document_id) const {
    return SearchServer::MatchDocument
        (std::execution::seq, raw_query, document_id);
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(
        std::execution::parallel_policy policy, 
        const std::string_view raw_query, 
        int document_id) const {
    std::vector<std::string_view> matched_words;
    if (document_id >= 0) {
        static auto query_par = ParParseQuery(raw_query);
        
        auto check = find_if(std::execution::seq, 
                             query_par.minus_words.begin(), 
                             query_par.minus_words.end(),
            [&] (const std::string& word) {
                return word_to_document_freqs_.at(word).count(document_id);
            });
            if (check != query_par.minus_words.end()) {
                return std::tuple(matched_words, documents_.at(document_id).status);
            }
        
        matched_words.reserve(query_par.plus_words.size());
        for (const std::string_view word : query_par.plus_words) {
            if ((word_to_document_freqs_.count(word) != 0)
            &&word_to_document_freqs_.at(static_cast<std::string>(word)).count(document_id)) {
            matched_words.insert(matched_words.end(), word);
        }
    }
        std::sort(policy, matched_words.begin(), matched_words.end());
        matched_words.erase(std::unique(matched_words.begin(), 
                                        matched_words.end()), matched_words.end());
    } else {
        throw std::out_of_range("id");
    }
    return {matched_words, documents_.at(document_id).status};
}
    
std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument
        (std::execution::sequenced_policy policy, const std::string_view raw_query, int document_id) const {
    if(!document_ids_.count(document_id)){
            throw std::out_of_range("Документ не существует");
        }
    static Query query_;
    query_ = ParseQuery(raw_query);
    std::vector<std::string_view> matched_words;
    matched_words.reserve(query_.plus_words.size());
    
    for (const std::string& word : query_.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            query_.plus_words.clear();
            break;
        }
    }
    for (const std::string& word : query_.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    return {matched_words, documents_.at(document_id).status};
    }
    
bool SearchServer::IsStopWord(const std::string_view word) const {
    return stop_words_.count(static_cast<std::string>(word)) > 0;
}

bool SearchServer::IsValidWord(const std::string_view word) {
       // A valid word must not contain special characters
    return std::none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}

std::vector<std::string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    std::vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw std::invalid_argument("Word "s + word + " is invalid"s);
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}
int SearchServer::ComputeAverageRating(const std::vector<int>& ratings) {
    return std::accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(const std::string_view text) const {
    if (text.empty()) {
        throw std::invalid_argument("Query word is empty"s);
    }
    std::string_view word(text);
    bool is_minus = false;
    if (word[0] == '-') {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
        throw std::invalid_argument("Query word "s + static_cast<std::string>(text) + " is invalid");
    }
    return {word, is_minus, IsStopWord(word)};
}

SearchServer::Query SearchServer::ParseQuery(const std::string_view text) const {
    Query result;
    for (const std::string_view word : SplitIntoWordsView(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.insert(static_cast<std::string>(query_word.data));
            } else {
                result.plus_words.insert(static_cast<std::string>(query_word.data));
            }
        }
    }
    return result;
}

SearchServer::ParQuery SearchServer::ParParseQuery(const std::string_view text) const {
    ParQuery result;
    result.plus_words.reserve(500);
    result.minus_words.reserve(500);
    for (const std::string_view word : SplitIntoWordsView(text)) {
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.push_back(static_cast<std::string>(query_word.data));
            } else {
                result.plus_words.push_back(static_cast<std::string>(query_word.data));
            }
        }
    }
    return result;
}

    // Existence required
double SearchServer::ComputeWordInverseDocumentFreq(const std::string_view word) const {
    return std::log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(static_cast<std::string>(word)).size());
}

const std::map<std::string_view, double> SearchServer::GetWordFrequencies(int document_id) const {
    if(!document_ids_.count(document_id)){
        return empty_word_freqs_;
    }
    std::map<std::string_view, double> word_freqs;
    for(auto [i, j] : ids_word_freqs_.at(document_id)){
        word_freqs[i] = j;
    }
    
    return word_freqs;
}

void SearchServer::RemoveDocument(int document_id) {
    SearchServer::RemoveDocument(std::execution::seq, document_id);
}

void SearchServer::RemoveDocument(std::execution::sequenced_policy policy, int document_id) {
    if(document_ids_.count(document_id)) {
        for_each(policy, 
                 ids_word_freqs_.at(document_id).begin(), 
                 ids_word_freqs_.at(document_id).end(), 
                 [&](std::pair<std::string, double> words) {
            word_to_document_freqs_[words.first].erase(document_id);
        });
        documents_.erase(document_id);
        document_ids_.erase(document_id);
        ids_word_freqs_.erase(document_id);
    }
}
    
void SearchServer::RemoveDocument(std::execution::parallel_policy policy, int document_id) {
    if(document_ids_.count(document_id)) {
        std::vector<const std::string*> it_to_words;
        it_to_words.resize(ids_word_freqs_[document_id].size());
        
        std::transform(policy, ids_word_freqs_[document_id].begin(),
                       ids_word_freqs_[document_id].end(), it_to_words.begin(),
                       [](const auto& word) { return &word.first;
        });
        
        for_each(policy, 
                 it_to_words.begin(), 
                 it_to_words.end(), 
                 [&](auto word) {
            word_to_document_freqs_[*word].erase(document_id);
        });
        
        documents_.erase(document_id);
        document_ids_.erase(document_id);
        ids_word_freqs_.erase(document_id);
    }
}


std::set<int>::iterator SearchServer::begin(){
    return document_ids_.begin();
}

std::set<int>::iterator SearchServer::end(){
    return document_ids_.end();
}
