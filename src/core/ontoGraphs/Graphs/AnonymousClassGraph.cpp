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

  ClassExpression* AnonymousClassGraph::createTreeNodes(ClassExpressionDescriptor_t* class_expression_descriptor, size_t& depth, AnonymousClassTree* related_tree, bool mark_tree_content)
  {
    size_t local_depth = depth + 1;

    ClassExpression* node = createNodeContent(class_expression_descriptor, related_tree, mark_tree_content);

    if(node->type_ == ClassExpressionType_e::class_expression_restriction)
      mark_tree_content = false;

    for(auto* child : class_expression_descriptor->sub_expressions)
    {
      size_t child_depth = depth + 1;
      node->sub_elements_.push_back(createTreeNodes(child, child_depth, related_tree, mark_tree_content));
      local_depth = std::max(child_depth, local_depth);
    }

    depth = local_depth;

    return node;
  }

  ClassExpression* AnonymousClassGraph::createNodeContent(ClassExpressionDescriptor_t* expression_leaf, AnonymousClassTree* related_tree, bool mark_tree_content)
  {
    ClassExpression* ano_element = new ClassExpression();
    ano_element->type_ = expression_leaf->type;

    switch (expression_leaf->type)
    {
    case ClassExpressionType_e::class_expression_identifier:
      if(expression_leaf->data_usage)
      {
        if(expression_leaf->is_instanciated)
          ano_element->literal_involved_ = literal_graph_->findOrCreate(expression_leaf->resource_value);
        else
          ano_element->datatype_involved_ = literal_graph_->findOrCreateType(expression_leaf->resource_value);
      }
      else if(expression_leaf->is_instanciated)
      {
        ano_element->individual_involved_ = individual_graph_->findOrCreateBranch(expression_leaf->resource_value);
        if(mark_tree_content)
          related_tree->involves_individual = true;
      }
      else
      {
        ano_element->class_involved_ = class_graph_->findOrCreateBranch(expression_leaf->resource_value);
        if(mark_tree_content)
          related_tree->involves_class = true;
      }
      break;
    case ClassExpressionType_e::class_expression_restriction:
      if(expression_leaf->data_usage == true)
      {
        ano_element->data_property_involved_ = data_property_graph_->findOrCreateBranch(expression_leaf->restriction_property);
        if(mark_tree_content)
          related_tree->involves_data_property = true;
      }
      else
      {
        ano_element->object_property_involved_ = object_property_graph_->findBranch(expression_leaf->restriction_property);
        if(ano_element->object_property_involved_ != nullptr)
        {
          if(mark_tree_content)
            related_tree->involves_object_property = true;
        }
        else
        {
          ano_element->data_property_involved_ = data_property_graph_->findOrCreateBranch(expression_leaf->restriction_property);
          if(ano_element->data_property_involved_ != nullptr)
          {
            if(mark_tree_content)
              related_tree->involves_data_property = true;
            expression_leaf->data_usage = true;
          }
          else
            std::cout << "[Error][AnonymousClassGraph] unknown property " << expression_leaf->restriction_property << std::endl;
        }
      }

      ano_element->restriction_type_ = expression_leaf->restriction_type;
      switch (expression_leaf->restriction_type)
      {
      case RestrictionConstraintType_e::restriction_all_values_from:
        setCardRange(ano_element, expression_leaf);
        related_tree->involves_close_world_assumption = true;
        break;
      case RestrictionConstraintType_e::restriction_some_values_from:
        setCardRange(ano_element, expression_leaf);
        break;
      case RestrictionConstraintType_e::restriction_has_value:
        if(expression_leaf->resource_value.empty() == false)
        {
          auto* sub_expression = new ClassExpression();
          sub_expression->type_ = ClassExpressionType_e::class_expression_identifier;
          if(expression_leaf->data_usage == true)
            sub_expression->literal_involved_ = literal_graph_->findOrCreate(expression_leaf->resource_value);
          else
            sub_expression->individual_involved_ = individual_graph_->findOrCreateBranch(expression_leaf->resource_value);
          ano_element->sub_elements_.emplace_back(sub_expression);
        }
        break;
      case RestrictionConstraintType_e::restriction_max_cardinality:
        ano_element->cardinality_value_ = std::stoi(ClassExpressionDescriptor_t::splitData(expression_leaf->cardinality_value).second);
        setCardRange(ano_element, expression_leaf);
        related_tree->involves_close_world_assumption = true;
        break;
      case RestrictionConstraintType_e::restriction_min_cardinality:
        ano_element->cardinality_value_ = std::stoi(ClassExpressionDescriptor_t::splitData(expression_leaf->cardinality_value).second);
        setCardRange(ano_element, expression_leaf);
        related_tree->involves_close_world_assumption = true;
        break;
      case RestrictionConstraintType_e::restriction_cardinality:
        ano_element->cardinality_value_ = std::stoi(ClassExpressionDescriptor_t::splitData(expression_leaf->cardinality_value).second);
        setCardRange(ano_element, expression_leaf);
        related_tree->involves_close_world_assumption = true;
        break;
      default:
        break;
      }
      break;
    case ClassExpressionType_e::class_expression_complement_of:
      related_tree->involves_close_world_assumption = true;
      break;
    default:
      break;
    }

    return ano_element;
  }

  void AnonymousClassGraph::setCardRange(ClassExpression* ano_element, ClassExpressionDescriptor_t* expression_leaf)
  {
    if(expression_leaf->resource_value.empty() == false)
    {
      auto* sub_expression = new ClassExpression();
      sub_expression->type_ = ClassExpressionType_e::class_expression_identifier;
      if(expression_leaf->data_usage == true)
        sub_expression->datatype_involved_ = literal_graph_->findOrCreateType(expression_leaf->resource_value);
      else
        sub_expression->class_involved_ = class_graph_->findOrCreateBranch(expression_leaf->resource_value);
      ano_element->sub_elements_.emplace_back(sub_expression);
    }
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
    tree->root_node_ = copyTreeNodes(old_tree->root_node_);
    tree->depth_ = old_tree->depth_;
    tree->id = old_tree->id;

    tree->involves_class = old_tree->involves_class;
    tree->involves_data_property = old_tree->involves_data_property;
    tree->involves_individual = old_tree->involves_individual;
    tree->involves_object_property = old_tree->involves_object_property;

    return tree;
  }

  ClassExpression* AnonymousClassGraph::copyTreeNodes(ClassExpression* old_node)
  {
    ClassExpression* node = copyNodeContent(old_node);

    for(auto* child : old_node->sub_elements_)
      node->sub_elements_.push_back(copyTreeNodes(child));

    return node;
  }

  ClassExpression* AnonymousClassGraph::copyNodeContent(ClassExpression* old_node)
  {
    ClassExpression* new_node = new ClassExpression();

    new_node->type_ = old_node->type_;

    if(new_node->literal_involved_ != nullptr)
      new_node->literal_involved_ = literal_graph_->find(old_node->literal_involved_->value());

    if(new_node->datatype_involved_ != nullptr)
      new_node->datatype_involved_ = literal_graph_->findOrCreateType(old_node->datatype_involved_->value());

    if(old_node->object_property_involved_ != nullptr)
      new_node->object_property_involved_ = object_property_graph_->container_.find(old_node->object_property_involved_->value());

    if(old_node->data_property_involved_ != nullptr)
      new_node->data_property_involved_ = data_property_graph_->container_.find(old_node->data_property_involved_->value());

    if(old_node->individual_involved_ != nullptr)
      new_node->individual_involved_ = individual_graph_->container_.find(old_node->individual_involved_->value());
    
    if(old_node->class_involved_ != nullptr)
      new_node->class_involved_ = class_graph_->container_.find(old_node->class_involved_->value());

    return new_node;
  }

  void AnonymousClassGraph::printTree(ClassExpression* expression, size_t level)
  {
    std::string spaces("  ", level);
    std::string value;
    if(expression->data_property_involved_ != nullptr)
      value += "data_prop(" + expression->data_property_involved_->value() + ") ";
    if(expression->object_property_involved_ != nullptr)
      value += "obj_prop(" + expression->object_property_involved_->value() + ") ";

    if(expression->restriction_type_ != RestrictionConstraintType_e::restriction_unknown)
      value += "rest(" + std::to_string(expression->restriction_type_) + ") ";

    if(expression->cardinality_value_ != 0)
      value += "card(" + std::to_string(expression->cardinality_value_) + ") ";

    if(expression->class_involved_ != nullptr)
      value += "class(" + expression->class_involved_->value() + ") ";
    if(expression->individual_involved_ != nullptr)
      value += "indiv(" + expression->individual_involved_->value() + ") ";
    if(expression->literal_involved_ != nullptr)
      value += "literal(" + expression->literal_involved_->value() + ") ";
    if(expression->datatype_involved_ != nullptr)
      value += "datatype(" + expression->datatype_involved_->value() + ") ";

    std::cout << spaces << "-" << level << " " << value << std::endl;
    for(auto& sub : expression->sub_elements_)
      printTree(sub, level + 1);
  }

} // namespace ontologenius