#include "ontologenius/core/ontologyIO/Owl/writers/ObjectPropertiesOwlWriter.h"

#include <cstdio>
#include <shared_mutex>
#include <string>
#include <vector>

#include "ontologenius/core/ontoGraphs/Graphs/ObjectPropertyGraph.h"

namespace ontologenius {

  ObjectPropertiesOwlWriter::ObjectPropertiesOwlWriter(ObjectPropertyGraph* property_graph,
                                                       FILE* file,
                                                       const std::string& ns) : PropertiesOwlWriter(file, ns, "owl:ObjectProperty"),
                                                                                property_graph_(property_graph)
  {}

  void ObjectPropertiesOwlWriter::write()
  {
    const std::shared_lock<std::shared_timed_mutex> lock(property_graph_->mutex_);

    const std::vector<ObjectPropertyBranch*> properties = property_graph_->get();
    for(auto* property : properties)
      writeProperty(property);
  }

  void ObjectPropertiesOwlWriter::writeProperty(ObjectPropertyBranch* branch)
  {
    writeBranchStart(branch->value());

    for(auto& mother : branch->mothers_)
      writeSingleResource("rdfs:subPropertyOf", mother);

    writeDisjointWith(branch);

    for(auto& inverse : branch->inverses_)
      writeSingleResource("owl:inverseOf", inverse);

    writeProperties(branch);

    for(auto& range : branch->ranges_)
      writeSingleResource("rdfs:range", range);

    for(auto& domain : branch->domains_)
      writeSingleResource("rdfs:domain", domain);

    writeChain(branch);

    writeDictionary(branch);
    writeMutedDictionary(branch);

    writeBranchEnd();
  }

  void ObjectPropertiesOwlWriter::writeChain(ObjectPropertyBranch* branch)
  {
    for(auto& chain : branch->str_chains_)
    {
      std::string tmp = "        <owl:propertyChainAxiom rdf:parseType=\"Collection\">\n";

      for(auto& link : chain)
      {
        tmp += "            <rdf:Description rdf:about=\"" + ns_ + "#" +
               link +
               "\"/>\n";
      }

      tmp += "        </owl:propertyChainAxiom>\n";
      writeString(tmp);
    }
  }

} // namespace ontologenius
