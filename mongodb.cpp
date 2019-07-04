#include <cstdint>
#include <iostream>
#include <vector>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/builder/core.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/instance.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
int main()
{
    mongocxx::instance instance{}; // This should be done only once.
    mongocxx::uri uri("mongodb://192.168.0.27:27018");
    mongocxx::client cli(uri);

    mongocxx::database db = cli["mydb"];

    mongocxx::collection coll = db["test"];
    auto builder = bsoncxx::builder::stream::document{};
    bsoncxx::document::value doc_value = builder
        << "name" << "MongoDB"
        << "type" << "database"
        << "count" << 1
        << "versions" << bsoncxx::builder::stream::open_array
        << "v3.2" << "v3.0" << "v2.6"
        << close_array
        << "info" << bsoncxx::builder::stream::open_document
        << "x" << 203
        << "y" << 102
        << bsoncxx::builder::stream::close_document
        << bsoncxx::builder::stream::finalize;
    
    bsoncxx::document::view view = doc_value.view();

    bsoncxx::document::element element = view["name"];
    if(element.type() != bsoncxx::type::k_utf8) {
        // Error
    }
    std::string name = element.get_utf8().value.to_string();

    coll.insert_one(view);

    std::vector<bsoncxx::document::value> documents;
    for(int i = 0; i < 100; i++) {
        documents.push_back(
                bsoncxx::builder::stream::document{} << "i" << i << finalize);
    }

    coll.insert_many(documents);

    bsoncxx::stdx::optional<bsoncxx::document::value> maybe_result =
        coll.find_one({});
    if(maybe_result) {
        // Do something with *maybe_result;
    }

    mongocxx::cursor cursor = coll.find({});
    for(auto doc : cursor) {
        std::cout << bsoncxx::to_json(doc) << "\n";
    }


    maybe_result = coll.find_one(document{} << "i" << 71 << finalize);
    if(maybe_result) {
        std::cout << bsoncxx::to_json(*maybe_result) << "\n";
    }

   cursor = coll.find(
            document{} << "i" << open_document <<
            "$gt" << 50 <<
            "$lte" << 100
            << close_document << finalize);
    for(auto doc : cursor) {
        std::cout << bsoncxx::to_json(doc) << "\n";
    }

    coll.update_one(document{} << "i" << 10 << finalize,
            document{} << "$set" << open_document <<
            "i" << 110 << close_document << finalize);

    bsoncxx::stdx::optional<mongocxx::result::update> result =
        coll.update_many(
                document{} << "i" << open_document <<
                "$lt" << 100 << close_document << finalize,
                document{} << "$inc" << open_document <<
                "i" << 100 << close_document << finalize);

    if(result) {
        std::cout << result->modified_count() << "\n";
    }

/*    coll.delete_one(document{} << "i" << 110 << finalize);

    bsoncxx::stdx::optional<mongocxx::result::delete_result> result_del = coll.delete_many(
                document{} << "i" << open_document <<
                "$gte" << 100 << close_document << finalize);

    if(result_del) {
        std::cout << result_del->deleted_count() << "\n";
    }
*/
    auto index_specification = document{} << "i" << 1 << finalize;
    coll.create_index(std::move(index_specification));

}
