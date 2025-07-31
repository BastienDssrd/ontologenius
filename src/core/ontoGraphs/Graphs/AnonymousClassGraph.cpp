#include "ontologenius/core/ontoGraphs/Graphs/AnonymousClassGraph.h"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>

#include "ontologenius/core/ontoGraphs/Branchs/AnonymousClassBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/ClassBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/LiteralNode.h"
#include "ontologenius/core/ontoGraphs/Graphs/ClassGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/DataPropertyGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/Graph.h"
#include "ontologenius/core/ontoGraphs/Graphs/IndividualGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/ObjectPropertyGraph.h"
#include "ontologenius/utils/String.h"

// #define DEBUG

namespace ontologenius {

  AnonymousClassGraph::AnonymousClassGraph(ClassGraph* class_graph,
                                           ObjectPropertyGraph* object_property_graph,
                                           DataPropertyGraph* data_property_graph,
                                           IndividualGraph* individual_graph) : class_graph_(class_graph),
                                                                                object_property_graph_(object_property_graph),
                                                                                data_property_graph_(data_property_graph),
                                                                                individual_graph_(individual_graph)
  {}

  AnonymousClassGraph::AnonymousClassGraph(const AnonymousClassGraph& other,
                                           ClassGraph* class_graph,
                                           ObjectPropertyGraph* object_property_graph,
                                           DataPropertyGraph* data_property_graph,
                                           IndividualGraph* individual_graph) : Graph(other),
                                                                                class_graph_(class_graph),
                                                                                object_property_graph_(object_property_graph),
                                                                                data_property_graph_(data_property_graph),
                                                                                individual_graph_(individual_graph)

  {}

  AnonymousClassBranch* AnonymousClassGraph::add(const std::string& value, AnonymousClassVectors_t& ano)
  {
    const std::lock_guard<std::shared_timed_mutex> lock(Graph<AnonymousClassBranch>::mutex_);

    const std::string ano_name = "anonymous_" + value;
    AnonymousClassBranch* anonymous_branch = new AnonymousClassBranch(ano_name);
    ClassBranch* class_branch = class_graph_->findOrCreateBranch(value);

    anonymous_branch->class_equiv_ = class_branch;
    all_branchs_.push_back(anonymous_branch);
    class_branch->equiv_anonymous_class_ = anonymous_branch;

    for(size_t i = 0; i < ano.equivalence_trees.size(); i++)
    {
      AnonymousClassTree* tree = createTree(ano.equivalence_trees[i]);
      tree->ano_name = ano_name + "_" + std::to_string(i);
      anonymous_branch->ano_trees_.push_back(tree);

#ifdef DEBUG
      printTree(tree->root_node, 3, true);
#endif
    }

    return anonymous_branch;
  }

  void AnonymousClassGraph::deepCopy(const AnonymousClassGraph& other)
  {
    for(size_t i = 0; i < other.all_branchs_.size(); i++)
      cpyBranch(other.all_branchs_[i], all_branchs_[i]);
  }

  AnonymousClassBranch* AnonymousClassGraph::addHiddenRuleElem(const size_t& rule_id, const size_t& elem_id, ExpressionMember_t* ano_expression)
  {
    const std::string current_elem = std::to_string(rule_id) + "_" + std::to_string(elem_id);
    const std::string ano_name = "anonymous_rule_" + current_elem;
    const std::string hidden_class_name = "__rule_" + current_elem;

    AnonymousClassBranch* anonymous_branch = new AnonymousClassBranch(ano_name);

    ClassBranch* hidden_class_branch = class_graph_->findOrCreateBranch(hidden_class_name, true); // this ClassBranch has to be hidden
    hidden_class_branch->equiv_anonymous_class_ = anonymous_branch;
    anonymous_branch->class_equiv_ = hidden_class_branch;
    all_branchs_.push_back(anonymous_branch);

    AnonymousClassTree* tree = createTree(ano_expression);
    tree->ano_name = ano_name;
    anonymous_branch->ano_trees_.push_back(tree);

    return anonymous_branch;
  }

  AnonymousClassBranch* AnonymousClassGraph::addCopyHiddenRuleElem(const size_t& rule_id, const size_t& elem_id, AnonymousClassTree* anonymous_tree)
  {
    const std::string current_elem = std::to_string(rule_id) + "_" + std::to_string(elem_id);
    const std::string ano_name = "anonymous_rule_" + current_elem;
    const std::string hidden_class_name = "__rule_" + current_elem;

    AnonymousClassBranch* anonymous_branch = new AnonymousClassBranch(ano_name); // TODO check if it is not already created

    ClassBranch* hidden_class_branch = class_graph_->findOrCreateBranch(hidden_class_name, true); // this ClassBranch has to be hidden
    hidden_class_branch->equiv_anonymous_class_ = anonymous_branch;
    anonymous_branch->class_equiv_ = hidden_class_branch;
    all_branchs_.push_back(anonymous_branch);

    AnonymousClassTree* tree = copyTree(anonymous_tree);
    anonymous_branch->ano_trees_.push_back(tree);

    return anonymous_branch;
  }

  AnonymousClassTree* AnonymousClassGraph::createTree(ExpressionMember_t* member_node)
  {
    AnonymousClassTree* tree = new AnonymousClassTree(member_node->toString());
    size_t depth = 0;
    tree->root_node_ = createTreeNodes(member_node, depth, tree);
    tree->depth_ = depth;

    return tree;
  }

  AnonymousClassElement* AnonymousClassGraph::createTreeNodes(ExpressionMember_t* member_node, size_t& depth, AnonymousClassTree* related_tree)
  {
    size_t local_depth = depth + 1;

    AnonymousClassElement* node = createNodeContent(member_node, related_tree);

    for(auto* child : member_node->child_members)
    {
      size_t child_depth = depth + 1;
      node->sub_elements_.push_back(createTreeNodes(child, child_depth, related_tree));
      local_depth = std::max(child_depth, local_depth);
    }

    depth = local_depth;

    return node;
  }

  AnonymousClassElement* AnonymousClassGraph::createNodeContent(ExpressionMember_t* exp, AnonymousClassTree* related_tree)
  {
    AnonymousClassElement* ano_element = new AnonymousClassElement();
    ano_element->is_complex = exp->is_complex;
    Restriction_t current_rest = exp->rest;

    // ============= Node type =================
    if(exp->logical_type_ != logical_none)
    {
      ano_element->logical_type_ = exp->logical_type_;
      return ano_element;
    }
    else if(exp->oneof == true)
    {
      ano_element->oneof = true;
      return ano_element;
    }

    // ============  Property ==================
    if(!current_rest.property.empty())
    {
      if(exp->is_data_property)
      {
        ano_element->data_property_involved_ = data_property_graph_->findOrCreateBranch(current_rest.property);
        related_tree->involves_data_property = true;
      }
      else
      {
        ano_element->object_property_involved_ = object_property_graph_->findOrCreateBranch(current_rest.property);
        related_tree->involves_object_property = true;
      }
    }

    // ==============  Cardinality Type & Number =============
    if(current_rest.card.cardinality_type == "some")
      ano_element->card_.card_type_ = cardinality_some;
    else if(current_rest.card.cardinality_type == "only")
      ano_element->card_.card_type_ = cardinality_only;
    else if(current_rest.card.cardinality_type == "value")
      ano_element->card_.card_type_ = cardinality_value;
    else if(current_rest.card.cardinality_type == "exactly")
    {
      ano_element->card_.card_type_ = cardinality_exactly;
      ano_element->card_.card_number_ = std::stoi(current_rest.card.cardinality_number);
    }
    else if(current_rest.card.cardinality_type == "min")
    {
      ano_element->card_.card_type_ = cardinality_min;
      ano_element->card_.card_number_ = std::stoi(current_rest.card.cardinality_number);
    }
    else if(current_rest.card.cardinality_type == "max")
    {
      ano_element->card_.card_type_ = cardinality_max;
      ano_element->card_.card_number_ = std::stoi(current_rest.card.cardinality_number);
    }

    // ===============  Cardinality range  (value, some, only )  ===================
    const std::string card_range = current_rest.card.cardinality_range;
    if(!card_range.empty())
    {
      if(exp->is_data_property) // data property
      {
        if(ano_element->card_.card_type_ == cardinality_value)
          ano_element->card_.card_range_ = data_property_graph_->createLiteral(card_range);
        else
        {
          const std::string type_value = card_range.substr(card_range.find('#') + 1, -1);
          ano_element->card_.card_range_ = data_property_graph_->createLiteral(type_value + "#"); // need to add the "#"
        }
        related_tree->involves_data_property = true;
      }
      else // object property
      {
        if(ano_element->card_.card_type_ == cardinality_value) // indiv
        {
          ano_element->individual_involved_ = individual_graph_->findOrCreateBranch(card_range);
          related_tree->involves_individual = true;
        }
        else
        { // class
          ano_element->class_involved_ = class_graph_->findOrCreateBranch(card_range);
          related_tree->involves_class = true;
        }
      }
    }

    // ===============  Restriction range  (min, max, exactly )===================
    const std::string rest_range = current_rest.restriction_range;
    if(!rest_range.empty())
    {
      if(isIn("http://www.w3.org/", rest_range)) // literal node for complex data restriction (ClassX Eq to data_prop some (not(literal)))
      {
        const std::string type = split(rest_range, "#").back();
        ano_element->card_.card_range_ = data_property_graph_->createLiteral(type + "#"); // need to add the "#"
      }
      else if(exp->mother != nullptr && exp->mother->oneof) // individual node for oneOf (ClassX Eq to oneOf(indiv))
      {
        ano_element->individual_involved_ = individual_graph_->findOrCreateBranch(rest_range);
        related_tree->involves_individual = true;
      }
      else
      {
        ano_element->class_involved_ = class_graph_->findOrCreateBranch(rest_range); // class node for class only restriction (ClassX Eq to ClassY)
        related_tree->involves_class = true;
      }
    }

    return ano_element;
  }

  void AnonymousClassGraph::cpyBranch(AnonymousClassBranch* old_branch, AnonymousClassBranch* new_branch)
  {
    // here the container_ or class_graph_.container refers to the newly created ontology.
    // every branch has already been created with their names, but are empty so we have to fill them with the new values
    // that is why we need to look for old_branch->value(), since they can only be found by their names

    new_branch->nb_updates_ = old_branch->nb_updates_;
    new_branch->setUpdated(old_branch->isUpdated());
    new_branch->flags_ = old_branch->flags_;

    new_branch->dictionary_ = old_branch->dictionary_;
    new_branch->steady_dictionary_ = old_branch->steady_dictionary_;

    // fisrt we find the equivalent ClassBranch that has been copied by the ClassGraph
    auto* equiv_class = class_graph_->container_.find(old_branch->class_equiv_->value());
    // we then link this equivalent class with our new anonymous class in both directions
    new_branch->class_equiv_ = equiv_class;
    equiv_class->equiv_anonymous_class_ = new_branch;

    // copying the old class expressions that are equivalent to the class_equiv (e.g., 'Agent and (hasComponent some Camera)')
    for(auto* old_tree : old_branch->ano_trees_)
      new_branch->ano_trees_.emplace_back(copyTree(old_tree));
  }

  AnonymousClassTree* AnonymousClassGraph::copyTree(AnonymousClassTree* old_tree)
  {
    AnonymousClassTree* tree = new AnonymousClassTree(old_tree->getRule());
    tree->root_node_ = copyTreeNodes(old_tree->root_node_, tree);
    tree->depth_ = old_tree->depth_;
    tree->ano_name = old_tree->ano_name;
    return tree;
  }

  AnonymousClassElement* AnonymousClassGraph::copyTreeNodes(AnonymousClassElement* old_node, AnonymousClassTree* related_tree)
  {
    AnonymousClassElement* node = copyNodeContent(old_node, related_tree);

    for(auto* child : old_node->sub_elements_)
      node->sub_elements_.push_back(copyTreeNodes(child, related_tree));

    return node;
  }

  AnonymousClassElement* AnonymousClassGraph::copyNodeContent(AnonymousClassElement* old_node, AnonymousClassTree* related_tree)
  {
    AnonymousClassElement* new_node = new AnonymousClassElement();
    new_node->is_complex = old_node->is_complex;

    // ============= Node type =================
    if(old_node->logical_type_ != logical_none)
    {
      new_node->logical_type_ = old_node->logical_type_;
      return new_node;
    }
    else if(old_node->oneof == true)
    {
      new_node->oneof = true;
      return new_node;
    }

    // ============= Cardinality =================
    new_node->card_.card_number_ = old_node->card_.card_number_;
    new_node->card_.card_type_ = old_node->card_.card_type_;

    // ============= Expression members =================
    if(old_node->object_property_involved_ != nullptr)
    {
      new_node->object_property_involved_ = object_property_graph_->container_.find(old_node->object_property_involved_->value());
      related_tree->involves_object_property = true;
    }
    else if(old_node->data_property_involved_ != nullptr)
    {
      new_node->data_property_involved_ = data_property_graph_->container_.find(old_node->data_property_involved_->value());
      related_tree->involves_object_property = true;
    }

    if(old_node->individual_involved_ != nullptr)
    {
      new_node->individual_involved_ = individual_graph_->container_.find(old_node->individual_involved_->value());
      related_tree->involves_individual = true;
    }
    else if(old_node->class_involved_ != nullptr)
    {
      new_node->class_involved_ = class_graph_->container_.find(old_node->class_involved_->value());
      related_tree->involves_class = true;
    }

    return new_node;
  }

  void AnonymousClassGraph::printTree(AnonymousClassElement* ano_elem, size_t level, bool root) const
  {
    const std::string space(level * 4, ' ');
    std::string tmp;

    if(root)
      std::cout << space;

    if(ano_elem->logical_type_ == LogicalNodeType_e::logical_and)
      tmp += "and";
    else if(ano_elem->logical_type_ == LogicalNodeType_e::logical_or)
      tmp += "or";
    else if(ano_elem->logical_type_ == LogicalNodeType_e::logical_not)
      tmp += "not";
    else if(ano_elem->oneof)
      tmp += "oneOf";
    else if(ano_elem->object_property_involved_ != nullptr)
    {
      tmp += ano_elem->object_property_involved_->value();
      tmp += " " + cardinalityToString(ano_elem->card_.card_type_);

      if(ano_elem->card_.card_type_ == ontologenius::CardType_e::cardinality_value)
        tmp += " " + ano_elem->individual_involved_->value();
      else
      {
        if(ano_elem->card_.card_number_ != 0)
          tmp += " " + std::to_string(ano_elem->card_.card_number_);
        if(ano_elem->class_involved_ != nullptr)
          tmp += " " + ano_elem->class_involved_->value();
      }
    }
    else if(ano_elem->data_property_involved_ != nullptr)
    {
      tmp += ano_elem->data_property_involved_->value();
      tmp += " " + cardinalityToString(ano_elem->card_.card_type_);
      if(ano_elem->card_.card_number_ != 0)
        tmp += " " + std::to_string(ano_elem->card_.card_number_);
      if(ano_elem->card_.card_range_ != nullptr)
        tmp += " " + ano_elem->card_.card_range_->value();
    }
    else
    {
      if(ano_elem->class_involved_ != nullptr)
        tmp += ano_elem->class_involved_->value();
      else if(ano_elem->individual_involved_ != nullptr)
        tmp += ano_elem->individual_involved_->value();
      else if(ano_elem->card_.card_range_ != nullptr)
        tmp += ano_elem->card_.card_range_->type_;
    }

    std::cout << tmp << std::endl;

    for(auto* sub_elem : ano_elem->sub_elements_)
    {
      for(int i = 0; i < int(level); i++)
        std::cout << "│   ";
      if(sub_elem == ano_elem->sub_elements_.back())
        std::cout << "└── ";
      else
        std::cout << "├── ";
      printTree(sub_elem, level + 1, false);
    }
  }

  std::string AnonymousClassGraph::cardinalityToString(CardType_e value) const
  {
    switch(value)
    {
    case CardType_e::cardinality_some:
      return "some";
    case CardType_e::cardinality_only:
      return "only";
    case CardType_e::cardinality_min:
      return "min";
    case CardType_e::cardinality_max:
      return "max";
    case CardType_e::cardinality_exactly:
      return "exactly";
    case CardType_e::cardinality_value:
      return "value";
    case CardType_e::cardinality_error:
      return "error";
    default:
      return "";
    }
  }

} // namespace ontologenius