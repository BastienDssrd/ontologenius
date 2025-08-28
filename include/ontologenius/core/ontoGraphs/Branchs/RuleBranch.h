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
    RuleArgument_t(LiteralNode* literal) : name(literal->value()),
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

  enum RuleAtomType_e
  {
    rule_atom_class,
    rule_atom_object,
    rule_atom_data,
    rule_atom_builtin,
    rule_atom_unknown
  };

  enum RuleBuiltinType_e
  {
    builtin_greater_than,
    builtin_greater_than_or_equal,
    builtin_less_than,
    builtin_less_than_or_equal,
    builtin_equal,
    builtin_not_equal,
    builtin_unknon
  };

  struct RuleTriplet_t
  {
    RuleTriplet_t() : atom_type_(rule_atom_unknown), class_predicate(nullptr), anonymous_element(nullptr), object_predicate(nullptr), data_predicate(nullptr)
    {}

    RuleAtomType_e atom_type_;

    ClassBranch* class_predicate;            // set only if class atom
    AnonymousClassBranch* anonymous_element; // used to store the anonymous class if the class expression is complex
    ObjectPropertyBranch* object_predicate;  // set only if object atom
    DataPropertyBranch* data_predicate;      // set only if data atom
    RuleBuiltinType_e builtin;               // used only for builtin atoms
    std::vector<RuleArgument_t> arguments;   // can be variables or not (?c is, pr2 isn't)

    std::string builtinToString() const
    {
      switch(builtin)
      {
      case builtin_greater_than:          return "greaterThan";
      case builtin_greater_than_or_equal: return "greaterThanOrEqual";
      case builtin_less_than:             return "lessThan";
      case builtin_less_than_or_equal:    return "lessThanOrEqual";
      case builtin_equal:                 return "equal";
      case builtin_not_equal:             return "notEqual";
      default:                            return "unsupported builtin";
      }
    }

    std::string toString() const
    {
      std::string res;

      switch(atom_type_)
      {
      case rule_atom_class:
        res = class_predicate->value() + "(" + arguments.at(0).toString() + ")";
        break;
      case rule_atom_object:
        res = object_predicate->value() + "(" + arguments.at(0).toString() + ", " + arguments.at(1).toString() + ")";
        break;
      case rule_atom_data:
        res = data_predicate->value() + "(" + arguments.at(0).toString() + ", " + arguments.at(1).toString() + ")";
        break;
      case rule_atom_builtin:
        res = builtinToString() + "(" + arguments.at(0).toString() + ", " + arguments.at(1).toString() + ")"; // should be made more generic
        break;
      default:
        break;
      }
      return res;
    }
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