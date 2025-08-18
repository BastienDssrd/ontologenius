#ifndef ONTOLOGENIUS_ANONYMOUSCLASSOWLWRITER_H
#define ONTOLOGENIUS_ANONYMOUSCLASSOWLWRITER_H

#include <cstddef>
#include <cstdio>
#include <set>
#include <string>
#include <vector>

#include "ontologenius/core/ontologyIO/Owl/writers/GraphOwlWriter.h"

namespace ontologenius {

  class ClassExpression;

  class AnonymousClassOwlWriter : protected GraphOwlWriter
  {
  public:
    AnonymousClassOwlWriter(const std::string& key, FILE* file, const std::string& ns) : GraphOwlWriter(file, ns, key) {}
    ~AnonymousClassOwlWriter() = default;

    void writeClassExpression(ClassExpression* ano_elem, size_t level, bool datatype = false);

  private:
    void writeIdentifier(ClassExpression* ano_elem, size_t level);
    void writeRdfResource(ClassExpression* ano_elem, const std::string& key, size_t level);
    void writeCollection(ClassExpression* ano_elem, const std::string& key, size_t level, bool is_data_prop);
    void writeComplement(ClassExpression* ano_elem, size_t level, bool datatype);

    void writeRestriction(ClassExpression* ano_elem, size_t level);
    void writeCardinalityValue(ClassExpression* ano_elem, size_t level);
    void writeQualifier(ClassExpression* ano_element, size_t level);
    void writeCardinalityRange(ClassExpression* ano_elem, size_t level, bool is_data_prop);
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_ANONYMOUSCLASSOWLWRITER_H
