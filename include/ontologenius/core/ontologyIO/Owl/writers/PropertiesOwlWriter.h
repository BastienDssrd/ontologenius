#ifndef ONTOLOGENIUS_PROPERTIESOWLWRITER_H
#define ONTOLOGENIUS_PROPERTIESOWLWRITER_H

#include <string>

#include "ontologenius/core/ontoGraphs/Branchs/Branch.h"
#include "ontologenius/core/ontoGraphs/Branchs/PropertyBranch.h"
#include "ontologenius/core/ontologyIO/Owl/writers/GraphOwlWriter.h"

namespace ontologenius {

  template<typename T>
  class PropertiesOwlWriter : public GraphOwlWriter
  {
  public:
    PropertiesOwlWriter(FILE* file, const std::string& ns, const std::string& key) : GraphOwlWriter(file, ns, key) {}
    ~PropertiesOwlWriter() = default;

  protected:
    void writeDisjointWith(Branch<T>* branch);
    void writeProperties(PropertyBranch<T>* branch);
  };

  template<typename T>
  void PropertiesOwlWriter<T>::writeDisjointWith(Branch<T>* branch)
  {
    for(auto& disjoint : branch->disjoints_)
      if(disjoint.inferred == false)
      {
        std::string tmp = "        <owl:disjointWith" +
                          getProba(disjoint) +
                          " rdf:resource=\"" + ns_ + "#" +
                          disjoint.elem->value() + "\"/>\n\r";
        writeString(tmp);
      }
  }

  template<typename T>
  void PropertiesOwlWriter<T>::writeProperties(PropertyBranch<T>* branch)
  {
    if(branch->properties_.functional_property_ == true)
      writeString("<rdf:type rdf:resource=\"http://www.w3.org/2002/07/owl#FunctionalProperty\"/>\n\r",2);
    if(branch->properties_.inverse_functional_property_ == true)
      writeString("<rdf:type rdf:resource=\"http://www.w3.org/2002/07/owl#InverseFunctionalProperty\"/>\n\r",2);
    if(branch->properties_.transitive_property_ == true)
      writeString("<rdf:type rdf:resource=\"http://www.w3.org/2002/07/owl#TransitiveProperty\"/>\n\r",2);
    if(branch->properties_.symetric_property_ == true)
      writeString("<rdf:type rdf:resource=\"http://www.w3.org/2002/07/owl#SymmetricProperty\"/>\n\r",2);
    if(branch->properties_.antisymetric_property_ == true)
      writeString("<rdf:type rdf:resource=\"http://www.w3.org/2002/07/owl#AsymmetricProperty\"/>\n\r",2);
    if(branch->properties_.reflexive_property_ == true)
      writeString("<rdf:type rdf:resource=\"http://www.w3.org/2002/07/owl#ReflexiveProperty\"/>\n\r",2);
    if(branch->properties_.irreflexive_property_ == true)
      writeString("<rdf:type rdf:resource=\"http://www.w3.org/2002/07/owl#IrreflexiveProperty\"/>\n\r",2);
  }

} // namespace ontologenius

#endif // ONTOLOGENIUS_PROPERTIESOWLWRITER_H
