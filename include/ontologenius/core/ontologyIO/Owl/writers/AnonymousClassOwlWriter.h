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

    void writeAnonymousClassExpression(ClassExpression* ano_elem, size_t level);

  private:
    void writeRestriction(ClassExpression* ano_elem, size_t level);
    void writeClassExpression(ClassExpression* ano_elem, size_t level);
    void writeDatatypeExpression(ClassExpression* ano_elem, size_t level);
    void writeIntersection(ClassExpression* ano_elem, size_t level, bool is_data_prop = false);
    void writeUnion(ClassExpression* ano_elem, size_t level, bool is_data_prop = false);
    void writeOneOf(ClassExpression* ano_elem, size_t level);
    void writeComplement(ClassExpression* ano_elem, size_t level);
    void writeDataComplement(ClassExpression* ano_elem, size_t level);
    void writeComplexDescription(ClassExpression* ano_elem, size_t level, bool is_data_prop = false);
    void writeCardinalityValue(ClassExpression* ano_elem, size_t level);
    void writeCardinalityRange(ClassExpression* ano_elem, size_t level, bool is_data_prop);
    void writeCardinality(ClassExpression* ano_element, size_t level);

    std::string getResource(ClassExpression* ano_elem, const std::string& attribute_name = "rdf:resource", bool used_property = false);
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_ANONYMOUSCLASSOWLWRITER_H
