#ifndef ONTOLOGENIUS_CLASSCHECKER_H
#define ONTOLOGENIUS_CLASSCHECKER_H

#include "ontologenius/core/ontoGraphs/Checkers/ValidityChecker.h"
#include "ontologenius/core/ontoGraphs/Graphs/ClassGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/ClassGraph.h"

namespace ontologenius {

  class ClassChecker : public ValidityChecker<ClassBranch>
  {
  public:
    explicit ClassChecker(ClassGraph* graph,
                          DataPropertyGraph* data_property_graph,
                          ObjectPropertyGraph* object_property_graph) : ValidityChecker(graph), class_graph_(graph),
                                                                        data_property_graph_(data_property_graph),
                                                                        object_property_graph_(object_property_graph) {}
    ~ClassChecker() override = default;

    size_t check() override;
    void printStatus() override { ValidityChecker<ClassBranch>::printStatus("class", "classes", graph_vect_.size()); }

  private:
    void checkDisjoint(ClassBranch* branch, const std::unordered_set<ClassBranch*>& up);

    void checkObjectPropertyDomain(ClassBranch* branch, const std::unordered_set<ClassBranch*>& up);
    void checkObjectPropertyRange(ClassBranch* branch);

    void checkDataPropertyDomain(ClassBranch* branch, const std::unordered_set<ClassBranch*>& up);
    void checkDataPropertyRange(ClassBranch* branch);

    ClassGraph* class_graph_;
    DataPropertyGraph* data_property_graph_;
    ObjectPropertyGraph* object_property_graph_;
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_CLASSCHECKER_H
