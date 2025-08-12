#ifndef ONTOLOGENIUS_ANOCLASSGRAPH_H
#define ONTOLOGENIUS_ANOCLASSGRAPH_H

#include <cstddef>
#include <string>
#include <vector>

#include "ontologenius/core/ontoGraphs/Branchs/AnonymousClassBranch.h"
#include "ontologenius/core/ontoGraphs/Graphs/Graph.h"

namespace ontologenius {

  struct ClassExpressionDescriptor_t
  {
    ClassExpressionType_e type;
    std::string resource_value;                     // resource_value should be used as an end node to represent a specific URI (individual, class, datatype)
    std::string restriction_property;
    RestrictionConstraintType_e restriction_type;
    std::string cardinality_value;
    bool data_usage;
    bool is_instanciated;
    std::vector<ClassExpressionDescriptor_t*> sub_expressions;

    ClassExpressionDescriptor_t() : type(class_expression_unknown), restriction_type(restriction_unknown),
                                    data_usage(false), is_instanciated(false) {}
    /*~ClassExpressionDescriptor_t()
    {
      for(auto* sub_expression : sub_expressions)
        delete sub_expression;
    }*/

    static std::pair<std::string, std::string> splitData(const std::string& data)
    {
      size_t pose = data.find('#');
      return {data.substr(0, pose), data.substr(pose + 1)};
    }

    std::string toStringRestriction() const
    {
      std::string res;
      if(restriction_type == RestrictionConstraintType_e::restriction_all_values_from)
        res = " allValuesFrom ";
      else if(restriction_type == RestrictionConstraintType_e::restriction_some_values_from)
        res += " someValuesFrom ";
      else if(restriction_type == RestrictionConstraintType_e::restriction_has_value)
        res = " hasValue ";

      bool cardinality_restriction = res.empty();
      if(cardinality_restriction == false)
      {
        if(resource_value.empty() == false)
          res += resource_value;
        else if(sub_expressions.size() == 1)
          res += "(" + sub_expressions.front()->toString() + ")";
      }
      else
      {
        if(restriction_type == RestrictionConstraintType_e::restriction_min_cardinality)
          res = " minCardinality ";
        else if(restriction_type == RestrictionConstraintType_e::restriction_max_cardinality)
          res += " maxCardinality ";
        else if(restriction_type == RestrictionConstraintType_e::restriction_cardinality)
          res = " cardinality ";

        res += splitData(cardinality_value).second;
        if(resource_value.empty() == false)
          res += " " + resource_value;
        else if(sub_expressions.empty() == false)
          res += sub_expressions.front()->toString();
      }

      return restriction_property + res;
    }

    std::string toString() const
    {
      if(type == ClassExpressionType_e::class_expression_identifier) // Type 1
        return resource_value;
      else if(type == ClassExpressionType_e::class_expression_one_of) // Type 2
      {
        std::string list_string;
        for(auto* expression : sub_expressions)
          list_string += (list_string.empty() ? "" : ", ") + expression->toString();
        return "oneOf (" + list_string + ")";
      }
      if(type == ClassExpressionType_e::class_expression_restriction) // Type 3
        return toStringRestriction();
      else if(type == ClassExpressionType_e::class_expression_complement_of) // Type 6
        return "not(" + sub_expressions.front()->toString() + ")";
      else if(type == ClassExpressionType_e::class_expression_unknown) // Type 6
        return "undefined class expression type";
      else // types 4-5
      {
        std::string list_string;
        for(auto* expression : sub_expressions)
        {
          if(list_string.empty() == false)
          {
            if(type == ClassExpressionType_e::class_expression_intersection_of)
              list_string += " and ";
            else if(type == ClassExpressionType_e::class_expression_union_of)
              list_string += " or ";
          }

          list_string += "(" + expression->toString() + ")";
        }
        return list_string;
      }
    }
  };

  struct EquivalentClassDescriptor_t
  {
    std::string class_name;
    std::vector<ClassExpressionDescriptor_t*> expression_members;

    /*~EquivalentClassDescriptor_t()
    {
      for(auto* expression_member : expression_members)
        delete expression_member;
    }*/
  };

  struct Cardinality_t
  {
    std::string cardinality_type;
    std::string cardinality_number;
    std::string cardinality_range;

    std::string toString() const
    {
      std::string res;
      if(cardinality_type.empty() == false)
        res += " " + cardinality_type + " ";
      if(cardinality_number.empty() == false)
        res += cardinality_number + " ";
      if(cardinality_range.empty() == false)
        res += cardinality_range;
      return res;
    }
  };

  struct Restriction_t
  {
    Cardinality_t card;
    std::string property;
    std::string restriction_range;

    std::string toString() const
    {
      std::string res;

      if(!property.empty())
        res += property;

      res += card.toString();

      if(!restriction_range.empty())
        res += restriction_range;

      return res;
    }
  };

  struct ExpressionMember_t
  {
    LogicalNodeType_e logical_type_;
    bool oneof;
    bool is_complex;
    bool is_data_property;

    // for SWRL rule use only
    Builtin_t builtin_;

    Restriction_t rest; // Restriction (e.g hasComponent some Camera)
    std::vector<ExpressionMember_t*> child_members;
    ExpressionMember_t* mother;
    std::string ano_name;

    ExpressionMember_t() : logical_type_(logical_none), oneof(false),
                           is_complex(false), is_data_property(false),
                           mother(nullptr) {}

    std::string toString() const
    {
      std::string str_equivalence;

      if(builtin_.builtin_type_ != builtin_none)
        str_equivalence = builtin_.builtinToString();
      else if(child_members.empty())
        str_equivalence = rest.toString();
      else if(logical_type_ == logical_not)
        str_equivalence = "not (" + child_members.front()->toString() + ")";
      else
      {
        std::string inner;
        for(auto* child : child_members)
        {
          if(inner.empty() == false)
          {
            if(logical_type_ == logical_and)
              inner += " and ";
            else if(logical_type_ == logical_or)
              inner += " or ";
            else if(oneof == true)
              inner += ", ";
          }
          if(oneof == true)
            inner += child->toString();
          else
            inner += "(" + child->toString() + ")";
        }

        if(oneof == true)
          str_equivalence = "oneOf (" + inner + ")";
        else if(is_complex)
          str_equivalence = rest.toString() + inner;
        else
          str_equivalence = inner;
      }

      return str_equivalence;
    }
  };

  struct AnonymousClassVectors_t // TODO is it really used ?
  {
    std::string class_equiv;
    std::vector<ExpressionMember_t*> equivalence_trees;
    std::vector<std::string> str_equivalences;
  };

  // for friend
  class ObjectPropertyGraph;
  class DataPropertyGraph;
  class IndividualGraph;
  class LiteralGraph;
  class ClassGraph;

  class AnonymousClassChecker;

  class AnonymousClassGraph : public Graph<AnonymousClassBranch>
  {
    friend ObjectPropertyGraph;
    friend DataPropertyGraph;
    friend IndividualGraph;
    friend ClassGraph;

    friend AnonymousClassChecker;

  public:
    AnonymousClassGraph(LiteralGraph* literal_graph, ClassGraph* class_graph, ObjectPropertyGraph* object_property_graph,
                        DataPropertyGraph* data_property_graph, IndividualGraph* individual_graph);
    AnonymousClassGraph(const AnonymousClassGraph& other, LiteralGraph* literal_graph,
                        ClassGraph* class_graph, ObjectPropertyGraph* object_property_graph,
                        DataPropertyGraph* data_property_graph, IndividualGraph* individual_graph);
    ~AnonymousClassGraph() override = default;

    AnonymousClassBranch* add(EquivalentClassDescriptor_t& equivalence_descriptor, bool hidden_anonymous = false);
    AnonymousClassBranch* add(const std::string& value, AnonymousClassVectors_t& ano_class, bool hidden_anonymous = false);
    void deepCopy(const AnonymousClassGraph& other);

  private:
    LiteralGraph* literal_graph_;
    ClassGraph* class_graph_;
    ObjectPropertyGraph* object_property_graph_;
    DataPropertyGraph* data_property_graph_;
    IndividualGraph* individual_graph_;

    AnonymousClassTree* createTree(ClassExpressionDescriptor_t* class_expression_descriptor);
    AnonymousClassElement* createTreeNodes(ClassExpressionDescriptor_t* class_expression_descriptor, size_t& depth, AnonymousClassTree* related_tree);
    AnonymousClassElement* createNodeContent(ClassExpressionDescriptor_t* expression_leaf, AnonymousClassTree* related_tree);

    void setCardRange(AnonymousClassElement* ano_element, ClassExpressionDescriptor_t* expression_leaf, AnonymousClassTree* related_tree); // todo remove, temp

    AnonymousClassTree* createTree(ExpressionMember_t* member_node);
    AnonymousClassElement* createTreeNodes(ExpressionMember_t* member_node, size_t& depth, AnonymousClassTree* related_tree);
    AnonymousClassElement* createNodeContent(ExpressionMember_t* exp_leaf, AnonymousClassTree* related_tree);

    void cpyBranch(AnonymousClassBranch* old_branch, AnonymousClassBranch* new_branch);
    AnonymousClassTree* copyTree(AnonymousClassTree* old_tree);
    AnonymousClassElement* copyTreeNodes(AnonymousClassElement* old_node, AnonymousClassTree* related_tree);
    AnonymousClassElement* copyNodeContent(AnonymousClassElement* old_node, AnonymousClassTree* related_tree);

    void printTree(AnonymousClassElement* ano_elem, size_t level, bool root) const;
    std::string cardinalityToString(CardType_e value) const; 
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_ANOCLASSGRAPH_H