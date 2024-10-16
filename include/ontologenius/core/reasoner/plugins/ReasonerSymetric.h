#ifndef ONTOLOGENIUS_REASONERSYMETRIC_H
#define ONTOLOGENIUS_REASONERSYMETRIC_H

#include "ontologenius/core/reasoner/plugins/ReasonerInterface.h"

namespace ontologenius {

class ReasonerSymetric : public ReasonerInterface
{
public:
  ReasonerSymetric() {}
  virtual ~ReasonerSymetric() = default;

  virtual void postReason() override;

  virtual bool implementPostReasoning() override { return true; }

  virtual std::string getName() override;
  virtual std::string getDesciption() override;

  virtual bool defaultAvtive() override {return true;}
private:
  bool symetricExist(IndividualBranch_t* indiv_on, ObjectPropertyBranch_t* sym_prop, IndividualBranch_t* sym_indiv);
};

} // namespace ontologenius

#endif // ONTOLOGENIUS_REASONERSYMETRIC_H
