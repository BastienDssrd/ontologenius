#include "ontologenius/core/ontologyIO/Owl/writers/ClassOwlWriter.h"

#include <algorithm>
#include <cstdio>
#include <iterator>
#include <set>
#include <shared_mutex>
#include <string>
#include <vector>

#include "ontologenius/core/ontoGraphs/Branchs/AnonymousClassBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/ClassBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/DataPropertyBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/IndividualBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/ObjectPropertyBranch.h"
#include "ontologenius/core/ontoGraphs/Graphs/AnonymousClassGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/ClassGraph.h"
#include "ontologenius/graphical/Display.h"

namespace ontologenius {

  ClassOwlWriter::ClassOwlWriter(ClassGraph* class_graph, const std::string& ns) : GraphOwlWriter(ns, "owl:Class"),
                                                                                   class_graph_(class_graph)
  {}

  void ClassOwlWriter::write(FILE* file)
  {
    file_ = file;

    const std::shared_lock<std::shared_timed_mutex> lock(class_graph_->mutex_);

    const std::vector<ClassBranch*> classes = class_graph_->get();

    for(auto* classe : classes)
      writeClass(classe);

    file_ = nullptr;
  }

  void ClassOwlWriter::writeGeneralAxioms(FILE* file)
  {
    file_ = file;

    const std::shared_lock<std::shared_timed_mutex> lock(class_graph_->mutex_);

    std::vector<ClassBranch*> classes = class_graph_->get();
    writeDisjointWith(classes);

    file_ = nullptr;
  }

  void ClassOwlWriter::writeClass(ClassBranch* branch)
  {
    writeBranchStart(branch->value());

    writeEquivalentClass(branch);
    writeSubClassOf(branch);

    writeDisjointWith(branch);

    writeObjectProperties(branch);
    writeDataProperties(branch);

    writeDictionary(branch);
    writeMutedDictionary(branch);

    writeBranchEnd();
  }

  void ClassOwlWriter::writeEquivalentClass(ClassBranch* branch)
  {
    AnonymousClassBranch* equiv = branch->equiv_anonymous_class_;

    if(equiv != nullptr)
    {
      for(auto* tree : equiv->ano_trees_)
      {
        std::string tmp, field;
        field = "owl:equivalentClass";
        const size_t level = 2;

        auto* tree_root_node = tree->root_node_;

        // single expression
        if(tree_root_node->sub_elements_.empty() &&
           tree_root_node->class_involved_ != nullptr &&
           tree_root_node->object_property_involved_ == nullptr)
        {
          tmp = "<" + field + " " + getResource(tree_root_node) + "/>\n";
          writeString(tmp, level);
        }
        // Collection of expressions
        else
        {
          tmp = "<" + field + ">\n";
          writeString(tmp, level);

          if(tree_root_node->logical_type_ != logical_none || tree_root_node->oneof == true)
            writeClassExpression(tree_root_node, level + 1);
          else
            writeRestriction(tree_root_node, level + 1);

          tmp = "</" + field + ">\n";
          writeString(tmp, level);
        }
      }
    }
  }

  void ClassOwlWriter::writeRestriction(AnonymousClassElement* ano_elem, size_t level)
  {
    const std::string field = "owl:Restriction";

    writeString("<" + field + ">\n", level);
    writeString("<owl:onProperty " + getResource(ano_elem, "rdf:resource", true) + "/>\n", level + 1);

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

  void ClassOwlWriter::writeClassExpression(AnonymousClassElement* ano_elem, size_t level)
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

  void ClassOwlWriter::writeDatatypeExpression(AnonymousClassElement* ano_elem, size_t level)
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

  void ClassOwlWriter::writeIntersection(AnonymousClassElement* ano_elem, size_t level, bool is_data_prop)
  {
    const std::string field = "owl:intersectionOf"; 

    writeString("<" + field + " " + "rdf:parseType=\"Collection\">\n", level);

    for(auto* child : ano_elem->sub_elements_)
      writeComplexDescription(child, level + 1, is_data_prop);

    writeString("</" + field + ">\n", level);
  }

  void ClassOwlWriter::writeUnion(AnonymousClassElement* ano_elem, size_t level, bool is_data_prop)
  {
    const std::string field = "owl:unionOf";

    writeString("<" + field + " " + "rdf:parseType=\"Collection\">\n", level);

    for(auto* child : ano_elem->sub_elements_)
      writeComplexDescription(child, level + 1, is_data_prop);

    writeString("</" + field + ">\n", level);
  }

  void ClassOwlWriter::writeOneOf(AnonymousClassElement* ano_elem, size_t level)
  {
    const std::string field = "owl:oneOf";

    writeString("<" + field + " " + "rdf:parseType=\"Collection\">\n", level);

    for(auto* child : ano_elem->sub_elements_)
      writeString("<rdf:Description " + getResource(child, "rdf:about") + "/>\n", level + 1);

    writeString("</" + field + ">\n", level);
  }

  void ClassOwlWriter::writeComplement(AnonymousClassElement* ano_elem, size_t level)
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

  void ClassOwlWriter::writeDataComplement(AnonymousClassElement* ano_elem, size_t level)
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

  void ClassOwlWriter::writeComplexDescription(AnonymousClassElement* ano_elem, size_t level, bool is_data_prop)
  {
    if(ano_elem->sub_elements_.empty())
    {
      if(ano_elem->object_property_involved_ == nullptr && ano_elem->data_property_involved_ == nullptr)
        writeString("<rdf:Description " + getResource(ano_elem, "rdf:about") + "/>\n", level);
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

  void ClassOwlWriter::writeCardinalityValue(AnonymousClassElement* ano_elem, size_t level)
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

    writeString("<" + field + " rdf:datatype=\"http://www.w3.org/2001/XMLSchema#nonNegativeInteger\">" + std::to_string(ano_elem->card_.card_number_) + "</" + field + ">\n", level);

    writeCardinality(ano_elem, level);
  }

  void ClassOwlWriter::writeCardinalityRange(AnonymousClassElement* ano_elem, size_t level, bool is_data_prop)
  {
    std::string tmp, field;

    switch(ano_elem->card_.card_type_)
    {
    case cardinality_value:
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

  void ClassOwlWriter::writeCardinality(AnonymousClassElement* ano_element, size_t level)
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

  std::string ClassOwlWriter::getResource(AnonymousClassElement* ano_elem, const std::string& attribute_name, bool used_property)
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

  void ClassOwlWriter::writeSubClassOf(ClassBranch* branch)
  {
    for(auto& mother : branch->mothers_)
      writeSingleResource("rdfs:subClassOf", mother);
  }

  void ClassOwlWriter::writeDisjointWith(ClassBranch* branch)
  {
    if(branch->disjoints_.size() < 2)
      for(auto& disjoint : branch->disjoints_)
        writeSingleResource("owl:disjointWith", disjoint);
  }

  void ClassOwlWriter::writeDisjointWith(std::vector<ClassBranch*>& classes)
  {
    const std::string start = "    <rdf:Description>\n\
        <rdf:type rdf:resource=\"http://www.w3.org/2002/07/owl#AllDisjointClasses\"/>\n";

    const std::string end = "    </rdf:Description>\n";

    std::set<std::set<ClassBranch*>> disjoints_vects;

    for(auto& classe : classes)
      if(classe->disjoints_.size() > 1)
        getDisjointsSets(classe, disjoints_vects);

    for(const auto& disjoints_set : disjoints_vects)
    {
      std::string tmp;
      tmp += "        <owl:members rdf:parseType=\"Collection\">\n";

      for(const auto& disj : disjoints_set)
      {
        tmp += "             <rdf:Description rdf:about=\"" + ns_ + "#" +
               disj->value() +
               "\"/>\n";
      }

      tmp += "        </owl:members>\n";
      if(disjoints_set.empty() == false)
      {
        writeString(start);
        writeString(tmp);
        writeString(end);
      }
    }
  }

  void ClassOwlWriter::getDisjointsSets(ClassBranch* base, std::set<std::set<ClassBranch*>>& res)
  {
    std::set<ClassBranch*> restriction_set;

    for(auto& disjoint : base->disjoints_)
      restriction_set.insert(disjoint.elem);
    restriction_set.insert(base);

    for(auto& disjoint : base->disjoints_)
    {
      std::set<ClassBranch*> base_set;
      base_set.insert(base);
      base_set.insert(disjoint.elem);
      getDisjointsSets(disjoint.elem, base_set, restriction_set, res);
    }
  }

  void ClassOwlWriter::getDisjointsSets(ClassBranch* last, const std::set<ClassBranch*>& base_set, const std::set<ClassBranch*>& restriction_set, std::set<std::set<ClassBranch*>>& res)
  {
    std::set<ClassBranch*> local_disjoints;
    for(auto& disjoint : last->disjoints_)
      local_disjoints.insert(disjoint.elem);
    std::vector<ClassBranch*> new_restriction_vect;
    std::set_intersection(restriction_set.begin(), restriction_set.end(), local_disjoints.begin(), local_disjoints.end(), std::back_inserter(new_restriction_vect));
    std::set<ClassBranch*> new_restriction_set;
    for(auto& it : new_restriction_vect)
      new_restriction_set.insert(it);

    bool leaf = true;
    for(auto& disjoint : last->disjoints_)
    {
      if(restriction_set.find(disjoint.elem) != restriction_set.end())
      {
        if(base_set.find(disjoint.elem) == base_set.end())
        {
          std::set<ClassBranch*> new_set = base_set;
          new_set.insert(disjoint.elem);
          getDisjointsSets(disjoint.elem, new_set, new_restriction_set, res);
          leaf = false;
        }
      }
    }

    if(leaf)
      res.insert(base_set);
  }

  void ClassOwlWriter::writeObjectProperties(ClassBranch* branch)
  {
    for(const ClassObjectRelationElement& relation : branch->object_relations_)
      if(relation.inferred == false)
        writeString("<" + relation.first->value() + getProba(relation) + " " + getRdfResource(relation.second->value()) + "/>\n", 2);
  }

  void ClassOwlWriter::writeDataProperties(ClassBranch* branch)
  {
    for(const ClassDataRelationElement& relation : branch->data_relations_)
      if(relation.inferred == false)
      {
        const std::string tmp = "<" + relation.first->value() + getProba(relation) + " " + getRdfDatatype(relation.second->type_) + ">" + 
                                relation.second->data() + "</" + relation.first->value() + ">\n";
        writeString(tmp, 2);
      }
  }

} // namespace ontologenius
