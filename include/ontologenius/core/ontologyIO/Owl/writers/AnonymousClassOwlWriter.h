#ifndef ONTOLOGENIUS_ANONYMOUSCLASSOWLWRITER_H
#define ONTOLOGENIUS_ANONYMOUSCLASSOWLWRITER_H

#include <cstddef>
#include <cstdio>
#include <set>
#include <string>
#include <vector>

#include "ontologenius/core/ontologyIO/Owl/writers/GraphOwlWriter.h"

namespace ontologenius {

  class AnonymousClassElement;

  class AnonymousClassOwlWriter : protected GraphOwlWriter
  {
  public:
    AnonymousClassOwlWriter(const std::string& key, FILE* file, const std::string& ns) : GraphOwlWriter(file, ns, key) {}
    ~AnonymousClassOwlWriter() = default;

    void writeAnonymousClassExpression(AnonymousClassElement* ano_elem, size_t level);

  private:
    void writeRestriction(AnonymousClassElement* ano_elem, size_t level);
    void writeClassExpression(AnonymousClassElement* ano_elem, size_t level);
    void writeDatatypeExpression(AnonymousClassElement* ano_elem, size_t level);
    void writeIntersection(AnonymousClassElement* ano_elem, size_t level, bool is_data_prop = false);
    void writeUnion(AnonymousClassElement* ano_elem, size_t level, bool is_data_prop = false);
    void writeOneOf(AnonymousClassElement* ano_elem, size_t level);
    void writeComplement(AnonymousClassElement* ano_elem, size_t level);
    void writeDataComplement(AnonymousClassElement* ano_elem, size_t level);
    void writeComplexDescription(AnonymousClassElement* ano_elem, size_t level, bool is_data_prop = false);
    void writeCardinalityValue(AnonymousClassElement* ano_elem, size_t level);
    void writeCardinalityRange(AnonymousClassElement* ano_elem, size_t level, bool is_data_prop);
    void writeCardinality(AnonymousClassElement* ano_element, size_t level);

    std::string getResource(AnonymousClassElement* ano_elem, const std::string& attribute_name = "rdf:resource", bool used_property = false);
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_ANONYMOUSCLASSOWLWRITER_H
