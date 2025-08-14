#ifndef ONTOLOGENIUS_CLASSOWLWRITER_H
#define ONTOLOGENIUS_CLASSOWLWRITER_H

#include <cstddef>
#include <cstdio>
#include <set>
#include <string>
#include <vector>

#include "ontologenius/core/ontologyIO/Owl/writers/AnonymousClassOwlWriter.h"

namespace ontologenius {

  class ClassGraph;
  class ClassBranch;
  class AnonymousClassElement;

  class ClassOwlWriter : private AnonymousClassOwlWriter
  {
  public:
    ClassOwlWriter(ClassGraph* class_graph, FILE* file, const std::string& ns);
    ~ClassOwlWriter() = default;

    void write();
    void writeGeneralAxioms();

  private:
    ClassGraph* class_graph_;

    void writeClass(ClassBranch* branch);
    void writeSubClassOf(ClassBranch* branch);

    void writeEquivalentClass(ClassBranch* branch);

    void writeDisjointWith(ClassBranch* branch);
    void writeDisjointWith(std::vector<ClassBranch*>& classes);
    void getDisjointsSets(ClassBranch* base, std::set<std::set<ClassBranch*>>& res);
    void getDisjointsSets(ClassBranch* last, const std::set<ClassBranch*>& base_set, const std::set<ClassBranch*>& restriction_set, std::set<std::set<ClassBranch*>>& res);
    void writeObjectProperties(ClassBranch* branch);
    void writeDataProperties(ClassBranch* branch);
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_CLASSOWLWRITER_H
