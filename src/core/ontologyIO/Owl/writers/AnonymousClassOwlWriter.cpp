#include "ontologenius/core/ontologyIO/Owl/writers/AnonymousClassOwlWriter.h"

#include <cstddef>
#include <iostream>
#include <string>

#include "ontologenius/core/ontoGraphs/Branchs/AnonymousClassBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/IndividualBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/ObjectPropertyBranch.h"
#include "ontologenius/graphical/Display.h"

namespace ontologenius {

  void AnonymousClassOwlWriter::writeClassExpression(ClassExpression* ano_elem, size_t level, bool datatype)
  {
    if(ano_elem->type_ == class_expression_identifier)
      writeIdentifier(ano_elem, level);
    else
    {
      std::string field = datatype ? "rdfs:Datatype" : "owl:Class";
      field = (ano_elem->type_ == class_expression_restriction) ? "owl:Restriction" : field;

      writeString("<" + field + ">\n", level);

      if(ano_elem->type_ == class_expression_restriction)
        writeRestriction(ano_elem, level + 1);
      else if(ano_elem->type_ == class_expression_union_of)
        writeCollection(ano_elem, "owl:unionOf", level + 1, datatype);
      else if(ano_elem->type_ == class_expression_intersection_of)
        writeCollection(ano_elem, "owl:intersectionOf", level + 1, datatype);
      else if(ano_elem->type_ == class_expression_one_of)
      {
        if(datatype)
          writeOneofDatatype(ano_elem, level + 1);
        else
          writeCollection(ano_elem, "owl:oneOf", level + 1, datatype);
      }
      else if(ano_elem->type_ == class_expression_complement_of)
        writeComplement(ano_elem, level + 1, datatype);

      writeString("</" + field + ">\n", level);
    }
  }

  void AnonymousClassOwlWriter::writeIdentifier(ClassExpression* ano_elem, size_t level)
  {
    std::string ns = ns_;
    std::string value;
    if(ano_elem->individual_involved_ != nullptr)
      value = ano_elem->individual_involved_->value();
    else if(ano_elem->class_involved_ != nullptr)
      value = ano_elem->class_involved_->value();
    else if(ano_elem->datatype_involved_ != nullptr)
    {
      ns = ano_elem->datatype_involved_->getNamespace();
      value = ano_elem->datatype_involved_->value();
    }
    else
      std::cout << "trying to write description of unplanned type" << std::endl;

    writeString("<rdf:Description rdf:about=\"" + ns + "#" + value + "\"/>\n", level);
  }

  void AnonymousClassOwlWriter::writeRdfResource(ClassExpression* ano_elem, const std::string& key, size_t level)
  {
    std::string ns = ns_;
    std::string value;
    if(ano_elem->individual_involved_ != nullptr)
      value = ano_elem->individual_involved_->value();
    else if(ano_elem->class_involved_ != nullptr)
      value = ano_elem->class_involved_->value();
    else if(ano_elem->datatype_involved_ != nullptr)
    {
      ns = ano_elem->datatype_involved_->getNamespace();
      value = ano_elem->datatype_involved_->value();
    }
    else if(ano_elem->data_property_involved_ != nullptr)
      value = ano_elem->data_property_involved_->value();
    else if(ano_elem->object_property_involved_ != nullptr)
      value = ano_elem->object_property_involved_->value();
    else
      std::cout << "trying to write rdf resource of unplanned type" << std::endl;

    writeString("<" + key + " rdf:resource=\"" + ns + "#" + value + "\"/>\n", level);
  }

  void AnonymousClassOwlWriter::writeCollection(ClassExpression* ano_elem, const std::string& key, size_t level, bool is_data_prop)
  {
    writeString("<" + key + " " + "rdf:parseType=\"Collection\">\n", level);

    for(auto* child : ano_elem->sub_elements_)
      writeClassExpression(child, level + 1, is_data_prop);

    writeString("</" + key + ">\n", level);
  }

  void AnonymousClassOwlWriter::writeComplement(ClassExpression* ano_elem, size_t level, bool datatype)
  {
    const std::string field = datatype ? "owl:datatypeComplementOf" : "owl:complementOf";

    if(ano_elem->sub_elements_.front()->type_ == class_expression_identifier)
      writeRdfResource(ano_elem->sub_elements_.front(), field, level);
    else
    {
      writeString("<" + field + ">\n", level);
      writeClassExpression(ano_elem->sub_elements_.front(), level + 1, datatype);
      writeString("</" + field + ">\n", level);
    }
  }

  // continue there
  void AnonymousClassOwlWriter::writeRestriction(ClassExpression* ano_elem, size_t level)
  {
    writeRdfResource(ano_elem, "owl:onProperty", level); // error

    if(ano_elem->restriction_type_ == restriction_max_cardinality ||
       ano_elem->restriction_type_ == restriction_min_cardinality ||
       ano_elem->restriction_type_ == restriction_cardinality)
      writeCardinalityValue(ano_elem, level);
    else if(ano_elem->restriction_type_ == restriction_all_values_from ||
            ano_elem->restriction_type_ == restriction_some_values_from ||
            ano_elem->restriction_type_ == restriction_has_value)
      writeCardinalityRange(ano_elem, level, ano_elem->data_property_involved_ != nullptr);
  }

  void AnonymousClassOwlWriter::writeCardinalityValue(ClassExpression* ano_elem, size_t level)
  {
    bool qualified = (ano_elem->sub_elements_.empty() == false) ||
                     (ano_elem->datatype_involved_ != nullptr) ||
                     (ano_elem->class_involved_ != nullptr);
    std::string field;

    switch(ano_elem->restriction_type_)
    {
    case restriction_unknown:
      break;
    case restriction_cardinality:
      field = qualified ? "owl:qualifiedCardinality" : "owl:cardinality";
      break;
    case restriction_min_cardinality:
      field = qualified ? "owl:minQualifiedCardinality" : "owl:minCardinality";
      break;
    case restriction_max_cardinality:
      field = qualified ? "owl:maxQualifiedCardinality" : "owl:maxCardinality";
      break;
    default:
      Display::error("cardinality type " + std::to_string(ano_elem->restriction_type_) + " not supported by this function");
      break;
    }

    writeString("<" + field + " rdf:datatype=\"http://www.w3.org/2001/XMLSchema#nonNegativeInteger\">" + std::to_string(ano_elem->cardinality_value_) + "</" + field + ">\n", level);

    if(qualified)
      writeQualifier(ano_elem->sub_elements_.front(), level);
  }

  void AnonymousClassOwlWriter::writeQualifier(ClassExpression* ano_element, size_t level)
  {
    if(ano_element->data_property_involved_ == nullptr)
    {
      const std::string field = "owl:onClass";
      if(ano_element->class_involved_ != nullptr)
        writeRdfResource(ano_element, field, level);
      else
      {
        writeString("<" + field + ">\n", level);
        writeClassExpression(ano_element, level + 1);
        writeString("</" + field + ">\n", level);
      }
    }
    else
    {
      const std::string field = "owl:onDataRange";
      if(ano_element->literal_involved_ != nullptr)
        writeRdfResource(ano_element, field, level);
      else
      {
        writeString("<" + field + ">\n", level);
        writeClassExpression(ano_element, level + 1, true);
        writeString("</" + field + ">\n", level);
      }
    }
  }

  void AnonymousClassOwlWriter::writeCardinalityRange(ClassExpression* ano_elem, size_t level, bool is_data_prop)
  {
    std::string tmp, field;
    ClassExpression* data_elem = ano_elem->sub_elements_.front();

    switch(ano_elem->restriction_type_)
    {
    case restriction_has_value:
      field = "owl:hasValue";
      if(is_data_prop)
      {
        tmp += "<" + field + " rdf" + getRdfDatatype(data_elem->literal_involved_->type_) + ">" + data_elem->literal_involved_->data();
        tmp += "</" + field + ">\n";
      }
      else
        tmp += "<" + field + " " + getRdfResource(data_elem->individual_involved_->value()) + "/>\n";
      writeString(tmp, level);
      return;
    case restriction_all_values_from:
      field = "owl:allValuesFrom";
      break;
    case restriction_some_values_from:
      field = "owl:someValuesFrom";
      break;
    default:
      Display::error("cardinality type " + std::to_string(ano_elem->restriction_type_) + " not supported by this function");
      break;
    }

    if(data_elem->type_ == ClassExpressionType_e::class_expression_identifier)
      writeRdfResource(data_elem, field, level);
    else
    {
      writeString("<" + field + ">\n", level);
      writeClassExpression(data_elem, level + 1, is_data_prop);
      writeString("</" + field + ">\n", level);
    }
  }

  void AnonymousClassOwlWriter::writeOneofDatatype(ClassExpression* ano_elem, size_t level)
  {
    writeString("<owl:oneOf>\n", level);
    writeDatatypeList(ano_elem, level + 1);
    writeString("</owl:oneOf>\n", level);
  }

  void AnonymousClassOwlWriter::writeDatatypeList(ClassExpression* ano_elem, size_t level, size_t index)
  {
    writeString("<rdf:Description>\n", level);
    writeString("<rdf:type rdf:resource=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#List\"/>\n", level + 1);

    auto* data_elem = ano_elem->sub_elements_.at(index);
    std::string tmp = "<rdf:first rdf" + getRdfDatatype(data_elem->literal_involved_->type_) + ">" + data_elem->literal_involved_->data();
    tmp += "</rdf:first>\n";
    writeString(tmp, level + 1);

    if(index + 1 < ano_elem->sub_elements_.size())
    {
      writeString("<rdf:rest>\n", level + 1);
      writeDatatypeList(ano_elem, level + 2, index + 1);
      writeString("</rdf:rest>\n", level + 1);
    }
    else
      writeString("<rdf:rest rdf:resource=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#nil\"/>\n", level + 1);

    writeString("</rdf:Description>\n", level);
  }

} // namespace ontologenius