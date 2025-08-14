#ifndef ONTOLOGENIUS_RULEOWLWRITER_H
#define ONTOLOGENIUS_RULEOWLWRITER_H

#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

#include "ontologenius/core/ontoGraphs/Branchs/ClassBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/RuleBranch.h"
#include "ontologenius/core/ontoGraphs/Graphs/RuleGraph.h"
#include "ontologenius/core/ontologyIO/Owl/writers/AnonymousClassOwlWriter.h"

namespace ontologenius {

  class RuleGraph;
  class RuleBranch;

  class RuleOwlWriter : private AnonymousClassOwlWriter
  {
  public:
    RuleOwlWriter(RuleGraph* rule_graph, FILE* file, const std::string& ns);
    ~RuleOwlWriter() = default;

    void write();

  private:
    RuleGraph* rule_graph_;

    void writeRule(RuleBranch* branch);
    void writeVariable(const std::string& rule_variable);

    void writeAtom(const std::vector<RuleTriplet_t>& atom_list, const RuleTriplet_t& current_atom, size_t level, size_t index = 0);
    void writeClassAtom(const RuleTriplet_t& class_atom, size_t level);
    void writeObjectAtom(const RuleTriplet_t& object_atom, size_t level);
    void writeDataAtom(const RuleTriplet_t& data_atom, size_t level);
    void writeBuiltinAtom(const RuleTriplet_t& builtin_atom, size_t level);
    void writeRuleBuiltinArguments(const std::vector<RuleArgument_t>& arguments, size_t index, size_t level);
    void writeRuleArguments(const std::vector<RuleArgument_t>& arguments, size_t level);
    std::string getArgumentString(const RuleArgument_t& arg, const std::string& key);
  };
} // namespace ontologenius

#endif // ONTOLOGENIUS_RULEOWLWRITER_H