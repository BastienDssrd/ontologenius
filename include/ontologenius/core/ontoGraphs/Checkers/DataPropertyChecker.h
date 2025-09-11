#ifndef ONTOLOGENIUS_DATAPROPERTYCHECKER_H
#define ONTOLOGENIUS_DATAPROPERTYCHECKER_H

#include "ontologenius/core/ontoGraphs/Checkers/ValidityChecker.h"
#include "ontologenius/core/ontoGraphs/Graphs/DataPropertyGraph.h"

namespace ontologenius {

  class DataPropertyChecker : public ValidityChecker<DataPropertyBranch>
  {
  public:
    explicit DataPropertyChecker(DataPropertyGraph* graph,
                                 OntologyGraphs* all_graphs) : ValidityChecker(graph, all_graphs) {}
    ~DataPropertyChecker() override = default;

    size_t check() override;
    void printStatus() override { ValidityChecker<DataPropertyBranch>::printStatus(std::string("data property"), std::string("data properties"), graph_vect_.size()); }

  private:
    void checkDisjoint();
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_DATAPROPERTYCHECKER_H
