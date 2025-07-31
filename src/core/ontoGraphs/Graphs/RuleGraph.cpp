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
#include "ontologenius/core/ontoGraphs/Graphs/ObjectPropertyGraph.h"
#include "ontologenius/utils/String.h"

// #define DEBUG

namespace ontologenius {

  RuleGraph::RuleGraph(ClassGraph* class_graph, ObjectPropertyGraph* object_property_graph, DataPropertyGraph* data_property_graph,
                       IndividualGraph* individual_graph, AnonymousClassGraph* anonymous_graph) : class_graph_(class_graph),
                                                                                                  object_property_graph_(object_property_graph),
                                                                                                  data_property_graph_(data_property_graph),
                                                                                                  individual_graph_(individual_graph),
                                                                                                  anonymous_graph_(anonymous_graph)
  {}

  RuleGraph::RuleGraph(const RuleGraph& other, ClassGraph* class_graph,
                       ObjectPropertyGraph* object_property_graph,
                       DataPropertyGraph* data_property_graph,
                       IndividualGraph* individual_graph,
                       AnonymousClassGraph* anonymous_graph) : // Graph ccopy constructor is not called as RuleBranch need more advanced copy
                                                               class_graph_(class_graph),
                                                               object_property_graph_(object_property_graph),
                                                               data_property_graph_(data_property_graph),
                                                               individual_graph_(individual_graph),
                                                               anonymous_graph_(anonymous_graph)
  {
    for(auto* branch : other.all_branchs_)
    {
      auto* rule_branch = new RuleBranch(branch->value(), branch->getRule(), branch->isHidden());
      all_branchs_.push_back(rule_branch);
    }
  }

  RuleBranch* RuleGraph::add(const std::size_t& rule_id, Rule_t& rule)
  {
    const std::lock_guard<std::shared_timed_mutex> lock(Graph<RuleBranch>::mutex_);

    const std::string rule_name = "rule_" + std::to_string(rule_id);
    RuleBranch* rule_branch = new RuleBranch(rule_name, rule.rule_str);
    all_branchs_.push_back(rule_branch);

    size_t elem_id = 0;

    for(auto& atom_antec : rule.antecedents)
    {
      if(atom_antec.first != nullptr)
      {
        rule_branch->rule_body_.push_back(createRuleAtomTriplet(rule_branch, atom_antec, rule_id, elem_id, true));
        rule_branch->atom_initial_order_.push_back(rule_branch->rule_body_.size() - 1);
      }
      elem_id++;
    }
    for(auto& atom_conseq : rule.consequents)
    {
      if(atom_conseq.first != nullptr)
        rule_branch->rule_head_.push_back(createRuleAtomTriplet(rule_branch, atom_conseq, rule_id, elem_id, false));
      elem_id++;
    }

    return rule_branch;
  }

  RuleTriplet_t RuleGraph::createRuleAtomTriplet(RuleBranch* rule_branch, const std::pair<ontologenius::ExpressionMember_t*, std::vector<ontologenius::Variable_t>>& rule_element, const size_t& rule_id, const size_t& elem_id, const bool& is_head)
  {
    auto* rule_atom = rule_element.first;
    auto rule_variable = rule_element.second;

    if(rule_atom->builtin_.builtin_type_ != builtin_none)
      return createBuiltinTriplet(rule_branch, rule_atom, rule_variable);
    else if((rule_atom->is_data_property) && (rule_atom->logical_type_ == logical_none) && (!rule_atom->is_complex))
    {
      if(is_head)
        rule_branch->involves_data_property = true;
      return createDataPropertyTriplet(rule_branch, rule_atom, rule_variable);
    }
    else if((rule_atom->logical_type_ == logical_none) && (!rule_atom->is_complex) && (rule_atom->rest.restriction_range.empty()))
    {
      if(is_head)
        rule_branch->involves_object_property = true;
      return createObjectPropertyTriplet(rule_branch, rule_atom, rule_variable);
    }
    else
    {
      if(is_head)
        rule_branch->involves_class = true;
      return createClassTriplet(rule_branch, rule_atom, rule_variable.front(), rule_id, elem_id);
    }
  }

  RuleTriplet_t RuleGraph::createClassTriplet(RuleBranch* rule_branch, ExpressionMember_t* class_member, const Variable_t& variable, const size_t& rule_id, const size_t& elem_id)
  {
    if(class_member->logical_type_ != logical_none || class_member->oneof == true || class_member->is_complex == true || !class_member->rest.property.empty())
    {
      RuleResource_t resource = getRuleResource(rule_branch, variable);                                            // create the variable
      AnonymousClassBranch* rule_ano_branch = anonymous_graph_->addHiddenRuleElem(rule_id, elem_id, class_member); // returns the newly created ano branch
      AnonymousClassElement* ano_elem = rule_ano_branch->ano_elems_.front();                                       // complex expression
      ClassBranch* hidden = rule_ano_branch->class_equiv_;

      return RuleTriplet_t(hidden, ano_elem, resource);
    }
    else
    {
      RuleResource_t resource = getRuleResource(rule_branch, variable); // create the variable
      ClassBranch* class_branch = class_graph_->findOrCreateBranch(class_member->rest.restriction_range);

      return RuleTriplet_t(class_branch, resource);
    }
  }

  RuleTriplet_t RuleGraph::createObjectPropertyTriplet(RuleBranch* rule_branch, ExpressionMember_t* property_member, const std::vector<Variable_t>& variable)
  {
    RuleResource_t resource_from = getRuleResource(rule_branch, variable.front());
    ObjectPropertyBranch* property = object_property_graph_->findOrCreateBranch(property_member->rest.property);
    RuleResource_t resource_on = getRuleResource(rule_branch, variable.back());

    return RuleTriplet_t(resource_from, property, resource_on);
  }

  RuleTriplet_t RuleGraph::createDataPropertyTriplet(RuleBranch* rule_branch, ExpressionMember_t* property_member, const std::vector<Variable_t>& variable)
  {
    RuleResource_t resource_from = getRuleResource(rule_branch, variable.front());
    DataPropertyBranch* property = data_property_graph_->findOrCreateBranch(property_member->rest.property);
    RuleResource_t resource_on = getRuleResource(rule_branch, variable.back());

    return RuleTriplet_t(resource_from, property, resource_on);
  }

  RuleTriplet_t RuleGraph::createBuiltinTriplet(RuleBranch* rule_branch, ExpressionMember_t* property_member, const std::vector<Variable_t>& variable)
  {
    RuleResource_t var_from = getRuleResource(rule_branch, variable.front());
    RuleResource_t var_on = getRuleResource(rule_branch, variable.back());

    return RuleTriplet_t(var_from, property_member->builtin_, var_on);
  }

  RuleResource_t RuleGraph::getRuleResource(RuleBranch* rule_branch, const Variable_t& variable)
  {
    if(variable.is_instantiated == true)
    {
      // individual
      IndividualBranch* involved_indiv = individual_graph_->findOrCreateBranch(variable.var_name);
      RuleResource_t resource(involved_indiv);
      setVariableIndex(rule_branch, resource);
      return resource;
    }
    else if(variable.is_datavalue == true)
    {
      // literal
      LiteralNode* involved_datatype = data_property_graph_->createLiteral(variable.var_name); // invalid use of incomplete type ‘class ontologenius::DataPropertyGraph’
      RuleResource_t resource(involved_datatype);
      setVariableIndex(rule_branch, resource);
      return resource;
    }
    else
    {
      // variable
      RuleResource_t resource(variable.var_name);
      setVariableIndex(rule_branch, resource);
      return resource;
    }
  }

  void RuleGraph::setVariableIndex(RuleBranch* rule_branch, RuleResource_t& resource)
  {
    auto var_it = rule_branch->variables_.find(resource.name);
    if(var_it == rule_branch->variables_.end()) // if the variable doesn't already exist in the variables_ vector of the rule
    {
      int64_t index = std::int64_t(rule_branch->variables_.size());
      rule_branch->variables_[resource.name] = index;
      resource.variable_id = index;
      rule_branch->to_variables_.push_back(resource.name);
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

    new_branch->dictionary_ = old_branch->dictionary_;
    new_branch->steady_dictionary_ = old_branch->steady_dictionary_;

    // addon
    new_branch->involves_class = old_branch->involves_class;
    new_branch->involves_object_property = old_branch->involves_object_property;
    new_branch->involves_data_property = old_branch->involves_data_property;

    size_t elem_id = 0;
    size_t rule_id = std::stoi(split(old_branch->value(), "_").back());

    for(const auto& elem : old_branch->rule_body_)
    {
      new_branch->rule_body_.emplace_back(createCopyRuleTriplet(elem, new_branch, rule_id, elem_id));
      elem_id++;
    }

    for(const auto& elem : old_branch->rule_head_)
    {
      new_branch->rule_head_.emplace_back(createCopyRuleTriplet(elem, new_branch, rule_id, elem_id));
      elem_id++;
    }

    new_branch->atom_initial_order_ = old_branch->atom_initial_order_;
  }

  RuleTriplet_t RuleGraph::createCopyRuleTriplet(RuleTriplet_t old_triplet, RuleBranch* new_branch, const size_t& rule_id, const size_t& elem_id)
  {
    switch(old_triplet.atom_type_)
    {
    case class_atom:
    {
      if(old_triplet.class_element == nullptr)
      {
        ClassBranch* simple_class = class_graph_->container_.find(old_triplet.class_predicate->value());
        RuleResource_t resource;
        if(old_triplet.subject.indiv_value != nullptr)
          resource = RuleResource_t(individual_graph_->container_.find(old_triplet.subject.indiv_value->value()));
        else
          resource = RuleResource_t(old_triplet.subject.name);
        setVariableIndex(new_branch, resource);

        return RuleTriplet_t(simple_class, resource);
      }
      else
      {
        RuleResource_t resource;
        if(old_triplet.subject.indiv_value != nullptr)
          resource = RuleResource_t(individual_graph_->container_.find(old_triplet.subject.indiv_value->value()));
        else
          resource = RuleResource_t(old_triplet.subject.name);
        setVariableIndex(new_branch, resource);

        AnonymousClassBranch* rule_ano_branch = anonymous_graph_->addCopyHiddenRuleElem(rule_id, elem_id, old_triplet.class_element); // returns the newly created ano branch
        AnonymousClassElement* ano_elem = rule_ano_branch->ano_elems_.front();                                                        // complex expression
        ClassBranch* hidden = rule_ano_branch->class_equiv_;

        return RuleTriplet_t(hidden, ano_elem, resource);
      }
    }
    case object_atom:
    {
      RuleResource_t resource_from;
      if(old_triplet.subject.indiv_value != nullptr)
        resource_from = RuleResource_t(individual_graph_->container_.find(old_triplet.subject.indiv_value->value()));
      else
        resource_from = RuleResource_t(old_triplet.subject.name);
      setVariableIndex(new_branch, resource_from);

      ObjectPropertyBranch* property = object_property_graph_->container_.find(old_triplet.object_predicate->value());

      RuleResource_t resource_on;
      if(old_triplet.object.indiv_value != nullptr)
        resource_on = RuleResource_t(individual_graph_->container_.find(old_triplet.object.indiv_value->value()));
      else
        resource_on = RuleResource_t(old_triplet.object.name);
      setVariableIndex(new_branch, resource_on);

      return RuleTriplet_t(resource_from, property, resource_on);
    }
    case data_atom:
    {
      RuleResource_t resource_from;
      if(old_triplet.subject.indiv_value != nullptr)
        resource_from = RuleResource_t(individual_graph_->container_.find(old_triplet.subject.indiv_value->value()));
      else
        resource_from = RuleResource_t(old_triplet.subject.name);
      setVariableIndex(new_branch, resource_from);
      DataPropertyBranch* property = data_property_graph_->container_.find(old_triplet.data_predicate->value());

      RuleResource_t resource_on;
      if(old_triplet.object.datatype_value != nullptr)
        resource_on = old_triplet.object.datatype_value->value(); // cannot access the literal nodes through the dataproperty graph
      else
        resource_on = RuleResource_t(old_triplet.object.name);
      setVariableIndex(new_branch, resource_on);
      return RuleTriplet_t(resource_from, property, resource_on);
    }

    case builtin_atom:
    {
      RuleResource_t var_from, var_on;
      setVariableIndex(new_branch, var_from);
      setVariableIndex(new_branch, var_on);
      return RuleTriplet_t(var_from, old_triplet.builtin, var_on);
    }

    default:
      return RuleTriplet_t();
    }
  }

} // namespace ontologenius