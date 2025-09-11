#include "ontologenius/core/ontoGraphs/Graphs/OntologyGraphs.h"

#include "ontologenius/core/ontoGraphs/Graphs/AnonymousClassGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/ClassGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/DataPropertyGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/IndividualGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/LiteralGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/ObjectPropertyGraph.h"
#include "ontologenius/core/ontoGraphs/Graphs/RuleGraph.h"

namespace ontologenius {

  OntologyGraphs::OntologyGraphs() : anonymous_classes_(this),
                                     classes_(this),
                                     data_properties_(this),
                                     individuals_(this),
                                     object_properties_(this),
                                     rules_(this)
  {}

  OntologyGraphs::OntologyGraphs(const OntologyGraphs& other) : anonymous_classes_(other.anonymous_classes_, this),
                                                                classes_(other.classes_, this),
                                                                data_properties_(other.data_properties_, this),
                                                                individuals_(other.individuals_, this),
                                                                literals_(other.literals_),
                                                                object_properties_(other.object_properties_, this),
                                                                rules_(other.rules_, this)
  {
    literals_.deepCopy(other.literals_);
    classes_.deepCopy(other.classes_);
    object_properties_.deepCopy(other.object_properties_);
    data_properties_.deepCopy(other.data_properties_);
    individuals_.deepCopy(other.individuals_);

    anonymous_classes_.deepCopy(other.anonymous_classes_);
    rules_.deepCopy(other.rules_);
  }

} // namespace ontologenius