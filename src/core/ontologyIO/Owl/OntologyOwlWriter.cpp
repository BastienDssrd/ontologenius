#include "ontologenius/core/ontologyIO/Owl/OntologyOwlWriter.h"

#include <iostream>

#include "ontologenius/core/ontoGraphs/Ontology.h"
#include "ontologenius/core/ontologyIO/Owl/writers/AnnotationOwlWriter.h"
#include "ontologenius/core/ontologyIO/Owl/writers/ClassOwlWriter.h"
#include "ontologenius/core/ontologyIO/Owl/writers/DataPropertiesOwlWriter.h"
#include "ontologenius/core/ontologyIO/Owl/writers/IndividualOwlWriter.h"
#include "ontologenius/core/ontologyIO/Owl/writers/ObjectPropertiesOwlWriter.h"
#include "ontologenius/graphical/Display.h"

namespace ontologenius {

  OntologyOwlWriter::OntologyOwlWriter(ClassGraph* class_graph,
                                       ObjectPropertyGraph* object_property_graph,
                                       DataPropertyGraph* data_property_graph,
                                       IndividualGraph* individual_graph,
                                       AnonymousClassGraph* anonymous_graph) : file_(nullptr)
  {
    class_graph_ = class_graph;
    object_property_graph_ = object_property_graph;
    data_property_graph_ = data_property_graph;
    individual_graph_ = individual_graph;
    anonymous_graph_ = anonymous_graph;
  }

  OntologyOwlWriter::OntologyOwlWriter(Ontology& onto)
  {
    class_graph_ = &onto.class_graph_;
    object_property_graph_ = &onto.object_property_graph_;
    individual_graph_ = &onto.individual_graph_;
    data_property_graph_ = &onto.data_property_graph_;
    anonymous_graph_ = &onto.anonymous_graph_;
  }

  void OntologyOwlWriter::write(const std::string& file_name)
  {
    if(file_name != "none")
      file_name_ = file_name;

    if(file_name_ == "none")
      return;

    file_ = fopen(file_name_.c_str(), "w");
    if(file_ == nullptr)
    {
      Display::error("Fail to open file : ", false);
      std::cout << file_name_ << std::endl;
      return;
    }

    ns_ = "urn:absolute:ontologenius";

    writeStart();

    writeBanner("Annotations Properties");
    AnnotationOwlWriter annotations(object_property_graph_, data_property_graph_, ns_);
    annotations.write(file_);

    writeBanner("Classes");
    ClassOwlWriter classes(class_graph_, ns_);
    classes.write(file_);

    writeBanner("Object Properties");
    ObjectPropertiesOwlWriter object_properties(object_property_graph_, ns_);
    object_properties.write(file_);

    writeBanner("Data properties");
    DataPropertiesOwlWriter data_properties(data_property_graph_, ns_);
    data_properties.write(file_);

    writeBanner("Individuals");
    IndividualOwlWriter individuals(individual_graph_, ns_);
    individuals.write(file_);

    writeBanner("General axioms");
    classes.writeGeneralAxioms(file_);
    individuals.writeGeneralAxioms(file_);

    writeEnd();

    Display::success("ontology loaded in : ", false);
    std::cout << file_name_ << std::endl;

    if(file_ != nullptr)
      fclose(file_);
  }

  void OntologyOwlWriter::writeStart()
  {
    std::string tmp = "<?xml version=\"1.0\"?> \n\
<rdf:RDF xmlns=\"" + ns_ +
                      "#\" \n\
     xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n\
     xmlns:owl=\"http://www.w3.org/2002/07/owl#\"\n\
     xmlns:xml=\"http://www.w3.org/XML/1998/namespace\"\n\
     xmlns:xsd=\"http://www.w3.org/2001/XMLSchema#\"\n\
     xmlns:rdfs=\"http://www.w3.org/2000/01/rdf-schema#\"\n\
     xmlns:onto=\"ontologenius#\">\n\
    <owl:Ontology rdf:about=\"" +
                      ns_ + "\"/>\n\n\n\n";
    writeString(tmp);
  }

  void OntologyOwlWriter::writeEnd()
  {
    std::string tmp = "</rdf:RDF> \n\n\n\n\
<!-- Generated by the ontologenius https://sarthou.github.io/ontologenius/ -->\n";
    writeString(tmp);
  }

  void OntologyOwlWriter::writeBanner(const std::string& name)
  {
    std::string tmp = "    <!--\n\
    ///////////////////////////////////////////////////////////////////////////////////////\n\
    //\n\
    // " + name + "\n\
    //\n\
    ///////////////////////////////////////////////////////////////////////////////////////\n\
     -->\n\n\n\n\n";
    writeString(tmp);
  }

  void OntologyOwlWriter::writeString(const std::string& text)
  {
    if(file_ != nullptr)
      fwrite(text.c_str(), sizeof(char), text.size(), file_);
  }

} // namespace ontologenius
