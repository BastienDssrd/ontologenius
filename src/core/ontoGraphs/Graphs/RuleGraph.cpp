#include "ontologenius/core/ontoGraphs/Graphs/RuleGraph.h"

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <string>
#include <utility>
#include <vector>

#include "ontologenius/core/ontoGraphs/Branchs/AnonymousClassBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/ClassBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/LiteralNode.h"
#include "ontologenius/core/ontoGraphs/Branchs/RuleBranch.h"
#include "ontologenius/core/ontoGraphs/Graphs/AnonymousClassGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/ClassGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/DataPropertyGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/Graph.h"
#include "ontologenius/core/ontoGraphs/Graphs/IndividualGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/LiteralGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/ObjectPropertyGraph.h"
#include "ontologenius/utils/String.h"

// #define DEBUG

namespace ontologenius {

  RuleGraph::RuleGraph(LiteralGraph* literal_graph, ClassGraph* class_graph,
                       ObjectPropertyGraph* object_property_graph, DataPropertyGraph* data_property_graph,
                       IndividualGraph* individual_graph, AnonymousClassGraph* anonymous_graph) : literal_graph_(literal_graph),
                                                                                                  class_graph_(class_graph),
                                                                                                  object_property_graph_(object_property_graph),
                                                                                                  data_property_graph_(data_property_graph),
                                                                                                  individual_graph_(individual_graph),
                                                                                                  anonymous_graph_(anonymous_graph)
  {}

  RuleGraph::RuleGraph(const RuleGraph& other, LiteralGraph* literal_graph,
                       ClassGraph* class_graph,
                       ObjectPropertyGraph* object_property_graph,
                       DataPropertyGraph* data_property_graph,
                       IndividualGraph* individual_graph,
                       AnonymousClassGraph* anonymous_graph) : // Graph copy constructor is not called as RuleBranch need more advanced copy
                                                               literal_graph_(literal_graph),
                                                               class_graph_(class_graph),
                                                               object_property_graph_(object_property_graph),
                                                               data_property_graph_(data_property_graph),
                                                               individual_graph_(individual_graph),
                                                               anonymous_graph_(anonymous_graph)
  {
    all_branchs_.reserve(other.all_branchs_.size());
    for(auto* branch : other.all_branchs_)
    {
      auto* rule_branch = new RuleBranch(branch->value(), branch->getRule());
      all_branchs_.push_back(rule_branch);
    }
  }

  RuleBranch* RuleGraph::add(const RuleDescriptor_t& rule)
  {
    const std::lock_guard<std::shared_timed_mutex> lock(Graph<RuleBranch>::mutex_);

    const std::string rule_name = "rule_" + std::to_string(all_branchs_.size());
    RuleBranch* rule_branch = new RuleBranch(rule_name, rule.rule_str);
    all_branchs_.push_back(rule_branch);

    size_t elem_id = 0;

    for(const auto& atom : rule.antecedents)
    {
      rule_branch->atom_initial_order_.push_back(rule_branch->rule_body_.size());
      rule_branch->rule_body_.push_back(createRuleAtomTriplet(rule_branch, atom, elem_id, true));
      elem_id++;
    }

    for(const auto& atom : rule.consequents)
    {
      rule_branch->rule_head_.push_back(createRuleAtomTriplet(rule_branch, atom, elem_id, false));
      elem_id++;
    }

    return rule_branch;
  }

  RuleTriplet_t RuleGraph::createRuleAtomTriplet(RuleBranch* rule_branch, const std::pair<RuleAtomDescriptor_t, std::vector<RuleVariableDescriptor_t>>& atom, const size_t& elem_id, const bool& is_head)
  {
    RuleTriplet_t rule_triplet;

    const auto& rule_atom = atom.first;

    for(const auto& variable : atom.second)
      rule_triplet.arguments.push_back(getRuleArgument(rule_branch, variable));

    rule_triplet.atom_type_ = rule_atom.type;
    switch (rule_atom.type)
    {
    case RuleAtomType_e::rule_atom_builtin:
      rule_triplet.builtin = rule_atom.builtin;
      break;
    case RuleAtomType_e::rule_atom_data:
      if(is_head)
        rule_branch->involves_data_property = true;

      rule_triplet.data_predicate = data_property_graph_->findOrCreateBranch(rule_atom.resource_value);
      break;
    case RuleAtomType_e::rule_atom_object:
      if(is_head)
        rule_branch->involves_object_property = true;

      rule_triplet.object_predicate = object_property_graph_->findOrCreateBranch(rule_atom.resource_value);
      break;
    case RuleAtomType_e::rule_atom_class:
      if(is_head)
        rule_branch->involves_class = true;
      if(rule_atom.class_expression != nullptr)
      {
        EquivalentClassDescriptor_t class_descriptor;
        class_descriptor.class_name = rule_branch->value() + "_" + std::to_string(elem_id);
        class_descriptor.expression_members.push_back(rule_atom.class_expression);
        rule_triplet.anonymous_element = anonymous_graph_->add(class_descriptor, true);     // returns the newly created ano branch
        rule_triplet.class_predicate = rule_triplet.anonymous_element->class_equiv_;
      }
      else
        rule_triplet.class_predicate = class_graph_->findOrCreateBranch(rule_atom.resource_value); 
      break;
    default:
      break;
    }

    return rule_triplet;
  }

  RuleArgument_t RuleGraph::getRuleArgument(RuleBranch* rule_branch, const RuleVariableDescriptor_t& variable)
  {
    if(variable.is_instanciated == false)
    {
      RuleArgument_t resource(variable.name);
      setVariableIndex(rule_branch, resource);
      return resource;
    }
    else if(variable.datatype)
    {
      LiteralNode* involved_datatype = literal_graph_->findOrCreate(variable.name);
      RuleArgument_t resource(involved_datatype);
      setVariableIndex(rule_branch, resource); // Todo, should be removed, but removing it raise an error in ReasonerRule
      return resource;
    }
    else
    {
      IndividualBranch* involved_indiv = individual_graph_->findOrCreateBranch(variable.name);
      RuleArgument_t resource(involved_indiv);
      setVariableIndex(rule_branch, resource); // Todo, should be removed, but removing it raise an error in ReasonerRule
      return resource;
    }
  }

  void RuleGraph::setVariableIndex(RuleBranch* rule_branch, RuleArgument_t& resource)
  {
    auto var_it = rule_branch->variables_.find(resource.name);
    if(var_it == rule_branch->variables_.end()) // if the variable doesn't already exist in the variables_ vector of the rule
    {
      int64_t index = std::int64_t(rule_branch->variables_.size());
      rule_branch->variables_[resource.name] = index;
      resource.variable_id = index;
      variable_names_.insert(resource.name); // to rewrite the variables
    }
    else
      resource.variable_id = var_it->second; // if it already does, we just link it with its
  }

  void RuleGraph::deepCopy(const RuleGraph& other)
  {
    for(size_t i = 0; i < other.all_branchs_.size(); i++)
      cpyBranch(other.all_branchs_[i], all_branchs_[i]);
  }

  void RuleGraph::cpyBranch(RuleBranch* old_branch, RuleBranch* new_branch)
  {
    new_branch->nb_updates_ = old_branch->nb_updates_;
    new_branch->setUpdated(old_branch->isUpdated());
    new_branch->flags_ = old_branch->flags_;

    // addon
    new_branch->involves_class = old_branch->involves_class;
    new_branch->involves_object_property = old_branch->involves_object_property;
    new_branch->involves_data_property = old_branch->involves_data_property;

    for(const auto& elem : old_branch->rule_body_)
      new_branch->rule_body_.emplace_back(createCopyRuleTriplet(elem, new_branch));

    for(const auto& elem : old_branch->rule_head_)
      new_branch->rule_head_.emplace_back(createCopyRuleTriplet(elem, new_branch));

    new_branch->atom_initial_order_ = old_branch->atom_initial_order_;
  }

  RuleTriplet_t RuleGraph::createCopyRuleTriplet(const RuleTriplet_t& old_triplet, RuleBranch* new_branch)
  {
    RuleTriplet_t new_triplet;
    new_triplet.atom_type_ = old_triplet.atom_type_;

    if(old_triplet.class_predicate != nullptr)
      new_triplet.class_predicate = class_graph_->container_.find(old_triplet.class_predicate->value());
    
    if(old_triplet.anonymous_element != nullptr)
      new_triplet.anonymous_element = anonymous_graph_->container_.find(old_triplet.anonymous_element->value());

    if(old_triplet.data_predicate != nullptr)
      new_triplet.data_predicate = data_property_graph_->container_.find(old_triplet.data_predicate->value());

    if(old_triplet.object_predicate != nullptr)
      new_triplet.object_predicate = object_property_graph_->container_.find(old_triplet.object_predicate->value());

    new_triplet.builtin = old_triplet.builtin;

    for(const auto& arg : old_triplet.arguments)
    {
      RuleArgument_t new_arg;
      new_arg.name = arg.name;
      new_arg.variable_id = arg.variable_id;
      new_arg.is_variable = arg.is_variable;
      if(arg.indiv_value != nullptr)
        new_arg.indiv_value = individual_graph_->container_.find(arg.indiv_value->value());
      if(arg.datatype_value != nullptr)
        new_arg.datatype_value = literal_graph_->find(arg.datatype_value->value());

      new_triplet.arguments.push_back(new_arg);

      setVariableIndex(new_branch, new_arg);
    }

    return new_triplet;
  }

} // namespace ontologenius