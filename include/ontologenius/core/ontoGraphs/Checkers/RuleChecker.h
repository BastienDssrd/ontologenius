#ifndef ONTOLOGENIUS_RULECHECKER_H
#define ONTOLOGENIUS_RULECHECKER_H

#include "ontologenius/core/ontoGraphs/Checkers/ValidityChecker.h"
#include "ontologenius/core/ontoGraphs/Graphs/ClassGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/DataPropertyGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/ObjectPropertyGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/RuleGraph.h"

namespace ontologenius {

  class RuleChecker : public ValidityChecker<RuleBranch>
  {
  public:
    explicit RuleChecker(RuleGraph* graph) : ValidityChecker(graph), rule_graph_(graph) {}
    ~RuleChecker() override = default;

    size_t check() override;
    void checkDisjoint();

    void printStatus() override { ValidityChecker<RuleBranch>::printStatus("rule", "rules", graph_vect_.size()); }

  private:
    RuleGraph* rule_graph_;
    std::string current_rule_;

    void checkRuleDisjoint(RuleBranch* branch);
    void checkAtom(const RuleTriplet_t& atom,
                   std::unordered_map<std::string, std::pair<std::unordered_set<ClassBranch*>, std::unordered_set<LiteralType*>>>& variables_types,
                   std::unordered_map<std::string, Variable_t>& all_arguments);

    void setArgument(const RuleArgument_t& arg, std::unordered_map<std::string, Variable_t>& all_arguments, bool individual_usage);

    void checkClassAtom(const RuleTriplet_t& atom, std::unordered_map<std::string, std::pair<std::unordered_set<ClassBranch*>, std::unordered_set<LiteralType*>>>& variables_types);
    void checkObjectPropertyAtom(const RuleTriplet_t& atom, std::unordered_map<std::string, std::pair<std::unordered_set<ClassBranch*>, std::unordered_set<LiteralType*>>>& variables_types);
    void checkDataPropertyAtom(const RuleTriplet_t& atom, std::unordered_map<std::string, std::pair<std::unordered_set<ClassBranch*>, std::unordered_set<LiteralType*>>>& variables_types);
    void checkBuiltinAtom(const RuleTriplet_t& atom, std::unordered_map<std::string, std::pair<std::unordered_set<ClassBranch*>, std::unordered_set<LiteralType*>>>& variables_types);

    std::string check(const std::vector<ClassElement>& classes, std::unordered_set<ClassBranch*>& variables_classes);
    std::string check(const std::vector<LiteralType*>& types, std::unordered_set<LiteralType*>& variables_types);
    void raiseError(const RuleArgument_t& var, const std::string& msg);
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_RULECHECKER_H