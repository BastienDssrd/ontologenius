#ifndef ONTOLOGENIUS_CLASSCHECKER_H
#define ONTOLOGENIUS_CLASSCHECKER_H

#include "ontologenius/core/ontoGraphs/Checkers/ValidityChecker.h"
#include "ontologenius/core/ontoGraphs/Graphs/ClassGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/OntologyGraphs.h"

namespace ontologenius {

  class ClassChecker : public ValidityChecker<ClassBranch>
  {
  public:
    explicit ClassChecker(ClassGraph* graph,
                          OntologyGraphs* all_graphs) : ValidityChecker(graph, all_graphs) {}
    ~ClassChecker() override = default;

    size_t check() override;
    void printStatus() override { ValidityChecker<ClassBranch>::printStatus("class", "classes", graph_vect_.size()); }

  private:
    void checkDisjoint(ClassBranch* branch, const std::unordered_set<ClassBranch*>& up);

    void checkObjectPropertyDomain(ClassBranch* branch, const std::unordered_set<ClassBranch*>& up);
    void checkObjectPropertyRange(ClassBranch* branch);

    void checkDataPropertyDomain(ClassBranch* branch, const std::unordered_set<ClassBranch*>& up);
    void checkDataPropertyRange(ClassBranch* branch);
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_CLASSCHECKER_H
