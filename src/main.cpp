#include "descriptions.h"
#include "json.h"
#include "requests.h"
#include "sphere.h"
#include "transport_catalog.h"
#include "utils.h"

#include <iostream>

using namespace std;

int main() {
    auto const input_doc = Json::load(cin);
    auto const & input_map = input_doc.getRoot().asMap();

    TransportCatalog const db(
        Descriptions::readDescriptions(input_map.at("base_requests").asArray()),
        input_map.at("routing_settings").asMap()
    );

    Json::printValue(
        Requests::processAll(db, input_map.at("stat_requests").asArray()),
        cout
    );
    cout << endl;

    return 0;
}
