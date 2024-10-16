#ifndef ONTOLOGENIUS_GRAPH_H
#define ONTOLOGENIUS_GRAPH_H

#include <exception>
#include <string>
#include <vector>
#include <map>
#include <mutex>  // For std::unique_lock
#include <shared_mutex>

#include "ontologenius/core/ontoGraphs/BranchContainer/BranchContainerMap.h"
#include "ontologenius/core/ontoGraphs/BranchContainer/BranchContainerDyn.h"
#include "ontologenius/core/ontoGraphs/BranchContainer/BranchContainerSet.h"

#include "ontologenius/core/ontoGraphs/Branchs/ValuedNode.h"
#include "ontologenius/core/ontoGraphs/Branchs/Elements.h"

namespace ontologenius {

struct GraphException : public std::exception {
  std::string msg_;
  explicit GraphException(const std::string& msg) : msg_(msg) {}
  const char * what () const throw () {
    return msg_.c_str();
  }
};

template <typename B>
class Graph
{
  static_assert(std::is_base_of<ValuedNode,B>::value, "B must be derived from ValuedNode");
public:
  Graph() : language_("en") {}
  virtual ~Graph() {}

  void setLanguage(const std::string& language) {language_ = language; }
  std::string getLanguage() {return language_; }

  virtual void close() = 0;

  virtual std::vector<B*> get() = 0;
  virtual B* findBranch(const std::string& name);
  virtual B* findBranchUnsafe(const std::string& name);
  virtual B* create(const std::string& name);

  BranchContainerSet<B> container_;

  std::string language_;

  mutable std::shared_timed_mutex mutex_;
  //use std::lock_guard<std::shared_timed_mutex> lock(Graph<B>::mutex_); to WRITE A DATA
  //use std::shared_lock<std::shared_timed_mutex> lock(Graph<B>::mutex_); to READ A DATA

  inline void removeFromDictionary(std::map<std::string, std::vector<std::string>>& dictionary, const std::string& lang, const std::string& word)
  {
    if(dictionary.find(lang) != dictionary.end())
    {
      for(size_t i = 0; i < dictionary[lang].size();)
        if(dictionary[lang][i] == word)
          dictionary[lang].erase(dictionary[lang].begin() + i);
    }
  }

  template <class T>
  inline void removeFromVect(std::vector<T>& vect, const T& value)
  {
    for(size_t i = 0; i < vect.size();)
      if(vect[i] == value)
        vect.erase(vect.begin() + i);
      else
        i++;
  }

  template <class T>
  inline void removeFromElemVect(std::vector<Single_t<T>>& vect, const T& value)
  {
    for(size_t i = 0; i < vect.size();)
      if(vect[i].elem == value)
        vect.erase(vect.begin() + i);
      else
        i++;
  }

  template<typename T>
  inline void getInMap(T** ptr, const std::string& name, std::map<std::string, T*>& map)
  {
    if(*ptr != nullptr)
      return;

    auto it = map.find(name);
    if(it != map.end())
      *ptr = it->second;
  }

  template<typename C>
  inline void conditionalPushBack(std::vector<C>& vect, const C& data)
  {
    if(std::find(vect.begin(), vect.end(), data) == vect.end())
      vect.push_back(data);
  }
};

template <typename B>
B* Graph<B>::findBranch(const std::string& name)
{
  std::shared_lock<std::shared_timed_mutex> lock(mutex_);
  return container_.find(name);
}

template <typename B>
B* Graph<B>::findBranchUnsafe(const std::string& name)
{
  return container_.find(name);
}

template <typename B>
B* Graph<B>::create(const std::string& name)
{
  B* indiv = nullptr;
  {
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    indiv = container_.find(name);
  }

  if(indiv == nullptr)
  {
    std::lock_guard<std::shared_timed_mutex> lock(mutex_);
    indiv = new B(name);
    container_.insert(indiv);
  }
  return indiv;
}

} // namespace ontologenius

#endif // ONTOLOGENIUS_GRAPH_H
