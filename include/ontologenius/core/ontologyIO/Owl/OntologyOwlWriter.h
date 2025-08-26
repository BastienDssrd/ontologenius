#ifndef ONTOLOGENIUS_ONTOLOGYOWLWRITER_H
#define ONTOLOGENIUS_ONTOLOGYOWLWRITER_H

#include <cstdio>
#include <string>

#include "ontologenius/core/ontoGraphs/Graphs/OntologyGraphs.h"

namespace ontologenius {

  class Ontology;

  class OntologyOwlWriter
  {
  public:
    explicit OntologyOwlWriter(OntologyGraphs* graphs);
    ~OntologyOwlWriter() = default;

    void setFileName(const std::string& name) { file_name_ = name; }
    std::string getFileName() const { return file_name_; }
    void write(const std::string& file_name = "none");

  private:
    OntologyGraphs* graphs_;
    std::string ns_;

    std::string file_name_;
    FILE* file_;

    void writeStart();
    void writeEnd();
    void writeBanner(const std::string& name);
    void writeString(const std::string& text);
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_ONTOLOGYOWLWRITER_H
