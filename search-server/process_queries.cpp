#include "process_queries.h"

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries) {
        std::vector<std::vector<Document>> result;
        result.resize(queries.size());
        transform(std::execution::par,
            queries.begin(), queries.end(),
            result.begin(),
            [&search_server] (const std::string& querie) {
                return search_server.FindTopDocuments(querie);
            });
 
        return result;
    }
/* набор объектов Document */
std::list<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
    const std::vector<std::string>& queries){
    
        std::vector<std::list<Document>> result_;
        result_.resize(queries.size());
        transform(std::execution::par,
            queries.begin(), queries.end(),
            result_.begin(),
            [&search_server] (const std::string& querie) {
                std::vector<Document> it = search_server.FindTopDocuments(querie);
                std::list<Document> puls;
                for(auto c : it){
                    puls.push_back(c);
                }
                return puls;
            });
    
    std::list<Document> result;
        for(auto c : result_){
            result.splice(result.end(), std::move(c));
        }
 
        return result;
}