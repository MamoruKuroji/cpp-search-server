#pragma once
#include <iostream>

#include <vector>
#include <map>
#include <set>
#include <string>
#include <list>
#include <utility>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <iterator>
#include <execution>
#include <future>

#include "document.h"
#include "read_input_functions.h"
#include "string_processing.h"
#include "concurrent_map.h"

using namespace std::string_literals;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

class SearchServer {
public:
    template <typename StringContainer>
    SearchServer(const StringContainer& stop_words);
    
    explicit SearchServer(const std::string& stop_words_text);
    explicit SearchServer(const std::string_view stop_words_text);
    
 
    void AddDocument(int document_id, const std::string_view document, DocumentStatus status,
                     const std::vector<int>& ratings);
    
    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string_view raw_query,
        DocumentPredicate document_predicate) const ;
    
    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(std::execution::sequenced_policy policy, const std::string_view raw_query,
        DocumentPredicate document_predicate) const ;
    
    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(std::execution::parallel_policy policy, const std::string_view raw_query,
        DocumentPredicate document_predicate) const ;
 
    std::vector<Document> FindTopDocuments(const std::string_view raw_query, DocumentStatus status) const {
        return FindTopDocuments(std::execution::seq, 
            raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
                return document_status == status;
            });
    }
    
    std::vector<Document> FindTopDocuments(std::execution::sequenced_policy policy, const std::string_view raw_query, DocumentStatus status) const {
        return FindTopDocuments(policy, 
            raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
                return document_status == status;
            });
    }
    
    std::vector<Document> FindTopDocuments(std::execution::parallel_policy policy, const std::string_view raw_query, DocumentStatus status) const {
        return FindTopDocuments(policy, 
            raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
                return document_status == status;
            });
    }
 
    std::vector<Document> FindTopDocuments(const std::string_view raw_query) const {
        return FindTopDocuments(std::execution::seq, raw_query, DocumentStatus::ACTUAL);
    }
    
    std::vector<Document> FindTopDocuments(std::execution::sequenced_policy policy, const std::string_view raw_query) const {
        return FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
    }
    
    std::vector<Document> FindTopDocuments(std::execution::parallel_policy policy, const std::string_view raw_query) const {
        return FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
    }
    
    int GetDocumentCount() const;
 
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(const std::string_view raw_query, int document_id) const;
    
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument
        (std::execution::sequenced_policy policy, const std::string_view raw_query, int document_id) const;
    
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument
        (std::execution::parallel_policy policy, const std::string_view raw_query, int document_id) const;
    
    const std::map<std::string_view, double> GetWordFrequencies(int document_id) const;
    
    void RemoveDocument(int document_id);
    
    void RemoveDocument(std::execution::sequenced_policy policy, int document_id);
    
    void RemoveDocument(std::execution::parallel_policy policy, int document_id);
    
    std::set<int>::iterator begin();
    
    std::set<int>::iterator end();
    
private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    
    const std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>, std::less<>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::set<int> document_ids_;
    std::map<int, std::map<std::string, double>> ids_word_freqs_;
    std::map<std::string_view, double> empty_word_freqs_ = {};
    
    bool IsStopWord(const std::string_view word) const;
    
    static bool IsValidWord(const std::string_view word);
    
    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;
    
    static int ComputeAverageRating(const std::vector<int>& ratings);
    
    struct QueryWord {
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };
    
    QueryWord ParseQueryWord(const std::string_view text) const;
    
    struct Query {
        std::set<std::string, std::less<>> plus_words;
        std::set<std::string, std::less<>> minus_words;
    };
    struct ParQuery {
        std::vector<std::string> plus_words;
        std::vector<std::string> minus_words;
    };
    
    Query ParseQuery(const std::string_view text) const;
    
    ParQuery ParParseQuery(const std::string_view text) const;
    
    double ComputeWordInverseDocumentFreq(const std::string_view word) const;
 
    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(std::execution::sequenced_policy policy, Query& query, DocumentPredicate document_predicate) const;
    
    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(std::execution::parallel_policy policy, ParQuery& query, DocumentPredicate document_predicate) const;
    
};

template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words)
    : stop_words_(MakeUniqueNonEmptyStrings(stop_words))  // Extract non-empty stop words
{
    if (!std::all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
        throw std::invalid_argument("Some of stop words are invalid"s);
    }
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query,
        DocumentPredicate document_predicate) const {
    return FindTopDocuments(std::execution::seq, raw_query,
    document_predicate);
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(std::execution::sequenced_policy policy, const std::string_view raw_query,
    DocumentPredicate document_predicate) const {
    auto query = ParseQuery(raw_query);

    auto matched_documents = FindAllDocuments(policy, query, document_predicate);

    std::sort(policy, matched_documents.begin(), matched_documents.end(),
        [](const Document& lhs, const Document& rhs) {
            return lhs.relevance > rhs.relevance
                || (std::abs(lhs.relevance - rhs.relevance) < EPSILON && lhs.rating > rhs.rating);
        });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(std::execution::parallel_policy policy, const std::string_view raw_query,
    DocumentPredicate document_predicate) const {
    auto query = ParParseQuery(raw_query);

    auto matched_documents = FindAllDocuments(policy, query, document_predicate);

    std::sort(policy, matched_documents.begin(), matched_documents.end(),
        [](const Document& lhs, const Document& rhs) {
            return lhs.relevance > rhs.relevance
                || (std::abs(lhs.relevance - rhs.relevance) < EPSILON && lhs.rating > rhs.rating);
        });
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(std::execution::sequenced_policy policy, Query& query,
    DocumentPredicate document_predicate) const {
    std::map<int, double> document_to_relevance;
    for (const std::string_view word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto [document_id, term_freq] : word_to_document_freqs_.at(static_cast<std::string>(word))) {
            const auto& document_data = documents_.at(document_id);
            if (document_predicate(document_id, document_data.status, document_data.rating)) {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
    }
    for (const std::string_view word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto [document_id, _] : word_to_document_freqs_.at(static_cast<std::string>(word))) {
            document_to_relevance.erase(document_id);
        }
    }

    std::vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance) {
        matched_documents.push_back(
            { document_id, relevance, documents_.at(document_id).rating });
    }
    return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(std::execution::parallel_policy policy, ParQuery& query, DocumentPredicate document_predicate) const {
    ConcurrentMap<int, double> document_to_relevance(100);
    
    std::sort(query.plus_words.begin(), query.plus_words.end());
    std::sort(query.minus_words.begin(), query.minus_words.end());
    
    query.plus_words.erase(std::unique(query.plus_words.begin(), query.plus_words.end()), query.plus_words.end());    
    query.minus_words.erase(std::unique(query.minus_words.begin(), query.minus_words.end()), query.minus_words.end());
    
    for_each (policy, query.plus_words.begin(), query.plus_words.end(), [&](const std::string_view word) {
        if (word_to_document_freqs_.count(word) == 0) {
            return;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto [document_id, term_freq] : word_to_document_freqs_.at(static_cast<std::string>(word))) {
            const auto& document_data = documents_.at(document_id);
            if (document_predicate(document_id, document_data.status, document_data.rating)) {
                document_to_relevance[document_id].ref_to_value += term_freq * inverse_document_freq;
            }
        }
    });
    
    auto ordinary_map = document_to_relevance.BuildOrdinaryMap();
    for (const std::string_view word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto [document_id, _] : word_to_document_freqs_.at(static_cast<std::string>(word))) {
            ordinary_map.erase(document_id);
        }
    }
    
    
    std::vector<Document> matched_documents;
    matched_documents.reserve(ordinary_map.size());
    for_each (ordinary_map.begin(), ordinary_map.end(), [&](const auto& doc_map) {
        matched_documents.push_back(
            { doc_map.first, doc_map.second, documents_.at(doc_map.first).rating });
    });
    return matched_documents;
}
