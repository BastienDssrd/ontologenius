#include "ontologenius/core/ontologyIO/Owl/writers/DataPropertiesOwlWriter.h"

#include <cstdio>
#include <shared_mutex>
#include <string>
#include <vector>

#include "ontologenius/core/ontoGraphs/Graphs/DataPropertyGraph.h"

namespace ontologenius {

  DataPropertiesOwlWriter::DataPropertiesOwlWriter(DataPropertyGraph* property_graph,
                                                   const std::string& ns) : PropertiesOwlWriter(ns, "owl:DatatypeProperty"),
                                                                            property_graph_(property_graph)
  {}

  void DataPropertiesOwlWriter::write(FILE* file)
  {
    file_ = file;

    const std::shared_lock<std::shared_timed_mutex> lock(property_graph_->mutex_);

    const std::vector<DataPropertyBranch*> properties = property_graph_->get();
    for(auto* property : properties)
      writeProperty(property);

    file_ = nullptr;
  }

  void DataPropertiesOwlWriter::writeProperty(DataPropertyBranch* branch)
  {
    writeBranchStart(branch->value());

    for(auto& mother : branch->mothers_)
      writeSingleResource("rdfs:subPropertyOf", mother);

    writeDisjointWith(branch);
    writeProperties(branch);
    writeRange(branch);

    for(auto& domain : branch->domains_)
      writeSingleResource("rdfs:domain", domain);

    writeDictionary(branch);
    writeMutedDictionary(branch);

    writeBranchEnd();
  }

  void DataPropertiesOwlWriter::writeRange(DataPropertyBranch* branch)
  {
    for(auto& range : branch->ranges_)
      writeString("<rdfs:range rdf:resource=\"" + range->getNamespace() +  "#" + range->value() + +"\"/>\n", 2);
  }

} // namespace ontologenius
