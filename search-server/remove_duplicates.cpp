// в качестве заготовки кода используйте последнюю версию своей поисковой системы

#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server){
    std::vector<int> ids_to_delete;
    std::set<std::map<std::string, double>> not_to_delete;
    for(const int document_id : search_server){
        auto element = search_server.GetWordFrequencies(document_id);
        if(not_to_delete.count(element)){
            ids_to_delete.push_back(document_id);
        } else {
            not_to_delete.insert(element);
        }
    }
    for(auto value : ids_to_delete){
        std::cout << "Found duplicate document id " << value << std::endl;
        search_server.RemoveDocument(value);
    }
}