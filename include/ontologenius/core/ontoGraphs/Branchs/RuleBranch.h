#ifndef ONTOLOGENIUS_RULEBRANCH_H
#define ONTOLOGENIUS_RULEBRANCH_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "ontologenius/core/ontoGraphs/Branchs/AnonymousClassBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/ClassBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/IndividualBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/LiteralNode.h"
#include "ontologenius/core/ontoGraphs/Branchs/ObjectPropertyBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/ValuedNode.h"

namespace ontologenius {

  struct RuleArgument_t
  {
    // empty constructor
    RuleArgument_t() : variable_id(-1),
                       is_variable(false),
                       indiv_value(nullptr),
                       datatype_value(nullptr)
    {}
    // variable constructor
    RuleArgument_t(const std::string& name_value) : name(name_value),
                                                    variable_id(-1),
                                                    is_variable(true),
                                                    indiv_value(nullptr),
                                                    datatype_value(nullptr)
    {}
    // instantiated individual variable constructor
    RuleArgument_t(IndividualBranch* indiv) : name(indiv->value()),
                                              variable_id(-1),
                                              is_variable(false),
                                              indiv_value(indiv),
                                              datatype_value(nullptr)
    {}

    // instantiated literal variable constructor
    RuleArgument_t(LiteralNode* literal) : name(literal->toString()),
                                           variable_id(-1),
                                           is_variable(false),
                                           indiv_value(nullptr),
                                           datatype_value(literal)
    {}

    std::string name;
    int64_t variable_id;
    bool is_variable;
    IndividualBranch* indiv_value;
    LiteralNode* datatype_value;

    std::string toString() const
    {
      return (is_variable ? "?" : "" ) + name;
    }
  };

  enum AtomType_e
  {
    default_atom,
    class_atom,
    object_atom,
    data_atom,
    builtin_atom
  };

  struct RuleTriplet_t
  {
    // empty constructor
    RuleTriplet_t() : atom_type_(default_atom), class_predicate(nullptr), anonymous_element(nullptr), object_predicate(nullptr), data_predicate(nullptr)
    {}

    AtomType_e atom_type_;

    ClassBranch* class_predicate;            // set only if class atom
    AnonymousClassBranch* anonymous_element; // used to store the anonymous class if the class expression is complex
    ObjectPropertyBranch* object_predicate;  // set only if object atom
    DataPropertyBranch* data_predicate;      // set only if data atom
    Builtin_t builtin;                       // used only for builtin atoms
    std::vector<RuleArgument_t> arguments;   // can be variables or not (?c is, pr2 isn't)

    std::string toString() const
    {
      std::string res;

      switch(atom_type_)
      {
      case class_atom:
        res = class_predicate->value() + "(" + arguments.at(0).toString() + ")";
        break;
      case object_atom:
        res = object_predicate->value() + "(" + arguments.at(0).toString() + ", " + arguments.at(1).toString() + ")";
        break;
      case data_atom:
        res = data_predicate->value() + "(" + arguments.at(0).toString() + ", " + arguments.at(1).toString() + ")";
        break;
      case builtin_atom:
        /* code */
        break;
      default:
        break;
      }
      return res;
    }
  };

  struct Variable_t
  {
    Variable_t() : is_individual(false), is_datavalue(false), is_builtin_value(false), var_index(-1) {}

    std::string var_name;
    bool is_individual;  // for indiv
    bool is_datavalue;     // for literal
    bool is_builtin_value; // for builtin data // todo: why never used ?
    int64_t var_index;

    std::string toString() const { return var_name; }
  };

  class RuleBranch : public ValuedNode,
                     public InferenceRuleNode
  {
  public:
    explicit RuleBranch(const std::string& value,
                        const std::string& rule,
                        bool hidden = false) : ValuedNode(value, hidden),
                                               InferenceRuleNode(rule),
                                               involves_class(false),
                                               involves_object_property(false),
                                               involves_data_property(false)
    {}

    bool involves_class;
    bool involves_object_property;
    bool involves_data_property;

    std::vector<RuleTriplet_t> rule_body_;
    std::vector<RuleTriplet_t> rule_head_;

    // mapping between variables and creation of the RuleArgument_t elements
    std::unordered_map<std::string, int64_t> variables_; // mapping between var names and index

    std::vector<size_t> atom_initial_order_;
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_RULEBRANCH_H