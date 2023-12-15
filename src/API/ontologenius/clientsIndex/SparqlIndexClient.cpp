#include "ontologenius/API/ontologenius/clientsIndex/SparqlIndexClient.h"

namespace onto {

    std::pair<std::vector<std::string>, std::vector<ontologenius::compat::OntologeniusSparqlIndexResponse>> SparqlIndexClient::call(const std::string& query)
    {
        ontologenius::compat::RequestType<ontologenius::compat::OntologeniusSparqlIndexService> req;
        req->query = query;

        ontologenius::compat::ResponseType<ontologenius::compat::OntologeniusSparqlIndexService> res;


        using ResultTy = typename decltype(client_)::Status;

        switch (client_.call(req, res)) {
            case ResultTy::SUCCESSFUL_WITH_RETRIES:
            case ResultTy::SUCCESSFUL:
            {
                return { res->names, res->results };
            }
            case ResultTy::FAILURE:
            default:
            {
                return { };
            }
        }
    }

} // namespace onto
