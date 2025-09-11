#ifndef ONTOLOGENIUS_ANNOTATIONOWLWRITER_H
#define ONTOLOGENIUS_ANNOTATIONOWLWRITER_H

#include <string>
#include <vector>

#include "ontologenius/core/ontoGraphs/Graphs/DataPropertyGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/ObjectPropertyGraph.h"
#include "ontologenius/core/ontologyIO/Owl/writers/GraphOwlWriter.h"

namespace ontologenius {

  class AnnotationOwlWriter : private GraphOwlWriter
  {
  public:
    AnnotationOwlWriter(ObjectPropertyGraph* object_property_graph,
                        DataPropertyGraph* data_property_graph,
                        FILE* file,
                        const std::string& ns);

    void write();

  private:
    ObjectPropertyGraph* object_property_graph_;
    DataPropertyGraph* data_property_graph_;

    void writeAnnotation(ObjectPropertyBranch* branch);
    void writeAnnotation(DataPropertyBranch* branch);
    void writeRange(const std::vector<LiteralType*>& ranges);
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_ANNOTATIONOWLWRITER_H
