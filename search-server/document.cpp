//Вставьте сюда своё решение из урока «‎Очередь запросов».‎

#include <iostream>

#include "document.h"

using namespace std::string_literals;

std::ostream& operator<<(std::ostream& out, const Document& document) {
    out << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s;
    return out;
}

Document::Document(int id, double relevance, int rating)
    : id(id)
    , relevance(relevance)
    , rating(rating) {
}

void PrintDocument(const Document& document) {

    std::cout << "{ "s

        << "document_id = "s << document.id << ", "s

        << "relevance = "s << document.relevance << ", "s

        << "rating = "s << document.rating << " }"s << std::endl;

}

void PrintMatchDocumentResult(int document_id, const std::vector<std::string>& words, DocumentStatus status) {

    std::cout << "{ "s

        << "document_id = "s << document_id << ", "s

        << "status = "s << static_cast<int>(status) << ", "s

        << "words ="s;

    for (const std::string& word : words) {

        std::cout << ' ' << word;

    }

    std::cout << "}"s << std::endl;

}