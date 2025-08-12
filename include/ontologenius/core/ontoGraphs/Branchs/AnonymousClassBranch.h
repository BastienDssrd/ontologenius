#ifndef ONTOLOGENIUS_ANONYMOUSCLASSBRANCH_H
#define ONTOLOGENIUS_ANONYMOUSCLASSBRANCH_H

#include <cstddef>
#include <string>
#include <vector>

#include "ontologenius/core/ontoGraphs/Branchs/ClassBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/LiteralNode.h"
#include "ontologenius/core/ontoGraphs/Branchs/ValuedNode.h"

namespace ontologenius {

  enum RestrictionConstraintType_e
  {
    restriction_all_values_from,
    restriction_some_values_from,
    restriction_has_value,
    restriction_max_cardinality,
    restriction_min_cardinality,
    restriction_cardinality,
    restriction_unknown
  };

  enum ClassExpressionType_e
  {
    class_expression_identifier,      // type 1
    class_expression_one_of,          // type 2
    class_expression_restriction,     // type 3
    class_expression_intersection_of, // type 4
    class_expression_union_of,        // type 5
    class_expression_complement_of,   // type 6
    class_expression_unknown
  };

  enum CardType_e
  {
    cardinality_none,
    cardinality_some,
    cardinality_only,
    cardinality_min,
    cardinality_max,
    cardinality_exactly,
    cardinality_value,
    cardinality_error
  };

  enum LogicalNodeType_e
  {
    logical_and,
    logical_or,
    logical_not,
    logical_none
  };

  enum BuiltinType_e
  {
    builtin_none,
    greater_than,
    greater_than_or_equal,
    less_than,
    less_than_or_equal,
    equal,
    not_equal
  };

  struct Builtin_t // todo: to move
  {
    BuiltinType_e builtin_type_;
    std::string builtin_str_;

    Builtin_t() : builtin_type_(builtin_none) {}
    Builtin_t(const BuiltinType_e& builtin_type, const std::string& builtin_str) : builtin_type_(builtin_type),
                                                                                   builtin_str_(builtin_str)
    {}

    std::string builtinToString() const
    {
      std::string builtin_name;
      switch(this->builtin_type_)
      {
      case greater_than:
        builtin_name = "greaterThan";
        break;
      case greater_than_or_equal:
        builtin_name = "greaterThanOrEqual";
        break;
      case less_than:
        builtin_name = "lessThan";
        break;
      case less_than_or_equal:
        builtin_name = "lessThanOrEqual";
        break;
      case equal:
        builtin_name = "equal";
        break;
      case not_equal:
        builtin_name = "notEqual";
        break;
      default:
        break;
      }

      return builtin_name;
    }
  };

  struct CardinalityElement_t
  {
    CardType_e card_type_ = cardinality_none;
    size_t card_number_ = 0;
    LiteralNode* card_value_range_ = nullptr;
    LiteralType* card_type_range_ = nullptr;

    std::string toString() const
    {
      switch(card_type_)
      {
      case cardinality_some:
        return "some";
      case cardinality_only:
        return "only";
      case cardinality_value:
        return "value";
      case cardinality_min:
        return "min " + std::to_string(card_number_);
      case cardinality_max:
        return "max " + std::to_string(card_number_);
      case cardinality_exactly:
        return "exactly " + std::to_string(card_number_);
      default:
        return "";
      }
    }
  };

  class AnonymousClassTree;

  class AnonymousClassElement
  {
  public:
    AnonymousClassElement() : logical_type_(logical_none), oneof(false), is_complex(false),
                              class_involved_(nullptr), object_property_involved_(nullptr), data_property_involved_(nullptr), individual_involved_(nullptr)
    {}

    LogicalNodeType_e logical_type_;
    bool oneof; // true = OneOf element
    bool is_complex;

    // pointers to the concepts used in the equivalence relation
    ClassBranch* class_involved_;
    ObjectPropertyBranch* object_property_involved_;
    DataPropertyBranch* data_property_involved_;
    IndividualBranch* individual_involved_;

    CardinalityElement_t card_;

    std::vector<AnonymousClassElement*> sub_elements_;
  };

  class AnonymousClassTree : public InferenceRuleNode
  {
  public:
    AnonymousClassTree(const std::string& rule) : InferenceRuleNode(rule),
                                                  involves_class(false), involves_object_property(false), involves_data_property(false), involves_individual(false),
                                                  root_node_(nullptr)
    {}

    bool involves_class;
    bool involves_object_property;
    bool involves_data_property;
    bool involves_individual;

    AnonymousClassElement* root_node_;
    size_t depth_;

    std::string ano_name;

    std::string involvesToString() const
    {
      std::string involves_res;
      involves_res = " c : " + std::to_string(int(involves_class)) + " o : " + std::to_string(int(involves_object_property)) +
                     " d : " + std::to_string(int(involves_data_property)) + " i : " + std::to_string(int(involves_individual));
      return involves_res;
    }
  };

  class AnonymousClassBranch : public ValuedNode
  {
  public:
    explicit AnonymousClassBranch(const std::string& value, bool hidden = false) : ValuedNode(value, hidden), class_equiv_(nullptr) {}
    ~AnonymousClassBranch()
    {
      for(auto* tree : ano_trees_)
        delete tree;
    }

    ClassBranch* class_equiv_;
    std::vector<AnonymousClassTree*> ano_trees_;
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_ANONYMOUSCLASSBRANCH_H