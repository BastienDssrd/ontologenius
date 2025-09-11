#ifndef ONTOLOGENIUS_DATAPROPERTIESOWLWRITER_H
#define ONTOLOGENIUS_DATAPROPERTIESOWLWRITER_H

#include <cstdio>
#include <string>

#include "ontologenius/core/ontologyIO/Owl/writers/PropertiesOwlWriter.h"

namespace ontologenius {

  class DataPropertyGraph;
  class DataPropertyBranch;

  class DataPropertiesOwlWriter : public PropertiesOwlWriter<DataPropertyBranch>
  {
  public:
    DataPropertiesOwlWriter(DataPropertyGraph* property_graph, FILE* file, const std::string& ns);
    ~DataPropertiesOwlWriter() = default;

    void write();

  private:
    DataPropertyGraph* property_graph_;

    void writeProperty(DataPropertyBranch* branch);
    void writeRange(DataPropertyBranch* branch);
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_DATAPROPERTIESOWLWRITER_H
