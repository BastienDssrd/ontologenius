#ifndef ONTOLOGENIUS_ONTOLOGYGRAPHS_H
#define ONTOLOGENIUS_ONTOLOGYGRAPHS_H

#include "ontologenius/core/ontoGraphs/Graphs/AnonymousClassGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/ClassGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/DataPropertyGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/IndividualGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/LiteralGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/ObjectPropertyGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/RuleGraph.h"

namespace ontologenius {

  class OntologyGraphs
  {
  public:
    OntologyGraphs();
    OntologyGraphs(const OntologyGraphs& other);

    AnonymousClassGraph anonymous_classes_;
    ClassGraph classes_;
    DataPropertyGraph data_properties_;
    IndividualGraph individuals_;
    LiteralGraph literals_;
    ObjectPropertyGraph object_properties_;
    RuleGraph rules_;
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_ONTOLOGYGRAPHS_H