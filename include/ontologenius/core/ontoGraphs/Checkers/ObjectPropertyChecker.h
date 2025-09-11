#ifndef ONTOLOGENIUS_OBJECTPROPERTYCHECKER_H
#define ONTOLOGENIUS_OBJECTPROPERTYCHECKER_H

#include "ontologenius/core/ontoGraphs/Checkers/ValidityChecker.h"
#include "ontologenius/core/ontoGraphs/Graphs/ObjectPropertyGraph.h"

namespace ontologenius {

  class ObjectPropertyChecker : public ValidityChecker<ObjectPropertyBranch>
  {
  public:
    explicit ObjectPropertyChecker(ObjectPropertyGraph* graph,
                                   OntologyGraphs* all_graphs) : ValidityChecker(graph, all_graphs) {}
    ~ObjectPropertyChecker() override = default;

    size_t check() override;
    void printStatus() override
    {
      ValidityChecker<ObjectPropertyBranch>::printStatus("object property", "object properties", graph_vect_.size());
    }

  private:
    void checkDisjoint();
    void checkCharacteristics();
    void removeLoops();
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_OBJECTPROPERTYCHECKER_H
