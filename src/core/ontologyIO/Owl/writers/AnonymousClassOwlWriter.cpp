#include "ontologenius/core/ontologyIO/Owl/writers/AnonymousClassOwlWriter.h"

#include <string>
#include <cstddef>

#include "ontologenius/core/ontoGraphs/Branchs/AnonymousClassBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/IndividualBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/ObjectPropertyBranch.h"
#include "ontologenius/graphical/Display.h"

namespace ontologenius {

  void AnonymousClassOwlWriter::writeAnonymousClassExpression(AnonymousClassElement* ano_elem, size_t level)
  {
    if(ano_elem->logical_type_ != logical_none || ano_elem->oneof == true)
      writeClassExpression(ano_elem, level);
    else
      writeRestriction(ano_elem, level);
  }

  // todo: should be re-written

  void AnonymousClassOwlWriter::writeRestriction(AnonymousClassElement* ano_elem, size_t level)
  {
    const std::string field = "owl:Restriction";

    writeString("<" + field + ">\n", level);
    writeString("<owl:onProperty " + getResource(ano_elem, "rdf:resource", true) + "/>\n", level + 1);

    // Cardinality
    if(ano_elem->card_.card_type_ == restriction_max_cardinality ||
       ano_elem->card_.card_type_ == restriction_min_cardinality ||
       ano_elem->card_.card_type_ == restriction_cardinality)
    {
      writeCardinalityValue(ano_elem, level + 1);
    }
    else if(ano_elem->card_.card_type_ == restriction_all_values_from ||
            ano_elem->card_.card_type_ == restriction_some_values_from ||
            ano_elem->card_.card_type_ == restriction_has_value)
    {
      if(ano_elem->data_property_involved_ != nullptr)
        writeCardinalityRange(ano_elem, level + 1, true);
      else
        writeCardinalityRange(ano_elem, level + 1, false);
    }

    writeString("</" + field + ">\n", level);
  }

  void AnonymousClassOwlWriter::writeClassExpression(AnonymousClassElement* ano_elem, size_t level)
  {
    const std::string field = "owl:Class";

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

  void AnonymousClassOwlWriter::writeDatatypeExpression(AnonymousClassElement* ano_elem, size_t level)
  {
    const std::string field = "rdfs:Datatype";

    writeString("<" + field + ">\n", level);

    if(ano_elem->logical_type_ == logical_or)
      writeUnion(ano_elem, level + 1, true);
    else if(ano_elem->logical_type_ == logical_and)
      writeIntersection(ano_elem, level + 1, true);
    else if(ano_elem->logical_type_ == logical_not)
      writeDataComplement(ano_elem, level + 1);

    writeString("</" + field + ">\n", level);
  }

  void AnonymousClassOwlWriter::writeIntersection(AnonymousClassElement* ano_elem, size_t level, bool is_data_prop)
  {
    const std::string field = "owl:intersectionOf"; 

    writeString("<" + field + " " + "rdf:parseType=\"Collection\">\n", level);

    for(auto* child : ano_elem->sub_elements_)
      writeComplexDescription(child, level + 1, is_data_prop);

    writeString("</" + field + ">\n", level);
  }

  void AnonymousClassOwlWriter::writeUnion(AnonymousClassElement* ano_elem, size_t level, bool is_data_prop)
  {
    const std::string field = "owl:unionOf";

    writeString("<" + field + " " + "rdf:parseType=\"Collection\">\n", level);

    for(auto* child : ano_elem->sub_elements_)
      writeComplexDescription(child, level + 1, is_data_prop);

    writeString("</" + field + ">\n", level);
  }

  void AnonymousClassOwlWriter::writeOneOf(AnonymousClassElement* ano_elem, size_t level)
  {
    const std::string field = "owl:oneOf";

    writeString("<" + field + " " + "rdf:parseType=\"Collection\">\n", level);

    for(auto* child : ano_elem->sub_elements_)
      writeString("<rdf:Description " + getResource(child, "rdf:about") + "/>\n", level + 1);

    writeString("</" + field + ">\n", level);
  }

  void AnonymousClassOwlWriter::writeComplement(AnonymousClassElement* ano_elem, size_t level)
  {
    const std::string field = "owl:complementOf";

    if(ano_elem->sub_elements_.front()->class_involved_ != nullptr && ano_elem->object_property_involved_ == nullptr)
      writeString("<" + field + " " + getResource(ano_elem->sub_elements_.front()) + "/>\n", level);
    else
    {
      writeString("<" + field + ">\n", level);
      writeComplexDescription(ano_elem->sub_elements_.front(), level + 1);
      writeString("</" + field + ">\n", level);
    }
  }

  void AnonymousClassOwlWriter::writeDataComplement(AnonymousClassElement* ano_elem, size_t level)
  {
    const std::string field = "owl:datatypeComplementOf";

    if(ano_elem->sub_elements_.front()->card_.card_value_range_ != nullptr)
      writeString("<" + field + " " + getResource(ano_elem->sub_elements_.front()) + "/>\n", level);
    else
    {
      writeString("<" + field + ">\n", level);
      writeDatatypeExpression(ano_elem->sub_elements_.front(), level + 1);
      writeString("</" + field + ">\n", level);
    }
  }

  void AnonymousClassOwlWriter::writeComplexDescription(AnonymousClassElement* ano_elem, size_t level, bool is_data_prop)
  {
    if(ano_elem->sub_elements_.empty())
    {
      if(ano_elem->object_property_involved_ == nullptr && ano_elem->data_property_involved_ == nullptr)
        writeString("<rdf:Description " + getResource(ano_elem, "rdf:about") + "/>\n", level);
      else
        writeRestriction(ano_elem, level);
    }
    else if(ano_elem->card_.card_type_ == restriction_unknown)
    {
      if(is_data_prop)
        writeDatatypeExpression(ano_elem, level);
      else
        writeClassExpression(ano_elem, level);
    }
    else
      writeRestriction(ano_elem, level);
  }

  void AnonymousClassOwlWriter::writeCardinalityValue(AnonymousClassElement* ano_elem, size_t level)
  {
    std::string field;

    switch(ano_elem->card_.card_type_)
    {
    case restriction_unknown:
      break;
    case restriction_cardinality:
      field = "owl:qualifiedCardinality";
      break;
    case restriction_min_cardinality:
      field = "owl:minQualifiedCardinality";
      break;
    case restriction_max_cardinality:
      field = "owl:maxQualifiedCardinality";
      break;
    default:
      Display::error("cardinality type " + std::to_string(ano_elem->card_.card_type_) + " not supported by this function");
      break;
    }

    writeString("<" + field + " rdf:datatype=\"http://www.w3.org/2001/XMLSchema#nonNegativeInteger\">" + std::to_string(ano_elem->card_.card_number_) + "</" + field + ">\n", level);

    writeCardinality(ano_elem, level);
  }

  void AnonymousClassOwlWriter::writeCardinalityRange(AnonymousClassElement* ano_elem, size_t level, bool is_data_prop)
  {
    std::string tmp, field;

    switch(ano_elem->card_.card_type_)
    {
    case restriction_has_value:
      field = "owl:hasValue";
      if(is_data_prop)
      {
        tmp += "<" + field + " rdf" + getRdfDatatype(ano_elem->card_.card_value_range_->type_) + ">" + ano_elem->card_.card_value_range_->data();
        tmp += "</" + field + ">\n";
      }
      else
        tmp += "<" + field + " " + getRdfResource(ano_elem->individual_involved_->value()) + "/>\n";
      writeString(tmp, level);
      return;
    case restriction_all_values_from:
      field = "owl:allValuesFrom";
      break;
    case restriction_some_values_from:
      field = "owl:someValuesFrom";
      break;
    default:
      Display::error("cardinality type " + std::to_string(ano_elem->card_.card_type_) + " not supported by this function");
      break;
    }

    if(ano_elem->is_complex == false)
      writeString("<" + field + " " + getResource(ano_elem) + "/>\n", level);
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

  void AnonymousClassOwlWriter::writeCardinality(AnonymousClassElement* ano_element, size_t level)
  {
    if(ano_element->data_property_involved_ == nullptr)
    {
      const std::string field = "owl:onClass";

      if(ano_element->class_involved_ != nullptr)
        writeString("<" + field + " " + getResource(ano_element) + "/>\n", level);
      else
      {
        writeString("<" + field + ">\n", level);
        writeClassExpression(ano_element->sub_elements_.front(), level + 1);
        writeString("</" + field + ">\n", level);
      }
    }
    else
    {
      const std::string field = "owl:onDataRange";

      if(ano_element->card_.card_value_range_ != nullptr)
        writeString("<" + field + " " + getResource(ano_element) + "/>\n", level);
      else
      {
        writeString("<" + field + ">\n", level);
        writeDatatypeExpression(ano_element->sub_elements_.front(), level + 1);
        writeString("</" + field + ">\n", level);
      }
    }
  }

  std::string AnonymousClassOwlWriter::getResource(AnonymousClassElement* ano_elem, const std::string& attribute_name, bool used_property)
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