#ifndef ONTOLOGENIUS_ONTOLOGYREADER_H
#define ONTOLOGENIUS_ONTOLOGYREADER_H

#include <iostream>
#include <string>
#include <vector>

#include "ontologenius/core/ontoGraphs/Branchs/Elements.h"
#include "ontologenius/core/ontoGraphs/Graphs/OntologyGraphs.h"

namespace ontologenius {

  class Ontology;

  class OntologyReader
  {
  public:
    explicit OntologyReader(OntologyGraphs* graphs) : graphs_(graphs) {}
    ~OntologyReader() = default;

    void setDisplay(bool display) { display_ = display; }
    int getNbLoadedElements() const { return nb_loaded_elem_; }
    bool empty() const { return (nb_loaded_elem_ == 0); }

  protected:
    OntologyGraphs* graphs_;

    int nb_loaded_elem_;
    bool display_;

    void push(std::vector<std::string>& vect, const std::string& elem, const std::string& symbole) const
    {
      vect.emplace_back(elem);
      if(display_ && symbole.empty() == false)
        std::cout << "│   │   ├── " << symbole << elem << std::endl;
    }

    void push(std::vector<bool>& vect, bool elem, const std::string& symbole) const
    {
      vect.push_back(elem);
      if(display_ && symbole.empty() == false)
      {
        if(elem == true)
          std::cout << "│   │   ├── " << symbole << " true" << std::endl;
        else
          std::cout << "│   │   ├── " << symbole << " false" << std::endl;
      }
    }

    void push(std::vector<PairElement<std::string, std::string>>& vect, const PairElement<std::string, std::string>& elem, const std::string& symbole1, const std::string& symbole2) const
    {
      vect.emplace_back(elem);
      if(display_)
      {
        if(symbole1.empty() == false)
          std::cout << "│   │   ├── " << symbole1 << elem.first << std::endl;

        if(symbole2.empty() == false)
          std::cout << "│   │   ├── " << symbole2 << elem.second << std::endl;
      }
    }
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_ONTOLOGYREADER_H