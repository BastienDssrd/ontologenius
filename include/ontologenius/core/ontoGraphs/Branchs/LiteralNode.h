#ifndef ONTOLOGENIUS_LITERALNODE_H
#define ONTOLOGENIUS_LITERALNODE_H

#include <cstddef>
#include <string>

#include "ontologenius/core/ontoGraphs/Branchs/WordTable.h"

namespace ontologenius {

  class LiteralType
  {
  public:
    static WordTable table;

    LiteralType(const std::string& value, const std::string& ns = "") : index_(table.add(value)),
                                                                        namespace_(ns)
    {}

    const std::string& value() const { return table[index_]; }
    index_t get() const { return index_; }
    std::string getNamespace() const { return namespace_; }

    bool operator==(const LiteralType& other) const
    {
      return (index_ == other.get());
    }

    bool operator!=(const LiteralType& other) const
    {
      return (index_ != other.get());
    }

    bool operator==(index_t other) const
    {
      return (get() == other);
    }

    // std::vector<LiteralType*> mothers_;

  private:

    index_t index_;
    std::string namespace_;
  };

  class LiteralNode
  {
  public:
  static WordTable table;

    LiteralType* type_;

    explicit LiteralNode(LiteralType* type, const std::string& value) : type_(type),
                                                                        index_(table.add(type->value() + "#" + value)),
                                                                        data_value_(value)
    {}

    std::string value() const { return table[index_]; }
    const std::string& data() const { return data_value_; }
    index_t get() const { return -index_; }

    bool operator==(const LiteralNode& other) const
    {
      return (index_ == other.index_);
    }

    bool operator!=(const LiteralNode& other) const
    {
      return (index_ != other.index_);
    }

    bool operator==(const std::string& other) const
    {
      return (value() == other);
    }

    bool operator==(index_t other) const
    {
      return (get() == other);
    }

  private:
    index_t index_;
    std::string data_value_;
  };

} // namespace ontologenius

#endif // ONTOLOGENIUS_LITERALNODE_H
