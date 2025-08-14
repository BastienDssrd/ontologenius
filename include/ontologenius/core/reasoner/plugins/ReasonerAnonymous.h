#ifndef ONTOLOGENIUS_REASONERANO_H
#define ONTOLOGENIUS_REASONERANO_H

#include "ontologenius/core/reasoner/plugins/ReasonerInterface.h"

namespace ontologenius {

  class ReasonerAnonymous : public ReasonerInterface
  {
  public:
    ReasonerAnonymous();
    ~ReasonerAnonymous() override = default;

    void postReason() override;
    void setParameter(const std::string& name, const std::string& value) override;

    bool implementPostReasoning() override { return true; }

    std::string getName() override;
    std::string getDescription() override;

    bool defaultActive() override { return true; }

  private:
    bool standard_mode_;

    std::unordered_map<ClassBranch*, std::unordered_set<ClassBranch*>> disjoints_cache_;

    bool checkClassesDisjointess(IndividualBranch* indiv, ClassBranch* class_equiv);

    void addInferredInheritance(IndividualBranch* indiv,
                                AnonymousClassBranch* anonymous_branch,
                                AnonymousClassTree* anonymous_tree,
                                const std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used);

    bool resolveFirstLayer(IndividualBranch* indiv, ClassExpression* ano_elem);
    bool resolveTree(IndividualBranch* indiv, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used);
    bool resolveTree(LiteralNode* literal, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used);

    bool resolveDisjunctionTree(IndividualBranch* indiv, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used);
    bool resolveDisjunctionTreeFirstLayer(IndividualBranch* indiv, ClassExpression* ano_elem);

    bool checkRestrictionFirstLayer(IndividualBranch* indiv, ClassExpression* ano_elem);
    bool checkRestriction(IndividualBranch* indiv, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used);
    bool checkTypeRestriction(IndividualBranch* indiv, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used);
    bool checkTypeRestriction(LiteralNode* literal, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used);
    bool checkValue(IndividualBranch* indiv_from, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used);
    bool checkValue(LiteralNode* literal_from, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used);
    bool checkIndividualRestriction(IndividualBranch* indiv, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used);

    bool checkCard(IndividualBranch* indiv, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used);

    bool checkMinCard(IndividualBranch* indiv, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used);
    bool checkMaxCard(IndividualBranch* indiv, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used);
    bool checkExactlyCard(IndividualBranch* indiv, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used);
    bool checkOnlyCard(IndividualBranch* indiv, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used);
    bool checkSomeCard(IndividualBranch* indiv, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used);
    bool checkValueCard(IndividualBranch* indiv, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used);

    std::string computeDebugUpdate(IndividualBranch* indiv, AnonymousClassTree* anonymous_tree)
    {
      std::string res;
      res = "indiv " + indiv->value() + ": " + indiv->updatesToString();
      res += "with " + anonymous_tree->ano_name + ": " + anonymous_tree->involvesToString();
      if(indiv->flags_.find("equiv") != indiv->flags_.end())
        res += "|equiv: 1";
      else
        res += "|equiv: 0";
      return res;
    }

    template<typename T>
    bool checkPropertyExistence(const std::vector<T>& relations, ClassExpression* ano_elem)
    {
      for(auto& relation : relations)
        if(testBranchInheritanceFirstLayer(ano_elem, relation.first))
          return true;
      return false;
    }

    template<typename T>
    std::vector<std::pair<std::string, size_t>> checkMinCard(const std::vector<T>& relations, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used)
    {
      std::vector<std::pair<std::string, size_t>> indexes;

      for(size_t i = 0; i < relations.size(); i++)
      {
        if(testBranchInheritance(ano_elem, relations[i].first, used))
        {
          if(ano_elem->sub_elements_.empty() == false)
          {
            if(resolveTree(relations[i].second, ano_elem->sub_elements_.front(), used))
            {
              indexes.emplace_back(relations[i].first->value() + "|" + relations[i].second->value(), i);
              if(indexes.size() >= ano_elem->cardinality_value_)
                return indexes;
            }
          }
          else
          {
            if(checkTypeRestriction(relations[i].second, ano_elem, used))
            {
              indexes.emplace_back(relations[i].first->value() + "|" + relations[i].second->value(), i);

              if(indexes.size() >= ano_elem->cardinality_value_)
                return indexes;
            }
          }
        }
      }
      used.clear();
      return {};
    }

    template<typename T>
    std::vector<std::pair<std::string, size_t>> checkMaxCard(const std::vector<T>& relations, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used)
    {
      std::vector<std::pair<std::string, size_t>> indexes;

      for(size_t i = 0; i < relations.size(); i++)
      {
        if(testBranchInheritance(ano_elem, relations[i].first, used))
        {
          if(ano_elem->sub_elements_.empty() == false)
          {
            if(resolveTree(relations[i].second, ano_elem->sub_elements_.front(), used))
            {
              indexes.emplace_back(relations[i].first->value() + "|" + relations[i].second->value(), i);

              if(indexes.size() > ano_elem->cardinality_value_)
              {
                used.clear();
                return {};
              }
            }
          }
          else
          {
            if(checkTypeRestriction(relations[i].second, ano_elem, used))
            {
              indexes.emplace_back(relations[i].first->value() + "|" + relations[i].second->value(), i);

              if(indexes.size() > ano_elem->cardinality_value_)
              {
                used.clear();
                return {};
              }
            }
          }
        }
      }
      return indexes;
    }

    template<typename T>
    std::vector<std::pair<std::string, size_t>> checkExactlyCard(const std::vector<T>& relations, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used)
    {
      std::vector<std::pair<std::string, size_t>> indexes;

      for(size_t i = 0; i < relations.size(); i++)
      {
        if(testBranchInheritance(ano_elem, relations[i].first, used))
        {
          if(ano_elem->sub_elements_.empty() == false)
          {
            if(resolveTree(relations[i].second, ano_elem->sub_elements_.front(), used))
              indexes.emplace_back(relations[i].first->value() + "|" + relations[i].second->value(), i);
          }
          else if(checkTypeRestriction(relations[i].second, ano_elem, used))
            indexes.emplace_back(relations[i].first->value() + "|" + relations[i].second->value(), i);
        }
      }

      if(indexes.size() == ano_elem->cardinality_value_)
        return indexes;
      else
      {
        used.clear();
        return {};
      }
    }

    template<typename T>
    std::vector<std::pair<std::string, size_t>> checkOnlyCard(const std::vector<T>& relations, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used)
    {
      std::vector<std::pair<std::string, size_t>> indexes;
      // need to ensure that there is at least one ?
      for(size_t i = 0; i < relations.size(); i++)
      {
        if(testBranchInheritance(ano_elem, relations[i].first, used))
        {
          if(ano_elem->sub_elements_.empty() == false)
          {
            if(resolveTree(relations[i].second, ano_elem->sub_elements_.front(), used) == false)
            {
              used.clear();
              return {};
            }
            else
              indexes.emplace_back(relations[i].first->value() + "|" + relations[i].second->value(), i);
          }
          else
          {
            if(checkTypeRestriction(relations[i].second, ano_elem, used) == false)
            {
              used.clear();
              return {};
            }
            else
              indexes.emplace_back(relations[i].first->value() + "|" + relations[i].second->value(), i);
          }
        }
      }

      return indexes;
    }

    template<typename T>
    std::pair<std::string, int> checkSomeCard(const std::vector<T>& relations, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used)
    {
      for(size_t i = 0; i < relations.size(); i++)
      {
        if(testBranchInheritance(ano_elem, relations[i].first, used))
        {
          if(ano_elem->sub_elements_.empty() == false)
          {
            if(resolveTree(relations[i].second, ano_elem->sub_elements_.front(), used) == true)
              return std::make_pair(relations[i].first->value() + "|" + relations[i].second->value(), i);
          }
          else if(checkTypeRestriction(relations[i].second, ano_elem, used) == true)
              return std::make_pair(relations[i].first->value() + "|" + relations[i].second->value(), i);
        }
      }
      return std::make_pair("", -1);
    }

    template<typename T>
    std::pair<std::string, int> checkValueCard(const std::vector<T>& relations, ClassExpression* ano_elem, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used)
    {
      for(size_t i = 0; i < relations.size(); i++)
        if(testBranchInheritance(ano_elem, relations[i].first, used))
          if(checkValue(relations[i].second, ano_elem, used))
            return std::make_pair(relations[i].first->value() + "|" + relations[i].second->value(), i);
      
      return std::make_pair("", -1);
    }

    std::unordered_set<ObjectPropertyBranch*> getUpProperty(ObjectPropertyBranch* property)
    {
      return ontology_->object_property_graph_.getUpPtrSafe(property);
    }

    std::unordered_set<DataPropertyBranch*> getUpProperty(DataPropertyBranch* property)
    {
      return ontology_->data_property_graph_.getUpPtrSafe(property);
    }

    template<typename T>
    bool testBranchInheritanceFirstLayer(ClassExpression* ano_elem, T property)
    {
      auto up_vector = getUpProperty(property);
      for(auto up : up_vector)
        if(testProperty(ano_elem, up))
          return true;
      return false;
    }

    template<typename T>
    bool testBranchInheritance(ClassExpression* ano_elem, T* branch, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used)
    {
      if(ano_elem->object_property_involved_ != nullptr)
        return (existInInheritance(branch, ano_elem->object_property_involved_->get(), used));
      else if(ano_elem->data_property_involved_ != nullptr)
        return (existInInheritance(branch, ano_elem->data_property_involved_->get(), used));
      else if(ano_elem->class_involved_ != nullptr)
        return (existInInheritance(branch, ano_elem->class_involved_->get(), used));
      else
        return false;
    }

    template<typename T>
    bool existInInheritance(T* branch, index_t selector, std::vector<std::pair<std::string, InheritedRelationTriplets*>>& used)
    {
      std::string explanation;

      if(branch->get() == selector)
        return true;
      else
      {
        for(size_t i = 0; i < branch->mothers_.size(); i++)
        {
          if(existInInheritance(branch->mothers_[i].elem, selector, used))
          {
            // explanation = branch->value() + "|isA|" + branch->mothers_[i].elem->value(); // todo: why is this commenented ?
            used.emplace_back(explanation, branch->mothers_.has_induced_inheritance_relations[i]);
            return true;
          }
        }
      }
      return false;
    }

    bool testProperty(ClassExpression* ano_elem, ObjectPropertyBranch* property)
    {
      return ano_elem->object_property_involved_ == property;
    }

    bool testProperty(ClassExpression* ano_elem, DataPropertyBranch* property)
    {
      return ano_elem->data_property_involved_ == property;
    }
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_REASONERANO_H
