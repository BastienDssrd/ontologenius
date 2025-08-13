#include "ontologenius/core/ontologyIO/Owl/writers/RuleOwlWriter.h"

#include <cstddef>
#include <cstdio>
#include <shared_mutex>
#include <string>
#include <vector>

#include "ontologenius/core/ontoGraphs/Branchs/AnonymousClassBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/ClassBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/RuleBranch.h"
#include "ontologenius/core/ontoGraphs/Graphs/RuleGraph.h"
#include "ontologenius/graphical/Display.h"

namespace ontologenius {

  RuleOwlWriter::RuleOwlWriter(RuleGraph* rule_graph, const std::string& ns) : GraphOwlWriter(ns, ""),
                                                                               rule_graph_(rule_graph)
  {}

  void RuleOwlWriter::write(FILE* file)
  {
    file_ = file;

    const std::shared_lock<std::shared_timed_mutex> lock(rule_graph_->mutex_);

    const std::vector<RuleBranch*> rules = rule_graph_->get();

    // write all the variables involved in the rules
    for(const auto& var : rule_graph_->variable_names_)
      writeVariable(var);

    // write the rules
    for(auto* rule : rules)
      writeRule(rule);

    file_ = nullptr;
  }

  void RuleOwlWriter::writeRule(RuleBranch* branch)
  {
    const size_t level = 1;
    const std::string field = "rdf:Description";

    writeString("<" + field + ">\n", level);
    writeString("<rdf:type rdf:resource=\"http://www.w3.org/2003/11/swrl#Imp\"/>\n", level + 1);

    // write the body of the rule
    writeString("<swrl:body>\n", level + 1);
    writeAtom(branch->rule_body_, branch->rule_body_.front(), level + 2);
    writeString("</swrl:body>\n", level + 1);

    // write the head of the rule
    writeString("<swrl:head>\n", level + 1);
    writeAtom(branch->rule_head_, branch->rule_head_.front(), level + 2);
    writeString("</swrl:head>\n", level + 1);

    writeString("</" + field + ">\n", level);
  }

  void RuleOwlWriter::writeAtom(const std::vector<RuleTriplet_t>& atom_list, const RuleTriplet_t& current_atom, size_t level, size_t index)
  {
    const std::string field = "rdf:Description";

    writeString("<" + field + ">\n", level);
    writeString("<rdf:type rdf:resource=\"http://www.w3.org/2003/11/swrl#AtomList\"/>\n", level + 1);

    writeString("<rdf:first>\n", level + 1);
    writeString("<" + field + ">\n", level + 2);

    // write the atom
    switch(current_atom.atom_type_)
    {
    case rule_atom_class:
      writeClassAtom(current_atom, level + 3);
      break;
    case rule_atom_object:
      writeObjectAtom(current_atom, level + 3);
      break;
    case rule_atom_data:
      writeDataAtom(current_atom, level + 3);
      break;
    case rule_atom_builtin:
      writeBuiltinAtom(current_atom, level + 3);
      break;
    default:
      break;
    }

    writeString("</" + field + ">\n", level + 2);
    writeString("</rdf:first>\n", level + 1);

    index++;
    if(index < atom_list.size())
    {
      writeString("<rdf:rest>\n", level + 1);
      writeAtom(atom_list, atom_list[index], level + 2, index);
      writeString("</rdf:rest>\n", level + 1);
    }
    else
      writeString("<rdf:rest rdf:resource=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#nil\"/>\n", level + 1);

    writeString("</" + field + ">\n", level);
  }

  void RuleOwlWriter::writeClassAtom(const RuleTriplet_t& class_atom, size_t level)
  {
    const std::string field_name = "swrl:classPredicate";

    // writing the Atom Type
    writeString("<rdf:type rdf:resource=\"http://www.w3.org/2003/11/swrl#ClassAtom\"/>\n", level);
    if(class_atom.anonymous_element == nullptr)
      writeString("<" + field_name + " " + getRdfResource(class_atom.class_predicate->value()) + "/>\n", level + 1);
    else if(class_atom.anonymous_element->ano_trees_.empty() == false)
    {
      writeString("<" + field_name + ">\n", level);
      auto* root_node = class_atom.anonymous_element->ano_trees_.front()->root_node_;
      if(root_node->logical_type_ != logical_none)
          writeClassExpression(root_node, level + 1);
      else if(root_node->is_complex || root_node->oneof ||
              (root_node->data_property_involved_ != nullptr) ||
              (root_node->object_property_involved_ != nullptr)) // complex class
        writeRestriction(root_node, level + 1);

      writeString("</" + field_name + ">\n", level);
    }

    writeRuleArguments(class_atom.arguments, level);
  }

  void RuleOwlWriter::writeObjectAtom(const RuleTriplet_t& object_atom, size_t level)
  {
    writeString("<rdf:type rdf:resource=\"http://www.w3.org/2003/11/swrl#IndividualPropertyAtom\"/>\n", level);
    writeString("<swrl:propertyPredicate " + getRdfResource(object_atom.object_predicate->value()) + "/>\n", level);
    writeRuleArguments(object_atom.arguments, level);
  }

  void RuleOwlWriter::writeDataAtom(const RuleTriplet_t& data_atom, size_t level)
  {
    writeString("<rdf:type rdf:resource=\"http://www.w3.org/2003/11/swrl#DatavaluedPropertyAtom\"/>\n", level);
    writeString("<swrl:propertyPredicate " + getRdfResource(data_atom.data_predicate->value()) + "/>\n", level);
    writeRuleArguments(data_atom.arguments, level);
  }

  void RuleOwlWriter::writeBuiltinAtom(const RuleTriplet_t& builtin_atom, size_t level)
  {
    writeString("<rdf:type rdf:resource=\"http://www.w3.org/2003/11/swrl#BuiltinAtom\"/>\n", level);
    writeString("<swrl:builtin rdf:resource=\"http://www.w3.org/2003/11/swrlb#" + builtin_atom.builtinToString() + "\"/>\n", level);
    writeRuleBuiltinArguments(builtin_atom.arguments, 0, level);
  }

  void RuleOwlWriter::writeRuleBuiltinArguments(const std::vector<RuleArgument_t>& arguments, size_t index, size_t level)
  {
    if(index >= arguments.size())
    {
      writeString("<rdf:rest rdf:resource=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#nil\"/>", level);
      return;
    }

    bool use_collection = arguments.at(index).is_variable;
    if(use_collection)
    {
      for(size_t i = index + 1; i < arguments.size(); i++)
        if(arguments.at(i).is_variable == false)
        {
          use_collection = false;
          break;
        }
    }

    std::string key = (index == 0) ? "swrl:arguments" : "rdf:rest";

    if(use_collection)
    {
      writeString("<" + key + " rdf:parseType=\"Collection\">\n", level);
      for(size_t i = index; i < arguments.size(); i++)
        writeString(getArgumentString(arguments.at(i), "rdf:Description"), level + 1);
      writeString("</" + key + ">\n", level);
    }
    else
    {
      writeString("<" + key + ">\n", level);
      writeString("<rdf:Description>\n", level + 1);
      writeString("<rdf:type rdf:resource=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#List\"/>\n", level + 2);
      writeString(getArgumentString(arguments.at(index), "rdf:first"), level + 2);
      writeRuleBuiltinArguments(arguments, index + 1, level + 2);
      writeString("</rdf:Description>\n", level + 1);
      writeString("</" + key + ">\n", level);
    }
  }

  void RuleOwlWriter::writeRuleArguments(const std::vector<RuleArgument_t>& arguments, size_t level)
  {
    for(size_t i = 0; i < arguments.size(); i++)
      writeString(getArgumentString(arguments.at(i), "swrl:argument" + std::to_string(i+1)), level);
  }

  std::string RuleOwlWriter::getArgumentString(const RuleArgument_t& arg, const std::string& key)
  {
    if(arg.datatype_value != nullptr)
      return "<" + key + " rdf:datatype=\"" + arg.datatype_value->type_->getNamespace() + "#" +arg.datatype_value->type_->value() + "\">" +
            arg.datatype_value->data() + "</" + key + ">";
    else if(arg.indiv_value != nullptr)
      return "<" + key + " " + getRdfResource(arg.indiv_value->value()) + "/>";
    else
      return "<" + key + " rdf:resource=\"urn:swrl:var#" + arg.name + "\"/>";
  }

  void RuleOwlWriter::writeVariable(const std::string& rule_variable)
  {
    const size_t level = 1;
    std::string field = "rdf:Description";

    writeString("<" + field + " rdf:about=\"urn:swrl:var#" + rule_variable + "\">\n", level);
    writeString("<rdf:type rdf:resource=\"http://www.w3.org/2003/11/swrl#Variable\"/>\n", level + 1);
    writeString("</" + field + ">\n", level);
  }

  // just anonymous class there bellow

  void RuleOwlWriter::writeEquivalentClass(ClassBranch* branch)
  {
    AnonymousClassBranch* equiv = branch->equiv_anonymous_class_;

    if(equiv != nullptr)
    {
      for(auto* tree : equiv->ano_trees_)
      {
        std::string field = "owl:equivalentClass";
        const size_t level = 2;

        auto* tree_root_node = tree->root_node_;

        // single expression
        if(tree_root_node->sub_elements_.empty() &&
           tree_root_node->class_involved_ != nullptr &&
           tree_root_node->object_property_involved_ == nullptr)
        {
          writeString("<" + field + " " + getResource(tree_root_node) + "/>\n", level);
        }
        // Collection of expressions
        else
        {
          writeString("<" + field + ">\n", level);

          if(tree_root_node->logical_type_ != logical_none || tree_root_node->oneof == true)
            writeClassExpression(tree_root_node, level + 1);
          else
            writeRestriction(tree_root_node, level + 1);

          writeString("</" + field + ">\n", level);
        }
      }
    }
  }

  void RuleOwlWriter::writeRestriction(AnonymousClassElement* ano_elem, size_t level)
  {
    std::string field = "owl:Restriction";
    std::string subfield = "owl:onProperty";

    writeString("<" + field + ">\n", level);

    // Property
    std::string tmp = "<" + subfield + " " + getResource(ano_elem, "rdf:resource", true) + "/>\n";
    writeString(tmp, level + 1);

    // Cardinality
    if(ano_elem->card_.card_type_ == cardinality_max ||
       ano_elem->card_.card_type_ == cardinality_min ||
       ano_elem->card_.card_type_ == cardinality_exactly)
    {
      writeCardinalityValue(ano_elem, level + 1);
    }
    else if(ano_elem->card_.card_type_ == cardinality_only ||
            ano_elem->card_.card_type_ == cardinality_some ||
            ano_elem->card_.card_type_ == cardinality_value)
    {
      if(ano_elem->data_property_involved_ != nullptr)
        writeCardinalityRange(ano_elem, level + 1, true);
      else
        writeCardinalityRange(ano_elem, level + 1, false);
    }

    writeString("</" + field + ">\n", level);
  }

  void RuleOwlWriter::writeClassExpression(AnonymousClassElement* ano_elem, size_t level)
  {
    std::string field = "owl:Class";

    writeString("<" + field + ">\n", level);

    if(ano_elem->logical_type_ == logical_or)
      writeUnion(ano_elem, level + 1);
    else if(ano_elem->logical_type_ == logical_and)
      writeIntersection(ano_elem, level + 1);
    else if(ano_elem->logical_type_ == logical_not)
      writeComplement(ano_elem, level + 1);
    else if(ano_elem->oneof == true)
      writeOneOf(ano_elem, level + 1);

    writeString("</" + field + ">\n", level);
  }

  void RuleOwlWriter::writeDatatypeExpression(AnonymousClassElement* ano_elem, size_t level)
  {
    std::string field = "rdfs:Datatype";

    writeString("<" + field + ">\n", level);

    if(ano_elem->logical_type_ == logical_or)
      writeUnion(ano_elem, level + 1, true);
    else if(ano_elem->logical_type_ == logical_and)
      writeIntersection(ano_elem, level + 1, true);
    else if(ano_elem->logical_type_ == logical_not)
      writeDataComplement(ano_elem, level + 1);

    writeString("</" + field + ">\n", level);
  }

  void RuleOwlWriter::writeIntersection(AnonymousClassElement* ano_elem, size_t level, bool is_data_prop)
  {
    std::string field = "owl:intersectionOf";

    writeString("<" + field + " " + "rdf:parseType=\"Collection\">\n", level);

    for(auto* child : ano_elem->sub_elements_)
      writeComplexDescription(child, level + 1, is_data_prop);

    writeString("</" + field + ">\n", level);
  }

  void RuleOwlWriter::writeUnion(AnonymousClassElement* ano_elem, size_t level, bool is_data_prop)
  {
    const std::string field = "owl:unionOf";

    writeString("<" + field + " " + "rdf:parseType=\"Collection\">\n", level);

    for(auto* child : ano_elem->sub_elements_)
      writeComplexDescription(child, level + 1, is_data_prop);

    writeString("</" + field + ">\n", level);
  }

  void RuleOwlWriter::writeOneOf(AnonymousClassElement* ano_elem, size_t level)
  {
    std::string tmp;
    const std::string field = "owl:oneOf";

    writeString("<" + field + " " + " rdf:parseType=\"Collection\">\n", level);

    for(auto* child : ano_elem->sub_elements_)
    {
      tmp = "<rdf:Description " + getResource(child, "rdf:about") + "/>\n";
      writeString(tmp, level + 1);
    }

    writeString("</" + field + ">\n", level);
  }

  void RuleOwlWriter::writeComplement(AnonymousClassElement* ano_elem, size_t level)
  {
    std::string tmp;
    const std::string field = "owl:complementOf";

    if(ano_elem->sub_elements_.front()->class_involved_ != nullptr && ano_elem->object_property_involved_ == nullptr)
    {
      tmp = "<" + field + " " + getResource(ano_elem->sub_elements_.front()) + "/>\n";
      writeString(tmp, level);
    }
    else
    {
      writeString("<" + field + ">\n", level);

      writeComplexDescription(ano_elem->sub_elements_.front(), level + 1);

      writeString("</" + field + ">\n", level);
    }
  }

  void RuleOwlWriter::writeDataComplement(AnonymousClassElement* ano_elem, size_t level)
  {
    std::string tmp;
    const std::string field = "owl:datatypeComplementOf";

    if(ano_elem->sub_elements_.front()->card_.card_value_range_ != nullptr)
    {
      tmp = "<" + field + " " + getResource(ano_elem->sub_elements_.front()) + "/>\n";
      writeString(tmp, level);
    }
    else
    {
      writeString("<" + field + ">\n", level);

      writeDatatypeExpression(ano_elem->sub_elements_.front(), level + 1);

      writeString("</" + field + ">\n", level);
    }
  }

  void RuleOwlWriter::writeComplexDescription(AnonymousClassElement* ano_elem, size_t level, bool is_data_prop)
  {
    std::string tmp;

    if(ano_elem->sub_elements_.empty())
    {
      if(ano_elem->object_property_involved_ == nullptr && ano_elem->data_property_involved_ == nullptr)
      {
        tmp = "<rdf:Description " + getResource(ano_elem, "rdf:about") + "/>\n";
        writeString(tmp, level);
      }
      else
        writeRestriction(ano_elem, level);
    }
    else if(ano_elem->card_.card_type_ == cardinality_none)
    {
      if(is_data_prop)
        writeDatatypeExpression(ano_elem, level);
      else
        writeClassExpression(ano_elem, level);
    }
    else
      writeRestriction(ano_elem, level);
  }

  void RuleOwlWriter::writeCardinalityValue(AnonymousClassElement* ano_elem, size_t level)
  {
    std::string field;

    switch(ano_elem->card_.card_type_)
    {
    case cardinality_none:
      break;
    case cardinality_exactly:
      field = "owl:qualifiedCardinality";
      break;
    case cardinality_min:
      field = "owl:minQualifiedCardinality";
      break;
    case cardinality_max:
      field = "owl:maxQualifiedCardinality";
      break;
    case cardinality_error:
      Display::error("cardinality type error");
      break;
    default:
      Display::error("cardinality type " + std::to_string(ano_elem->card_.card_type_) + " not supported by this function");
      break;
    }

    std::string tmp = "<" + field + " rdf:datatype=\"http://www.w3.org/2001/XMLSchema#nonNegativeInteger\">" +
          std::to_string(ano_elem->card_.card_number_) + "</" + field + ">\n";
    writeString(tmp, level);

    writeCardinality(ano_elem, level);
  }

  void RuleOwlWriter::writeCardinalityRange(AnonymousClassElement* ano_elem, size_t level, bool is_data_prop)
  {
    std::string tmp, field;

    switch(ano_elem->card_.card_type_)
    {
    case cardinality_value:
      field = "owl:hasValue";
      if(is_data_prop)
      {
        tmp += "<" + field + " rdf:datatype=\"";
        tmp += ano_elem->card_.card_value_range_->type_->getNamespace() + "#" + ano_elem->card_.card_value_range_->type_->value() + "\">" + ano_elem->card_.card_value_range_->data();
        tmp += "</" + field + ">\n";
      }
      else
        tmp += "<" + field + " " + getRdfResource(ano_elem->individual_involved_->value()) + "/>\n";
      writeString(tmp, level);
      return;
    case cardinality_only:
      field = "owl:allValuesFrom";
      break;
    case cardinality_some:
      field = "owl:someValuesFrom";
      break;
    case cardinality_error:
      Display::error("cardinality type error");
      break;
    default:
      Display::error("cardinality type " + std::to_string(ano_elem->card_.card_type_) + " not supported by this function");
      break;
    }

    if(ano_elem->is_complex == false)
    {
      tmp = "<" + field + " " + getResource(ano_elem) + "/>\n";
      writeString(tmp, level);
    }
    else
    {
      writeString("<" + field + ">\n", level);

      if(is_data_prop == true)
        writeDatatypeExpression(ano_elem->sub_elements_.front(), level + 1);
      else
        writeComplexDescription(ano_elem->sub_elements_.front(), level + 1);

      writeString("</" + field + ">\n", level);
    }
  }

  void RuleOwlWriter::writeCardinality(AnonymousClassElement* ano_element, size_t level)
  {
    std::string tmp;

    if(ano_element->data_property_involved_ == nullptr)
    {
      std::string field = "owl:onClass";

      if(ano_element->class_involved_ != nullptr)
      {
        tmp = "<" + field + " " + getResource(ano_element) + "/>\n";
        writeString(tmp, level);
      }
      else
      {
        writeString("<" + field + ">\n", level);

        writeClassExpression(ano_element->sub_elements_.front(), level + 1);

        writeString("</" + field + ">\n", level);
      }
    }
    else
    {
      std::string field = "owl:onDataRange";

      if(ano_element->card_.card_value_range_ != nullptr)
      {
        tmp = "<" + field + " " + getResource(ano_element) + "/>\n";
        writeString(tmp, level);
      }
      else
      {
        writeString("<" + field + ">\n", level);

        writeDatatypeExpression(ano_element->sub_elements_.front(), level + 1);

        writeString("</" + field + ">\n", level);
      }
    }
  }

  std::string RuleOwlWriter::getResource(AnonymousClassElement* ano_elem, const std::string& attribute_name, bool used_property)
  {
    if(used_property == true)
    {
      if(ano_elem->object_property_involved_ != nullptr)
        return attribute_name + "=\"" + ns_ + "#" + ano_elem->object_property_involved_->value() + "\"";
      else if(ano_elem->data_property_involved_ != nullptr)
        return attribute_name + "=\"" + ns_ + "#" + ano_elem->data_property_involved_->value() + "\"";
      else
        return "";
    }
    else if(ano_elem->class_involved_ != nullptr)
      return attribute_name + "=\"" + ns_ + "#" + ano_elem->class_involved_->value() + "\"";

    else if(ano_elem->card_.card_type_range_ != nullptr)
      return attribute_name + "=\"" + ano_elem->card_.card_type_range_->getNamespace() + "#" + ano_elem->card_.card_type_range_->value() + "\"";

    else if(ano_elem->individual_involved_ != nullptr)
      return attribute_name + "=\"" + ns_ + "#" + ano_elem->individual_involved_->value() + "\"";
    else
      return "";
  }

} // namespace ontologenius