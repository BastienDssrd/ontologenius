#ifndef ONTOLOGENIUS_VERSIONOR_H
#define ONTOLOGENIUS_VERSIONOR_H

#include <unordered_map>

#include "ontologenius/core/feeder/FeedStorage.h"
#include "ontologenius/core/feeder/VersionNode.h"

namespace ontologenius {

  class Versionor
  {
  public:
    explicit Versionor(FeedStorage* storage);
    Versionor(const Versionor& other) = delete;
    ~Versionor();

    Versionor& operator=(const Versionor& other) = delete;

    void activate(bool activated) { activated_ = activated; }

    void insert(Feed_t data);
    bool commit(const std::string& id);
    bool checkout(const std::string& id);
    std::string getCurrentCommit();
    size_t getNbData();
    bool areSameStates(const std::string& from_node, const std::string& to_node);

    void print() { nodes_["0"]->print(); }
    void exportToXml(const std::string& path);

  private:
    bool activated_;
    FeedStorage* storage_;
    size_t order_;

    std::unordered_map<std::string, VersionNode*> nodes_;
    VersionNode* current_node_;

    std::vector<Feed_t> getDataBetween(VersionNode* from_node, VersionNode* to_node);
    std::vector<VersionNode*> getPrevs(VersionNode* from_node);
    std::unordered_map<std::string, VersionNode*>::iterator getNode(const std::string& id);
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_VERSIONOR_H
