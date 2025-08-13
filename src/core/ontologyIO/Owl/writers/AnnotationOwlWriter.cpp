#include "ontologenius/core/ontologyIO/Owl/writers/AnnotationOwlWriter.h"

#include <cstdio>
#include <shared_mutex>
#include <string>
#include <vector>

#include "ontologenius/core/ontoGraphs/Branchs/DataPropertyBranch.h"
#include "ontologenius/core/ontoGraphs/Branchs/LiteralNode.h"
#include "ontologenius/core/ontoGraphs/Branchs/ObjectPropertyBranch.h"
#include "ontologenius/core/ontoGraphs/Graphs/DataPropertyGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/ObjectPropertyGraph.h"

namespace ontologenius {

  AnnotationOwlWriter::AnnotationOwlWriter(ObjectPropertyGraph* object_property_graph,
                                           DataPropertyGraph* data_property_graph,
                                           FILE* file,
                                           const std::string& ns) : GraphOwlWriter(file, ns, "owl:AnnotationProperty"),
                                                                    object_property_graph_(object_property_graph),
                                                                    data_property_graph_(data_property_graph)
  {}

  void AnnotationOwlWriter::write()
  {
    {
      const std::shared_lock<std::shared_timed_mutex> lock(object_property_graph_->mutex_);

      const std::vector<ObjectPropertyBranch*> properties = object_property_graph_->get();
      for(auto* property : properties)
        if(property->annotation_usage_)
          writeAnnotation(property);
    }

    {
      const std::shared_lock<std::shared_timed_mutex> lock(data_property_graph_->mutex_);

      const std::vector<DataPropertyBranch*> properties = data_property_graph_->get();
      for(auto* property : properties)
        if(property->annotation_usage_)
          writeAnnotation(property);
    }
  }

  void AnnotationOwlWriter::writeAnnotation(ObjectPropertyBranch* branch)
  {
    writeBranchStart(branch->value());

    for(auto& mother : branch->mothers_)
      writeSingleResource("rdfs:subPropertyOf", mother);

    for(auto range : branch->ranges_)
      writeSingleResource("rdfs:range", range);

    for(auto domain : branch->domains_)
      writeSingleResource("rdfs:domain", domain);

    writeBranchEnd();
  }

  void AnnotationOwlWriter::writeAnnotation(DataPropertyBranch* branch)
  {
    writeBranchStart(branch->value());

    for(auto& mother : branch->mothers_)
      writeString("<rdfs:subPropertyOf" + getProba(mother) + " " + getRdfResource(mother.elem->value()) + "/>\n", 2);

    writeRange(branch->ranges_);

    for(auto domain : branch->domains_)
      writeSingleResource("rdfs:domain", domain);

    writeBranchEnd();
  }


  void AnnotationOwlWriter::writeRange(const std::vector<LiteralType*>& ranges)
  {
    for(const auto& range : ranges)
    {
      const std::string tmp = "<rdfs:range rdf:resource=\"" +
                              range->getNamespace() +
                              "#" +
                              range->value() +
                              +"\"/>\n";
      writeString(tmp, 2);
    }
  }

} // namespace ontologenius
