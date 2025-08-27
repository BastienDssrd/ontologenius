#ifndef ONTOLOGENIUS_ANONYMOUSCLASSCHECKER_H
#define ONTOLOGENIUS_ANONYMOUSCLASSCHECKER_H

#include <unordered_set>
#include <vector>

#include "ontologenius/core/ontoGraphs/Branchs/AnonymousClassBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/ClassBranch.h"
#include "ontologenius/core/ontoGraphs/Checkers/ValidityChecker.h"
#include "ontologenius/core/ontoGraphs/Graphs/AnonymousClassGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/OntologyGraphs.h"

namespace ontologenius {

  class AnonymousClassChecker : public ValidityChecker<AnonymousClassBranch>
  {
  public:
    explicit AnonymousClassChecker(AnonymousClassGraph* graph,
                                   OntologyGraphs* all_graphs) : ValidityChecker(graph, all_graphs) {}
    ~AnonymousClassChecker() override = default;

    size_t check() override;
    void checkDisjoint();

    void printStatus() override
    {
      ValidityChecker<AnonymousClassBranch>::printStatus("anonymous_class", "anonymous_classes", graph_vect_.size());
    }

  private:
    std::string current_ano_;

    std::vector<std::string> resolveTreeDisjoint(ClassExpression* ano_elem, std::unordered_set<ClassBranch*>& disjoints, std::unordered_set<ClassBranch*>& uppers, bool complement_mode);
    std::vector<std::string> resolveTreeDisjoint(ClassExpression* ano_elem, std::unordered_set<LiteralType*>& data_ranges);

    std::vector<std::string> checkIdentifier(ClassExpression* ano_elem, std::unordered_set<ClassBranch*>& disjoints, std::unordered_set<ClassBranch*>& uppers, bool complement_mode);
    std::vector<std::string> checkIdentifier(ClassExpression* ano_elem, std::unordered_set<LiteralType*>& data_ranges);
    std::vector<std::string> checkOneOf(ClassExpression* ano_elem, std::unordered_set<ClassBranch*>& disjoints, std::unordered_set<ClassBranch*>& uppers, bool complement_mode);
    std::vector<std::string> checkOneOf(ClassExpression* ano_elem, std::unordered_set<LiteralType*>& data_ranges);
    std::vector<std::string> checkRestriction(ClassExpression* ano_elem, std::unordered_set<ClassBranch*>& disjoints, std::unordered_set<ClassBranch*>& uppers, bool complement_mode);

    std::vector<std::string> resolveTreeDataTypes(ClassExpression* ano_elem, std::unordered_set<LiteralType*>& data_ranges); // Err
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_ANONYMOUSCLASSCHECKER_H