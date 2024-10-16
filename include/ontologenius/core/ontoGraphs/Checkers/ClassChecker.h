#ifndef ONTOLOGENIUS_CLASSCHECKER_H
#define ONTOLOGENIUS_CLASSCHECKER_H

#include "ontologenius/core/ontoGraphs/Graphs/ClassGraph.h"
#include "ontologenius/core/ontoGraphs/Checkers/ValidityChecker.h"

namespace ontologenius {

class ClassChecker : public ValidityChecker<ClassBranch_t>
{
public:
  explicit ClassChecker(ClassGraph* graph) : ValidityChecker(graph) {class_graph_ = graph;}
  ~ClassChecker() {}

  size_t check() override;
  void printStatus(){ValidityChecker<ClassBranch_t>::printStatus("class", "classes", graph_vect_.size());}

private:
  void checkDisjoint();

  void checkObectPropertyDomain();
  void checkObectPropertyRange();

  void checkDataPropertyDomain();
  void checkDataPropertyRange();

  ClassGraph* class_graph_;
};

} // namespace ontologenius

#endif // ONTOLOGENIUS_CLASSCHECKER_H
