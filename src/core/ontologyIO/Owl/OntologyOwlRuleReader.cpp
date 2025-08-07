#include <cstddef>
#include <iostream>
#include <string>
#include <tinyxml2.h>
#include <utility>
#include <vector>

#include "ontologenius/core/ontoGraphs/Branchs/AnonymousClassBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/RuleBranch.h"
#include "ontologenius/core/ontoGraphs/Graphs/AnonymousClassGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/RuleGraph.h"
#include "ontologenius/core/ontologyIO/Owl/OntologyOwlReader.h"
#include "ontologenius/utils/String.h"

namespace ontologenius {

  Rule_t OntologyOwlReader::readRuleDescription(tinyxml2::XMLElement* elem)
  {
    Rule_t rule;
    // read body
    auto* rule_body = elem->FirstChildElement("swrl:body");
    if(rule_body == nullptr)
      return rule;
    readRuleCollection(rule_body, rule.antecedents);

    // read head
    auto* rule_head = elem->FirstChildElement("swrl:head");
    if(rule_head == nullptr)
      return rule;
    readRuleCollection(rule_head, rule.consequents);

    // compute the full str rule
    rule.rule_str = rule.toStringRule();

    return rule;
  }

  void OntologyOwlReader::readRuleCollection(tinyxml2::XMLElement* elem, std::vector<std::pair<ExpressionMember_t*, std::vector<Variable_t>>>& exp_vect)
  {
    auto* sub_elem = elem->FirstChildElement("rdf:Description");
    if(sub_elem != nullptr)
      readRuleCollection(sub_elem, exp_vect);
    else
    {
      // Atom Type
      std::pair<ontologenius::ExpressionMember_t*, std::vector<Variable_t>> res;
      auto* type_elem = elem->FirstChildElement("rdf:type");
      if(type_elem != nullptr)
      {
        const char* type_resource = nullptr;
        type_resource = type_elem->Attribute("rdf:resource");
        if(type_resource != nullptr)
        {
          std::string type_name = getName(std::string(type_resource));
          if(type_name == "AtomList")
          {
            // read first elem
            auto* first_elem = elem->FirstChildElement("rdf:first");
            if(first_elem != nullptr)
              readFirstAtom(first_elem, exp_vect);
            else
              return;
            // read rest elem
            auto* rest_elem = elem->FirstChildElement("rdf:rest");
            if(rest_elem != nullptr)
              readRestAtom(rest_elem, exp_vect);
            else
              return;
          }
          else
          {
            res = readRuleAtom(elem, type_name);
            exp_vect.push_back(res);
          }
        }
      }
      else
        return;
    }
  }

  std::pair<ExpressionMember_t*, std::vector<Variable_t>> OntologyOwlReader::readRuleAtom(tinyxml2::XMLElement* elem, const std::string& type_atom)
  {
    std::pair<ExpressionMember_t*, std::vector<Variable_t>> res;

    if(type_atom == "ClassAtom")
      res = readRuleClassAtom(elem);
    else if(type_atom == "IndividualPropertyAtom")
      res = readRuleObjectPropertyAtom(elem);
    else if(type_atom == "DatavaluedPropertyAtom")
      res = readRuleDataPropertyAtom(elem);
    else if(type_atom == "BuiltinAtom")
      res = readRuleBuiltinAtom(elem);

    return res;
  }

  void OntologyOwlReader::readFirstAtom(tinyxml2::XMLElement* elem, std::vector<std::pair<ExpressionMember_t*, std::vector<Variable_t>>>& exp_vect)
  {
    readRuleCollection(elem, exp_vect);
  }

  void OntologyOwlReader::readRestAtom(tinyxml2::XMLElement* elem, std::vector<std::pair<ExpressionMember_t*, std::vector<Variable_t>>>& exp_vect)
  {
    const char* type_rest = nullptr;
    type_rest = elem->Attribute("rdf:resource");
    if(type_rest != nullptr)
    {
      std::string rest_name = getName(std::string(type_rest));
      if(rest_name == "nil")
        return; // TODO; why ?? at the moment we enter the prebious if we will exist the same
    }
    else
      readRuleCollection(elem, exp_vect);
  }

  std::pair<ExpressionMember_t*, std::vector<Variable_t>> OntologyOwlReader::readRuleClassAtom(tinyxml2::XMLElement* elem)
  {
    auto* class_pred = elem->FirstChildElement("swrl:classPredicate");
    if(class_pred != nullptr)
    {
      ExpressionMember_t* temp_exp = nullptr;
      //  check for the class expression
      const char* class_resource = nullptr;
      class_resource = class_pred->Attribute("rdf:resource");

      if(class_resource != nullptr)
        temp_exp = readRuleResource(class_pred, "rdf:resource");
      else
      {
        //  complex class expression
        auto* restriction_elem = class_pred->FirstChildElement("owl:Restriction");   // simple restriction : hasComponent some Component
        auto* complex_restriction_elem = class_pred->FirstChildElement("owl:Class"); // complex restriction : (hasComponent some Component and has_node only rational)

        if(restriction_elem != nullptr)
          temp_exp = readRuleRestriction(restriction_elem);
        else if(complex_restriction_elem != nullptr)
          temp_exp = readRuleClassExpression(complex_restriction_elem);
      }

      auto variables = readRuleVariables(elem);
      // todo: raise exception to stop local reading in case of wrong number of args

      return {temp_exp, variables};
    }
    else
      return {nullptr, {}};
  }

  std::pair<ExpressionMember_t*, std::vector<Variable_t>> OntologyOwlReader::readRuleObjectPropertyAtom(tinyxml2::XMLElement* elem)
  {
    auto* obj_pred = elem->FirstChildElement("swrl:propertyPredicate");

    // get Object Property
    if(obj_pred != nullptr)
    {
      const auto* obj_prop = obj_pred->Attribute("rdf:resource");
      if(obj_prop != nullptr)
      {
        ExpressionMember_t* temp_exp = new ExpressionMember_t();
        temp_exp->rest.property = getName(std::string(obj_prop));

        auto variables = readRuleVariables(elem);
        // todo: raise exception to stop local reading in case of wrong number of args

        return {temp_exp, variables};
      }
    }

    return {nullptr, {}};
  }

  std::pair<ExpressionMember_t*, std::vector<Variable_t>> OntologyOwlReader::readRuleDataPropertyAtom(tinyxml2::XMLElement* elem)
  {
    auto* data_pred = elem->FirstChildElement("swrl:propertyPredicate");

    // get Data Property
    if(data_pred != nullptr)
    {
      const auto* data_prop = data_pred->Attribute("rdf:resource");
      if(data_prop != nullptr)
      {
        ExpressionMember_t* temp_exp = new ExpressionMember_t();
        temp_exp->rest.property = getName(std::string(data_prop));
        temp_exp->is_data_property = true;

        auto variables = readRuleVariables(elem);
        // todo: raise exception to stop local reading in case of wrong number of args

        return {temp_exp, variables};
      }
    }

    return {nullptr, {}};
  }

  std::vector<Variable_t> OntologyOwlReader::readRuleVariables(tinyxml2::XMLElement* elem)
  {
    std::vector<Variable_t> res;

    for(size_t index = 1;; index++)
    {
      std::string arg_name = "swrl:argument" + std::to_string(index);
      auto* var_arg = elem->FirstChildElement(arg_name.c_str());
      if(var_arg != nullptr)
        res.push_back(getRuleArgument(var_arg));
      else
        break;
    }

    return res;
  }

  Variable_t OntologyOwlReader::getRuleArgument(tinyxml2::XMLElement* elem)
  {
    Variable_t new_var;
    const auto* var_name_resource = elem->Attribute("rdf:resource");
    const auto* var_name_dataype = elem->Attribute("rdf:datatype");

    if(var_name_resource != nullptr)
    {
      new_var.var_name = getName(std::string(var_name_resource));
      new_var.is_individual = !isIn("swrl:var", var_name_resource);
    }
    else if(var_name_dataype != nullptr)
    {
      new_var.var_name = getName(std::string(var_name_dataype) + "#" + std::string(elem->GetText()));
      new_var.is_datavalue = true;
    }

    return new_var;
  }

  std::pair<ExpressionMember_t*, std::vector<Variable_t>> OntologyOwlReader::readRuleBuiltinAtom(tinyxml2::XMLElement* elem)
  {
    ExpressionMember_t* temp_exp = new ExpressionMember_t();

    // get builtin type
    const auto* builtin_type = elem->FirstChildElement("swrl:builtin")->Attribute("rdf:resource");

    if(builtin_type != nullptr)
    {
      std::string builtin_name = getName(std::string(builtin_type));

      if(builtin_name == "greaterThan")
        temp_exp->builtin_ = Builtin_t(greater_than, builtin_name);
      else if(builtin_name == "greaterThanOrEqual")
        temp_exp->builtin_ = Builtin_t(greater_than_or_equal, builtin_name);
      else if(builtin_name == "lessThan")
        temp_exp->builtin_ = Builtin_t(less_than, builtin_name);
      else if(builtin_name == "lessThanOrEqual")
        temp_exp->builtin_ = Builtin_t(less_than_or_equal, builtin_name);
      else if(builtin_name == "equal")
        temp_exp->builtin_ = Builtin_t(equal, builtin_name);
      else if(builtin_name == "notEqual")
        temp_exp->builtin_ = Builtin_t(not_equal, builtin_name);
      else
      {
        temp_exp->builtin_ = Builtin_t(builtin_none, "none");
        if(display_)
          std::cout << "unsupported buitlin atom : " << builtin_name << std::endl;
      }
    }
    // get builtins arguments in order
    auto* builtin_arguments = elem->FirstChildElement("swrl:arguments");

    std::vector<Variable_t> variables;
    readRuleBuiltinArguments(builtin_arguments, variables);

    return {temp_exp, variables};
  }

  void OntologyOwlReader::readRuleBuiltinArguments(tinyxml2::XMLElement* elem, std::vector<Variable_t>& variables)
  {

    const auto* parsing_type_attribute = elem->Attribute("rdf:parseType");
    if(parsing_type_attribute != nullptr)
    {
      std::string parsing_type = std::string(parsing_type_attribute);
      if(parsing_type == "Collection")
        readBuiltinArgumentsCollection(elem, variables);
      // else // todo: error unsuported parsing type
    }
    else
      readBuiltinArgumentsList(elem, variables);
  }

  void OntologyOwlReader::readBuiltinArgumentsCollection(tinyxml2::XMLElement* elem, std::vector<Variable_t>& variables)
  {
    for(tinyxml2::XMLElement* description_elem = elem->FirstChildElement("rdf:Description");
        description_elem != nullptr; description_elem = description_elem->NextSiblingElement("rdf:Description"))
    {
      const auto* builtin_arg = description_elem->Attribute("rdf:about");
      if(builtin_arg != nullptr)
      {
        variables.emplace_back(); // create an empty variable at the end
        variables.back().is_builtin_value = true;
        variables.back().var_name = std::string(builtin_arg);
      }
    }
  }

  void OntologyOwlReader::readBuiltinArgumentsList(tinyxml2::XMLElement* elem, std::vector<Variable_t>& variables)
  {
    auto* description = elem->FirstChildElement("rdf:Description");
    if(description == nullptr)
      return; // should never happend

    auto* type_element = description->FirstChildElement("rdf:type");
    if(type_element == nullptr)
      return; // should never happend

    const auto* type = type_element->Attribute("rdf:resource");
    if(type != nullptr)
    {
      auto type_str = std::string(type);
      if(type_str == "http://www.w3.org/1999/02/22-rdf-syntax-ns#List")
      {
        auto* first_element = description->FirstChildElement("rdf:first");
        if(first_element != nullptr)
        {
          const auto* attribute = first_element->Attribute("rdf:resource");
          if(attribute != nullptr)
          {
            variables.emplace_back(); // create an empty variable at the end
            variables.back().var_name = getName(std::string(attribute));
          }
          else
          {
            attribute = first_element->Attribute("rdf:datatype");
            if(attribute != nullptr)
            {
              variables.emplace_back(); // create an empty variable at the end
              variables.back().is_datavalue = true;
              variables.back().var_name = getName(std::string(attribute) + "#" + std::string(first_element->GetText()));
            }
          }
        }

        auto* rest_element = description->FirstChildElement("rdf:rest");
        if(rest_element != nullptr)
        {
          const auto* resource = rest_element->Attribute("rdf:resource");
          if(resource == nullptr) // if not nullptr should be nil
            readRuleBuiltinArguments(rest_element, variables);
        }
      }
      // else // todo: error
    }
  }

  ExpressionMember_t* OntologyOwlReader::readRuleRestriction(tinyxml2::XMLElement* elem)
  {
    ExpressionMember_t* exp = new ExpressionMember_t();

    // get property
    auto* property_elem = elem->FirstChildElement("owl:onProperty");
    if(property_elem != nullptr)
      exp->rest.property = getName(property_elem->Attribute("rdf:resource"));

    // get cardinality
    for(tinyxml2::XMLElement* sub_elem = elem->FirstChildElement(); sub_elem != nullptr; sub_elem = sub_elem->NextSiblingElement())
    {
      const std::string sub_elem_name = sub_elem->Value();
      if((sub_elem_name == "owl:maxQualifiedCardinality") ||
         (sub_elem_name == "owl:minQualifiedCardinality") ||
         (sub_elem_name == "owl:qualifiedCardinality"))
      {
        readRuleCardinalityValue(sub_elem, exp);
        break;
      }
      else if((sub_elem_name == "owl:allValuesFrom") ||
              (sub_elem_name == "owl:hasValue") ||
              (sub_elem_name == "owl:someValuesFrom"))
      {
        if(readRuleCardinalityRange(sub_elem, exp) == false)
        {
          exp->is_complex = true;
          addRuleChildMember(exp, readRuleComplexDescription(sub_elem->FirstChildElement()), sub_elem->FirstChildElement());
          break;
        }
      }
    }

    // get range
    auto* class_elem = elem->FirstChildElement("owl:onClass");
    auto* data_elem = elem->FirstChildElement("owl:onDataRange");

    if(class_elem != nullptr)
    {
      exp->is_data_property = false;
      const char* resource = class_elem->Attribute("rdf:resource");
      // Simple restriction range : Camera Eq to  hasComponent some Lidar
      if(resource != nullptr)
        exp->rest.restriction_range = getName(resource);
      // Complex restriction range with max, min, exactly : Camera Eq to  hasComponent max 2 (not DirtyCutlery)
      else
      {
        exp->is_complex = true;
        addRuleChildMember(exp, readRuleComplexDescription(class_elem->FirstChildElement()), class_elem->FirstChildElement());
      }
    }
    else if(data_elem != nullptr)
    {
      exp->is_data_property = true;
      const char* resource = data_elem->Attribute("rdf:resource");
      // Simple restriction range : Camera Eq to  has_node some boolean
      if(resource != nullptr)
        exp->rest.restriction_range = resource;
      // Complex restriction range with max, min, exactly : Camera Eq to  has_node some (not(boolean))
      else
      {
        exp->is_complex = true;
        addRuleChildMember(exp, readRuleComplexDescription(data_elem->FirstChildElement()), data_elem->FirstChildElement());
      }
    }
    return exp;
  }

  ExpressionMember_t* OntologyOwlReader::readRuleClassExpression(tinyxml2::XMLElement* elem)
  {
    // check for type of node
    auto* union_node = elem->FirstChildElement("owl:unionOf");
    if(union_node != nullptr)
      return readRuleUnion(union_node);

    auto* inter_node = elem->FirstChildElement("owl:intersectionOf");
    if(inter_node != nullptr)
      return readRuleIntersection(inter_node);

    auto* negat_node = elem->FirstChildElement("owl:complementOf");
    if(negat_node != nullptr)
      return readRuleComplement(negat_node);

    auto* oneof_node = elem->FirstChildElement("owl:oneOf");
    if(oneof_node != nullptr)
      return readRuleOneOf(oneof_node);

    return nullptr;
  }

  ExpressionMember_t* OntologyOwlReader::readRuleDatatypeExpression(tinyxml2::XMLElement* elem)
  {
    ExpressionMember_t* exp = nullptr;
    // check for type of node
    auto* union_node = elem->FirstChildElement("owl:unionOf");
    if(union_node != nullptr)
      exp = readRuleUnion(union_node);

    auto* inter_node = elem->FirstChildElement("owl:intersectionOf");
    if(inter_node != nullptr)
      exp = readRuleIntersection(inter_node);

    auto* negat_node = elem->FirstChildElement("owl:datatypeComplementOf");
    if(negat_node != nullptr)
      exp = readRuleComplement(negat_node);

    if(exp != nullptr)
    {
      exp->is_data_property = true;
      return exp;
    }
    return nullptr;
  }

  ExpressionMember_t* OntologyOwlReader::readRuleIntersection(tinyxml2::XMLElement* elem)
  {
    ExpressionMember_t* exp = new ExpressionMember_t();
    exp->logical_type_ = logical_and;

    for(tinyxml2::XMLElement* sub_elem = elem->FirstChildElement(); sub_elem != nullptr; sub_elem = sub_elem->NextSiblingElement())
    {
      ExpressionMember_t* child_exp = readRuleComplexDescription(sub_elem);
      addRuleChildMember(exp, child_exp, sub_elem);
    }

    return exp;
  }

  ExpressionMember_t* OntologyOwlReader::readRuleUnion(tinyxml2::XMLElement* elem)
  {
    ExpressionMember_t* exp = new ExpressionMember_t();
    exp->logical_type_ = logical_or;

    for(tinyxml2::XMLElement* sub_elem = elem->FirstChildElement(); sub_elem != nullptr; sub_elem = sub_elem->NextSiblingElement())
    {
      ExpressionMember_t* child_exp = readRuleComplexDescription(sub_elem);
      addRuleChildMember(exp, child_exp, sub_elem);
    }

    return exp;
  }

  ExpressionMember_t* OntologyOwlReader::readRuleOneOf(tinyxml2::XMLElement* elem)
  {
    ExpressionMember_t* exp = new ExpressionMember_t();
    exp->oneof = true;

    for(tinyxml2::XMLElement* sub_elem = elem->FirstChildElement(); sub_elem != nullptr; sub_elem = sub_elem->NextSiblingElement())
    {
      ExpressionMember_t* child_exp = readRuleResource(sub_elem, "rdf:about");
      addRuleChildMember(exp, child_exp, sub_elem);
    }

    return exp;
  }

  ExpressionMember_t* OntologyOwlReader::readRuleComplement(tinyxml2::XMLElement* elem)
  {
    ExpressionMember_t* exp = new ExpressionMember_t();
    exp->logical_type_ = logical_not;
    ExpressionMember_t* child_exp = nullptr;

    const char* resource = elem->Attribute("rdf:resource");

    if(resource != nullptr)
      child_exp = readRuleResource(elem);
    else
      child_exp = readRuleComplexDescription(elem->FirstChildElement());

    addRuleChildMember(exp, child_exp, elem->FirstChildElement());

    return exp;
  }

  ExpressionMember_t* OntologyOwlReader::readRuleComplexDescription(tinyxml2::XMLElement* elem)
  {
    if(elem == nullptr)
      return nullptr;

    const std::string elem_name = elem->Value();
    if(elem_name == "owl:Class")
      return readRuleClassExpression(elem);
    else if(elem_name == "rdfs:Datatype")
      return readRuleDatatypeExpression(elem);
    else if(elem_name == "owl:Restriction")
      return readRuleRestriction(elem);
    else if(elem_name == "rdf:Description")
      return readRuleResource(elem, "rdf:about");
    else
      return nullptr;
  }

  ExpressionMember_t* OntologyOwlReader::readRuleResource(tinyxml2::XMLElement* elem, const std::string& attribute_name)
  {
    ExpressionMember_t* exp = nullptr;

    const char* resource = elem->Attribute(attribute_name.c_str());
    if(resource != nullptr)
    {
      exp = new ExpressionMember_t();

      const std::string attr_class = elem->Attribute(attribute_name.c_str());
      if(isIn("http://www.w3.org/", attr_class))
      {
        exp->rest.restriction_range = attr_class;
        exp->is_data_property = true;
      }
      else
        exp->rest.restriction_range = getName(attr_class);
    }

    return exp;
  }

  void OntologyOwlReader::addRuleChildMember(ExpressionMember_t* parent, ExpressionMember_t* child, tinyxml2::XMLElement* used_elem)
  {
    if(child != nullptr)
    {
      parent->child_members.push_back(child);
      child->mother = parent;
      if((used_elem != nullptr) && (std::string(used_elem->Value()) != "owl:Restriction"))
        parent->is_data_property = child->is_data_property;
    }
  }

  void OntologyOwlReader::readRuleCardinalityValue(tinyxml2::XMLElement* elem, ExpressionMember_t* exp)
  {
    const std::string sub_elem_name = elem->Value();

    exp->rest.card.cardinality_type = card_map_[sub_elem_name];

    if(elem->GetText() != nullptr)
      exp->rest.card.cardinality_number = elem->GetText();
  }

  bool OntologyOwlReader::readRuleCardinalityRange(tinyxml2::XMLElement* elem, ExpressionMember_t* exp)
  {
    const std::string sub_elem_name = elem->Value();

    const char* resource = elem->Attribute("rdf:resource");
    const char* resource_data = elem->Attribute("rdf:datatype");
    exp->rest.card.cardinality_type = card_map_[sub_elem_name];

    if(resource != nullptr)
    {
      const std::string s = resource;
      if(isIn("http://www.w3.org/", s))
      {
        exp->rest.card.cardinality_range = resource;
        exp->is_data_property = true;
      }
      else
        exp->rest.card.cardinality_range = getName(resource);
      return true;
    }
    else if(resource_data != nullptr)
    {
      exp->is_data_property = true;
      exp->rest.card.cardinality_range = getName(resource_data) + "#" + elem->GetText();
      return true;
    }
    return false;
  }

} // namespace ontologenius