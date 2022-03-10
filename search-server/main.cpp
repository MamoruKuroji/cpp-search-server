// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(), "Stop words must be excluded from documents"s);
    }
}

/*
Разместите код остальных тестов здесь
*/
void TestAddingDocumentContent() {
    const int doc_id_0 = 0, doc_id_1 = 1, doc_id_2 = 2;
    const string document_text_0 = "ухоженный пёс выразительные глаза и модный ошейник"s;
    const string document_text_1 = "ухоженный кот выразительные глаза и модный ошейник"s;
    const string document_text_2 = "кот и модный ошейник"s;
    const vector<int> doc_ratings = { 1, 2, 3 };
    const DocumentStatus status = DocumentStatus::ACTUAL;
    SearchServer server;
    ASSERT_HINT(!server.GetDocumentCount(), "New search server has 0 documents");
    server.AddDocument(doc_id_0, document_text_0, status, doc_ratings);
    
    ASSERT_HINT(server.GetDocumentCount(), "New document not added");
    
    server.AddDocument(doc_id_1, document_text_1, status, doc_ratings);
    server.AddDocument(doc_id_2, document_text_2, status, doc_ratings);
    
    ASSERT_EQUAL_HINT(server.GetDocumentCount(), 3 , "Incorrect adding several documents");

    
}

void TestExcludeMinusWordsFromDocument() {
    SearchServer server;
    const string document_text = "ухоженный пёс выразительные глаза и модный ошейник"s;
    server.AddDocument(0, document_text, DocumentStatus::ACTUAL, { 5 });

    const std::string query = "пёс выразительные глаза -ошейник"s;
    auto [matched_words, status] = server.MatchDocument(query, 0);
    ASSERT_HINT(matched_words.empty(), "Minus words don't work");

}

void TestMatchingDocumentContentWithSearchingContent() {
    SearchServer server;
    const int doc_id = 0;
    const vector<int> ratings = { 5 };
    const string document_text = "ухоженный пёс выразительные глаза и модный ошейник"s;
    server.AddDocument(doc_id, document_text, DocumentStatus::ACTUAL, ratings);
    const string query = "пёс"s;

    // Убеждаемся, что документ найден по плюс-слову
    {
        auto [match, status] = server.MatchDocument(query, doc_id);
        vector<string> expect = { "пёс"s };
        ASSERT_EQUAL_HINT(expect, match, "Incorrect search for plus words");
    }
}

void TestSortingDocumentContentByRelevance() {    
    SearchServer server;
    int doc_id = 1, doc_id_1 = 2;
    const string document_text = "ухоженный пёс выразительные глаза и модный ошейник"s;
    server.AddDocument(doc_id, document_text, DocumentStatus::ACTUAL, { 5 , 6 , 7 });
    const string document_text_2 = "белый кот и модный ошейник"s;
    server.AddDocument(doc_id_1, document_text_2, DocumentStatus::ACTUAL, { 5 , 6 , 7 });
    const auto found_docs = server.FindTopDocuments("кот и ошейник"s);
    //assert(found_docs.size() == 2);
    const Document& doc0 = found_docs[0];
    const Document& doc1 = found_docs[1];
    ASSERT_EQUAL_HINT(doc0.id, doc_id_1, "Incorrect searching by relevance");
    ASSERT_EQUAL_HINT(doc1.id, doc_id, "Incorrect searching by relevance");
    bool isDecrease = false;
    if(doc0.relevance > doc1.relevance){
        isDecrease = true;
    }
    ASSERT_HINT(isDecrease, "Relevance don't decrease");
}

void TestCalculateRatingOfDocumentContent() {    
    SearchServer server;
    const string document_text = "ухоженный пёс выразительные глаза и модный ошейник"s;
    server.AddDocument(0, document_text, DocumentStatus::ACTUAL, { 5 , 6 , -7 });

    const auto found_docs = server.FindTopDocuments("пёс выразительные глаза"s);
    //assert(found_docs.size() == 1);
    const Document& doc0 = found_docs[0];
    ASSERT_EQUAL_HINT(doc0.rating, ((5 + 6 + (-7)) / 3), "Incorrect calculating rating");
}


void TestFiltrationDocumentContentWithUserPredicate() {
    SearchServer server;
    server.SetStopWords("и в на"s);
    int doc_id_0 = 0, doc_id_1 = 1, doc_id_2 = 2, doc_id_3 = 3;
    server.AddDocument(doc_id_0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    server.AddDocument(doc_id_1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    server.AddDocument(doc_id_2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    server.AddDocument(doc_id_3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
    const auto found_docs = server.FindTopDocuments("пушистый ухоженный кот"s, []([[maybe_unused]] int document_id, [[maybe_unused]] DocumentStatus status, [[maybe_unused]] int rating) { return document_id % 2 == 0; });
    //assert(found_docs.size() == 2);
    const Document& doc0 = found_docs[0];
    const Document& doc1 = found_docs[1];
    ASSERT_EQUAL_HINT(doc0.id, doc_id_0, "Incorrect filtration with user predicate");
    ASSERT_EQUAL_HINT(doc1.id, doc_id_2, "Incorrect filtration with user predicate");
}

void TestSearchDocumentContentWithAssignedStatus() {
    SearchServer server;
    const string document_text = "ухоженный пёс выразительные глаза и модный ошейник"s;
    server.AddDocument(0, document_text, DocumentStatus::ACTUAL, { 5 , 6 , 7 });
    server.AddDocument(1, document_text, DocumentStatus::BANNED, { 5 , 6 , 7 });
    server.AddDocument(2, document_text, DocumentStatus::IRRELEVANT, { 5 , 6 , 7 });
    server.AddDocument(3, document_text, DocumentStatus::REMOVED, { 5 , 6 , 7 });

    const auto found_docs_ACTUAL = server.FindTopDocuments("пёс выразительные глаза"s, DocumentStatus::ACTUAL);
    const auto found_docs_BANNED = server.FindTopDocuments("пёс выразительные глаза"s, DocumentStatus::BANNED);
    const auto found_docs_IRRELEVANT = server.FindTopDocuments("пёс выразительные глаза"s, DocumentStatus::IRRELEVANT);
    const auto found_docs_REMOVED = server.FindTopDocuments("пёс выразительные глаза"s, DocumentStatus::BANNED);
    ASSERT_HINT(found_docs_ACTUAL.size(), "Incorrect search with ACTUAL status");
    ASSERT_HINT(found_docs_BANNED.size(), "Incorrect search with BANNED status");
    ASSERT_HINT(found_docs_IRRELEVANT.size(), "Incorrect search with IRRELEVANT status");
    ASSERT_HINT(found_docs_REMOVED.size(), "Incorrect search with REMOVED status");
}

void TestCalculateRelevanceOfDocumentContent() {
    SearchServer server;
    const string document_text = "ухоженный пёс выразительные глаза и модный ошейник"s;
    server.AddDocument(0, document_text, DocumentStatus::BANNED, { 5 , 6 , 7 });

    const auto found_docs = server.FindTopDocuments("пёс выразительные глаза"s, DocumentStatus::BANNED);
    const Document& doc0 = found_docs[0];
    const double EPSILON = 1e-6;
    bool isEqual = false;
    if(abs(doc0.relevance - ((log(1 * 1.0 / 1))*3)) < EPSILON) {
        isEqual = true;
    }
    ASSERT_HINT(isEqual, "Incorrect calculate relevance");
    
}

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    // Не забудьте вызывать остальные тесты здесь
    RUN_TEST(TestAddingDocumentContent);
    RUN_TEST(TestExcludeMinusWordsFromDocument);
    RUN_TEST(TestMatchingDocumentContentWithSearchingContent);
    RUN_TEST(TestSortingDocumentContentByRelevance);
    RUN_TEST(TestCalculateRatingOfDocumentContent);
    RUN_TEST(TestFiltrationDocumentContentWithUserPredicate);
    RUN_TEST(TestSearchDocumentContentWithAssignedStatus);
}

// --------- Окончание модульных тестов поисковой системы -----------
