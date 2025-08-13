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
#include "ontologenius/core/ontoGraphs/Graphs/LiteralGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/ObjectPropertyGraph.h"
#include "ontologenius/utils/String.h"

// #define DEBUG

namespace ontologenius {

  AnonymousClassGraph::AnonymousClassGraph(LiteralGraph* literal_graph,
                                           ClassGraph* class_graph,
                                           ObjectPropertyGraph* object_property_graph,
                                           DataPropertyGraph* data_property_graph,
                                           IndividualGraph* individual_graph) : literal_graph_(literal_graph),
                                                                                class_graph_(class_graph),
                                                                                object_property_graph_(object_property_graph),
                                                                                data_property_graph_(data_property_graph),
                                                                                individual_graph_(individual_graph)
  {}

  AnonymousClassGraph::AnonymousClassGraph(const AnonymousClassGraph& other,
                                           LiteralGraph* literal_graph,
                                           ClassGraph* class_graph,
                                           ObjectPropertyGraph* object_property_graph,
                                           DataPropertyGraph* data_property_graph,
                                           IndividualGraph* individual_graph) : Graph(other),
                                                                                literal_graph_(literal_graph),
                                                                                class_graph_(class_graph),
                                                                                object_property_graph_(object_property_graph),
                                                                                data_property_graph_(data_property_graph),
                                                                                individual_graph_(individual_graph)

  {}

  AnonymousClassBranch* AnonymousClassGraph::add(EquivalentClassDescriptor_t& equivalence_descriptor, bool hidden_anonymous)
  {
    const std::lock_guard<std::shared_timed_mutex> lock(Graph<AnonymousClassBranch>::mutex_);
    const std::string ano_name = "anonymous_" + equivalence_descriptor.class_name;
    AnonymousClassBranch* anonymous_branch = new AnonymousClassBranch(ano_name);
    ClassBranch* class_branch = class_graph_->findOrCreateBranch(equivalence_descriptor.class_name, hidden_anonymous);

    anonymous_branch->class_equiv_ = class_branch;
    all_branchs_.push_back(anonymous_branch);
    class_branch->equiv_anonymous_class_ = anonymous_branch;

    for(size_t i = 0; i < equivalence_descriptor.expression_members.size(); i++)
    {
      AnonymousClassTree* tree = createTree(equivalence_descriptor.expression_members[i]);
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

  AnonymousClassTree* AnonymousClassGraph::createTree(ClassExpressionDescriptor_t* class_expression_descriptor)
  {
    std::string rule = class_expression_descriptor->toString();
    AnonymousClassTree* tree = new AnonymousClassTree(rule);
    size_t depth = 0;
    tree->root_node_ = createTreeNodes(class_expression_descriptor, depth, tree);
    tree->depth_ = depth;

    return tree;
  }

  AnonymousClassElement* AnonymousClassGraph::createTreeNodes(ClassExpressionDescriptor_t* class_expression_descriptor, size_t& depth, AnonymousClassTree* related_tree)
  {
    size_t local_depth = depth + 1;

    AnonymousClassElement* node = createNodeContent(class_expression_descriptor, related_tree);

    for(auto* child : class_expression_descriptor->sub_expressions)
    {
      size_t child_depth = depth + 1;
      node->sub_elements_.push_back(createTreeNodes(child, child_depth, related_tree));
      local_depth = std::max(child_depth, local_depth);
    }

    depth = local_depth;

    return node;
  }

  AnonymousClassElement* AnonymousClassGraph::createNodeContent(ClassExpressionDescriptor_t* expression_leaf, AnonymousClassTree* related_tree)
  {
    AnonymousClassElement* ano_element = new AnonymousClassElement();
    ano_element->logical_type_ = LogicalNodeType_e::logical_none;

    switch (expression_leaf->type)
    {
    case ClassExpressionType_e::class_expression_identifier:
      //std::cout << "-->" << expression_leaf->resource_value << " => data=" << expression_leaf->data_usage << " instance=" << expression_leaf->is_instanciated << std::endl;
      if(expression_leaf->data_usage)
      {
        if(expression_leaf->is_instanciated)
          ano_element->card_.card_value_range_ = literal_graph_->findOrCreate(expression_leaf->resource_value);
        else
          ano_element->card_.card_type_range_ = literal_graph_->findOrCreateType(expression_leaf->resource_value);
      }
      else
      {
        if(expression_leaf->is_instanciated)
        {
          ano_element->individual_involved_ = individual_graph_->findOrCreateBranch(expression_leaf->resource_value);
          related_tree->involves_individual = true;
        }
        else
        {
          ano_element->class_involved_ = class_graph_->findOrCreateBranch(expression_leaf->resource_value);
          related_tree->involves_class = true;
        }
      }
      break;
    case ClassExpressionType_e::class_expression_one_of:
      ano_element->oneof = true;
      break;
    case ClassExpressionType_e::class_expression_restriction:
      if(expression_leaf->data_usage == true)
      {
        ano_element->data_property_involved_ = data_property_graph_->findOrCreateBranch(expression_leaf->restriction_property);
        related_tree->involves_data_property = true;
      }
      else
      {
        ano_element->object_property_involved_ = object_property_graph_->findBranch(expression_leaf->restriction_property);
        if(ano_element->object_property_involved_ != nullptr)
          related_tree->involves_object_property = true;
        else
        {
          ano_element->data_property_involved_ = data_property_graph_->findOrCreateBranch(expression_leaf->restriction_property);
          if(ano_element->data_property_involved_ != nullptr)
          {
            related_tree->involves_data_property = true;
            expression_leaf->data_usage = true;
          }
          else
            std::cout << "[Error][AnonymousClassGraph] unknown property " << expression_leaf->restriction_property << std::endl;
        }
      }

      switch (expression_leaf->restriction_type)
      {
      case RestrictionConstraintType_e::restriction_all_values_from:
        ano_element->card_.card_type_ = cardinality_only;
        setCardRange(ano_element, expression_leaf, related_tree);
        break;
      case RestrictionConstraintType_e::restriction_some_values_from:
        ano_element->card_.card_type_ = cardinality_some;
        setCardRange(ano_element, expression_leaf, related_tree);
        break;
      case RestrictionConstraintType_e::restriction_has_value:
        ano_element->card_.card_type_ = cardinality_value;
        if(expression_leaf->resource_value.empty() == false)
        {
          if(expression_leaf->data_usage == true)
            ano_element->card_.card_value_range_ = literal_graph_->findOrCreate(expression_leaf->resource_value);
          else
          {
            ano_element->individual_involved_ = individual_graph_->findOrCreateBranch(expression_leaf->resource_value);
            related_tree->involves_individual = true;
          }
        }
        break;
      case RestrictionConstraintType_e::restriction_max_cardinality:
        ano_element->card_.card_type_ = cardinality_max;
        ano_element->card_.card_number_ = std::stoi(ClassExpressionDescriptor_t::splitData(expression_leaf->cardinality_value).second);
        setCardRange(ano_element, expression_leaf, related_tree); // todo: remove complex flag if no child
        break;
      case RestrictionConstraintType_e::restriction_min_cardinality:
        ano_element->card_.card_type_ = cardinality_min;
        ano_element->card_.card_number_ = std::stoi(ClassExpressionDescriptor_t::splitData(expression_leaf->cardinality_value).second);
        setCardRange(ano_element, expression_leaf, related_tree); // todo: remove complex flag if no child
        break;
      case RestrictionConstraintType_e::restriction_cardinality:
        ano_element->card_.card_type_ = cardinality_exactly;
        ano_element->card_.card_number_ = std::stoi(ClassExpressionDescriptor_t::splitData(expression_leaf->cardinality_value).second);
        setCardRange(ano_element, expression_leaf, related_tree); // todo: remove complex flag if no child
        break;
      
      default:
        break;
      }
      break;
    case ClassExpressionType_e::class_expression_intersection_of:
      ano_element->logical_type_ = LogicalNodeType_e::logical_and;
      break;
    case ClassExpressionType_e::class_expression_union_of:
      ano_element->logical_type_ = LogicalNodeType_e::logical_or;
      break;
    case ClassExpressionType_e::class_expression_complement_of:
      ano_element->logical_type_ = LogicalNodeType_e::logical_not;
      break;
    
    default:
      break;
    }

    return ano_element;
  }

  void AnonymousClassGraph::setCardRange(AnonymousClassElement* ano_element, ClassExpressionDescriptor_t* expression_leaf, AnonymousClassTree* related_tree)
  {
    if(expression_leaf->resource_value.empty() == false)
    {
      if(expression_leaf->data_usage == true)
        ano_element->card_.card_type_range_ = literal_graph_->findOrCreateType(expression_leaf->resource_value);
      else
      {
        ano_element->class_involved_ = class_graph_->findOrCreateBranch(expression_leaf->resource_value);
        related_tree->involves_class = true;
      }
    }
    else
      ano_element->is_complex = true; // todo: check that child has the data usage info
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

    // fisrt we find the equivalent ClassBranch that has been copied by the ClassGraph, even if hidden
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
      if(ano_elem->card_.card_value_range_ != nullptr)
        tmp += " " + ano_elem->card_.card_value_range_->value();
    }
    else
    {
      if(ano_elem->class_involved_ != nullptr)
        tmp += ano_elem->class_involved_->value();
      else if(ano_elem->individual_involved_ != nullptr)
        tmp += ano_elem->individual_involved_->value();
      else if(ano_elem->card_.card_value_range_ != nullptr)
        tmp += ano_elem->card_.card_value_range_->type_->value();
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