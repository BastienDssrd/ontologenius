#include "ontoloGenius/TreeObject.h"
#include <iostream>

using namespace std;

TreeObject::~TreeObject()
{
  for(unsigned int i = 0; i < branchs_.size(); i++)
    delete branchs_[i];

  for(unsigned int i = 0; i < roots_.size(); i++)
    delete roots_[i];

  branchs_.clear();
  roots_.clear();

  std::cout << "TreeObject" << std::endl;
}

void TreeObject::add(string value, ObjectVectors_t& object_vector)
{
  //am I a created mother ?
  Branch_t* me = nullptr;
  for(unsigned int i = 0; i < tmp_mothers_.size(); i++)
  {
    if(tmp_mothers_[i]->value_ == value)
    {
      me = tmp_mothers_[i];
      tmp_mothers_.erase(tmp_mothers_.begin() + i);
      break;
    }
  }

  //am I created ?
  if(me == nullptr)
    me = new struct Branch_t(value);

  me->nb_mothers_ = object_vector.mothers_.size();

  //am I a root ?
  if(me->nb_mothers_ == 0)
    roots_.push_back(me);
  else
  {
    //for all my mothers
    for(unsigned int mothers_i = 0; mothers_i < object_vector.mothers_.size(); mothers_i++)
    {
      bool i_find_my_mother = false;

      //is a root my mother ?
      for(unsigned int root_i = 0; root_i < roots_.size(); root_i++)
        if(object_vector.mothers_[mothers_i] == roots_[root_i]->value_)
        {
          roots_[root_i]->childs_.push_back(me);
          me->mothers_.push_back(roots_[root_i]);
          i_find_my_mother = true;
        }

      //is a branch my mother ?
      for(unsigned int branch_i = 0; branch_i < branchs_.size(); branch_i++)
        if(object_vector.mothers_[mothers_i] == branchs_[branch_i]->value_)
        {
          branchs_[branch_i]->childs_.push_back(me);
          me->mothers_.push_back(branchs_[branch_i]);
          i_find_my_mother = true;
        }

      //is a tmp mother is mine ?
      for(unsigned int branch_i = 0; branch_i < tmp_mothers_.size(); branch_i++)
        if(object_vector.mothers_[mothers_i] == tmp_mothers_[branch_i]->value_)
        {
          tmp_mothers_[branch_i]->childs_.push_back(me);
          me->mothers_.push_back(tmp_mothers_[branch_i]);
          i_find_my_mother = true;
        }

      //I create my mother
      if(!i_find_my_mother)
      {
        Branch_t* my_mother = new struct Branch_t(object_vector.mothers_[mothers_i]);
        my_mother->childs_.push_back(me);
        me->mothers_.push_back(my_mother);
        tmp_mothers_.push_back(my_mother);
      }
    }

    //but i am also a branch
    branchs_.push_back(me);
  }

  //for all my disjoints
  for(unsigned int disjoints_i = 0; disjoints_i < object_vector.disjoints_.size(); disjoints_i++)
  {
    bool i_find_my_disjoint = false;

    //is a root my disjoint ?
    for(unsigned int root_i = 0; root_i < roots_.size(); root_i++)
      if(object_vector.disjoints_[disjoints_i] == roots_[root_i]->value_)
      {
        me->disjoints_.push_back(roots_[root_i]);
        roots_[root_i]->disjoints_.push_back(me);
        i_find_my_disjoint = true;
      }

    //is a branch my disjoint ?
    for(unsigned int branch_i = 0; branch_i < branchs_.size(); branch_i++)
      if(object_vector.disjoints_[disjoints_i] == branchs_[branch_i]->value_)
      {
        me->disjoints_.push_back(branchs_[branch_i]);
        branchs_[branch_i]->disjoints_.push_back(me);
        i_find_my_disjoint = true;
      }

    //is a tmp mother is my disjoint ?
    for(unsigned int branch_i  =0; branch_i < tmp_mothers_.size(); branch_i++)
      if(object_vector.disjoints_[disjoints_i] == tmp_mothers_[branch_i]->value_)
      {
        me->disjoints_.push_back(tmp_mothers_[branch_i]);
        tmp_mothers_[branch_i]->disjoints_.push_back(me);
        i_find_my_disjoint = true;
      }

    //I create my disjoint
    if(!i_find_my_disjoint)
    {
      Branch_t* my_disjoint = new struct Branch_t(object_vector.disjoints_[disjoints_i]);
      me->disjoints_.push_back(my_disjoint);
      my_disjoint->disjoints_.push_back(me);
      tmp_mothers_.push_back(my_disjoint); //I put my disjoint as tmp_mother
    }
  }

  me->dictionary_ = object_vector.dictionary_;
  if(me->dictionary_.find("en") == me->dictionary_.end())
    me->dictionary_["en"] = me->value_;
}

void TreeObject::add(vector<string>& disjoints)
{
  for(unsigned int disjoints_i = 0; disjoints_i < disjoints.size(); disjoints_i++)
  {
    //I need to find myself
    Branch_t* me = nullptr;
    //Am I a root ?
    for(unsigned int root_i = 0; root_i < roots_.size(); root_i++)
      if(disjoints[disjoints_i] == roots_[root_i]->value_)
        me = roots_[root_i];

    //Am I a branch ?
    if(me == nullptr)
      for(unsigned int branch_i = 0; branch_i < branchs_.size(); branch_i++)
        if(disjoints[disjoints_i] == branchs_[branch_i]->value_)
          me = branchs_[branch_i];

    //Am I a tmp_mother ?
    if(me == nullptr)
      for(unsigned int branch_i  =0; branch_i < tmp_mothers_.size(); branch_i++)
        if(disjoints[disjoints_i] == tmp_mothers_[branch_i]->value_)
          me = tmp_mothers_[branch_i];

    // I don't exist ? so I will be a tmp_mother
    if(me == nullptr)
    {
      me = new struct Branch_t(disjoints[disjoints_i]);
      tmp_mothers_.push_back(me);
    }

    //for all my disjoints ...
    for(unsigned int disjoints_j = 0; disjoints_j < disjoints.size(); disjoints_j++)
    {
      //... excepted me
      if(disjoints_i != disjoints_j)
      {
        bool i_find_my_disjoint = false;

        //is a root my disjoint ?
        for(unsigned int root_i = 0; root_i < roots_.size(); root_i++)
          if(disjoints[disjoints_j] == roots_[root_i]->value_)
          {
            me->disjoints_.push_back(roots_[root_i]);
            i_find_my_disjoint = true;
          }

        //is a branch my disjoint ?
        for(unsigned int branch_i = 0; branch_i < branchs_.size(); branch_i++)
          if(disjoints[disjoints_j] == branchs_[branch_i]->value_)
          {
            me->disjoints_.push_back(branchs_[branch_i]);
            i_find_my_disjoint = true;
          }

        //is a tmp mother is my disjoint ?
        for(unsigned int branch_i  =0; branch_i < tmp_mothers_.size(); branch_i++)
          if(disjoints[disjoints_j] == tmp_mothers_[branch_i]->value_)
          {
            me->disjoints_.push_back(tmp_mothers_[branch_i]);
            i_find_my_disjoint = true;
          }

        //I create my disjoint
        if(!i_find_my_disjoint)
        {
          Branch_t* my_disjoint = new struct Branch_t(disjoints[disjoints_j]);
          me->disjoints_.push_back(my_disjoint);
          tmp_mothers_.push_back(my_disjoint); //I put my disjoint as tmp_mother
        }
      }
    }
  }
}

void TreeObject::close()
{
  for(unsigned int i = 0; i < tmp_mothers_.size(); i++)
    roots_.push_back(tmp_mothers_[i]);

  tmp_mothers_.clear();

  link();
}

set<string> TreeObject::getDown(string value)
{
  set<string> res;

  for(unsigned int i = 0; i < roots_.size(); i++)
    if(roots_[i]->value_ == value)
    {
      set<string> tmp = getDown(roots_[i], value);

      if(tmp.size())
      {
        res.insert(tmp.begin(), tmp.end());
        break;
      }
    }

  if(!res.size())
    for(unsigned int i = 0; i < branchs_.size(); i++)
      if(branchs_[i]->value_ == value)
      {
        set<string> tmp = getDown(branchs_[i], value);

        if(tmp.size())
        {
          res.insert(tmp.begin(), tmp.end());
          break;
        }
      }

  return res;
}

set<string> TreeObject::getUp(string value)
{
  set<string> res;

  for(unsigned int i = 0; i < roots_.size(); i++)
    if(roots_[i]->value_ == value)
    {
      set<string> tmp = getUp(roots_[i], value);

      if(tmp.size())
      {
        res.insert(tmp.begin(), tmp.end());
        break;
      }
    }

  if(!res.size())
    for(unsigned int i = 0; i < branchs_.size(); i++)
      if(branchs_[i]->value_ == value)
      {
        set<string> tmp = getUp(branchs_[i], value);

        if(tmp.size())
        {
          res.insert(tmp.begin(), tmp.end());
          break;
        }
      }

  return res;
}

set<string> TreeObject::getDisjoint(string value)
{
  set<string> res;

  for(unsigned int i = 0; i < roots_.size(); i++)
    if(roots_[i]->value_ == value)
    {
      cout << roots_[i]->value_ << endl;
      for(unsigned disjoint_i = 0; disjoint_i < roots_[i]->disjoints_.size(); disjoint_i++)
      {
        cout << "------" << roots_[i]->disjoints_[disjoint_i]->value_ << endl;
        set<string> tmp = getDown(roots_[i]->disjoints_[disjoint_i], value);

        if(tmp.size())
        {
          res.insert(tmp.begin(), tmp.end());
          break;
        }
      }
    }

  if(!res.size())
    for(unsigned int i = 0; i < branchs_.size(); i++)
      if(branchs_[i]->value_ == value)
      {
        cout << branchs_[i]->value_ << endl;
        for(unsigned disjoint_i = 0; disjoint_i < branchs_[i]->disjoints_.size(); disjoint_i++)
        {
          cout << "------" << branchs_[i]->disjoints_[disjoint_i]->value_ << endl;
          set<string> tmp = getDown(branchs_[i]->disjoints_[disjoint_i], value);

          if(tmp.size())
            res.insert(tmp.begin(), tmp.end());
        }
        break;
      }

  return res;
}

void TreeObject::link()
{
  depth_ = 0;

  uint8_t nb_root_family = roots_.size();
  for(uint8_t root_i = 0; root_i < roots_.size(); root_i++)
  {
    roots_[root_i]->family = 256/(nb_root_family+1) * root_i;
    for(unsigned int i = 0; i < roots_[root_i]->childs_.size(); i++)
      add_family(roots_[root_i]->childs_[i], roots_[root_i]->family);
  }
}

void TreeObject::add_family(Branch_t* branch, uint8_t family)
{
  branch->family += family/branch->nb_mothers_;
  for(unsigned int i = 0; i < branch->childs_.size(); i++)
  {
    depth_++;
    if(depth_ < 20)
      add_family(branch->childs_[i], family/branch->nb_mothers_);
    depth_--;
  }
}

set<string> TreeObject::getDown(Branch_t* branch, string value)
{
  set<string> res;
  res.insert(branch->value_);
  for(unsigned int i = 0; i < branch->childs_.size(); i++)
  {
    set<string> tmp = getDown(branch->childs_[i], value);

    if(tmp.size())
      res.insert(tmp.begin(), tmp.end());
  }

  return res;
}

set<string> TreeObject::getUp(Branch_t* branch, string value)
{
  set<string> res;
  res.insert(branch->value_);
  for(unsigned int i = 0; i < branch->mothers_.size(); i++)
  {
    set<string> tmp = getUp(branch->mothers_[i], value);

    if(tmp.size())
      res.insert(tmp.begin(), tmp.end());
  }

  return res;
}
