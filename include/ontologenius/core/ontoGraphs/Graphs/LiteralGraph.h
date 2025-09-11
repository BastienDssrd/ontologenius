#ifndef ONTOLOGENIUS_LITERALGRAPH_H
#define ONTOLOGENIUS_LITERALGRAPH_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "ontologenius/core/ontoGraphs/Branchs/LiteralNode.h"
#include "ontologenius/core/ontoGraphs/BranchContainer/BranchContainerSet.h"

namespace ontologenius {

  class LiteralGraph
  {
  public:
    LiteralGraph();
    LiteralGraph(const LiteralGraph& other);
    ~LiteralGraph() = default;

    void deepCopy(const LiteralGraph& other);

    LiteralNode* findOrCreate(const std::string& value);
    LiteralNode* findOrCreate(const std::string& type, const std::string& value);
    LiteralType* findOrCreateType(const std::string& value, const std::string& ns = "");

    LiteralNode* find(const std::string& type, const std::string& value);
    LiteralNode* find(const std::string& value);
    LiteralNode* find(index_t index);

    std::vector<index_t> getIndexes(const std::vector<std::string>& values);
    std::vector<std::string> getIdentifiers(const std::vector<index_t>& values);

  private:
    BranchContainerSet<LiteralNode> literal_container_;
    std::vector<LiteralNode*> all_literals_;
    std::vector<LiteralType*> all_types_;
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_LITERALGRAPH_H
