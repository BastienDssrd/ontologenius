#ifndef ONTOLOGENIUS_INDIVIDUALCHECKER_H
#define ONTOLOGENIUS_INDIVIDUALCHECKER_H

#include "ontologenius/core/ontoGraphs/Checkers/ValidityChecker.h"
#include "ontologenius/core/ontoGraphs/Graphs/IndividualGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/ClassGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/DataPropertyGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/ObjectPropertyGraph.h"

namespace ontologenius {

  class IndividualChecker : public ValidityChecker<IndividualBranch>
  {
  public:
    explicit IndividualChecker(IndividualGraph* graph,
                               ClassGraph* class_graph,
                               DataPropertyGraph* data_property_graph,
                               ObjectPropertyGraph* object_property_graph) : ValidityChecker(graph),
                                                                             individual_graph_(graph),
                                                                             class_graph_(class_graph),
                                                                             data_property_graph_(data_property_graph),
                                                                             object_property_graph_(object_property_graph) {}
    ~IndividualChecker() override = default;

    size_t check() override;
    void printStatus() override { ValidityChecker<IndividualBranch>::printStatus("individual", "individuals", graph_vect_.size()); }

  private:
    IndividualGraph* individual_graph_;
    ClassGraph* class_graph_;
    DataPropertyGraph* data_property_graph_;
    ObjectPropertyGraph* object_property_graph_;

    void checkDisjointInheritance(IndividualBranch* indiv, const std::unordered_set<ClassBranch*>& ups);

    void checkDisjoint(IndividualBranch* indiv);
    void checkReflexive(IndividualBranch* indiv);

    void checkObectRelations(IndividualBranch* indiv, const std::unordered_set<ClassBranch*>& up_from);
    void checkDataRelations(IndividualBranch* indiv, const std::unordered_set<ClassBranch*>& up_from);

    void checkAssymetric(IndividualBranch* indiv);

    bool symetricExist(IndividualBranch* indiv_on, ObjectPropertyBranch* sym_prop, IndividualBranch* sym_indiv);
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_INDIVIDUALCHECKER_H
