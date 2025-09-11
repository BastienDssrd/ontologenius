#include "ontologenius/core/ontologyIO/Owl/writers/GraphOwlWriter.h"

#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

#include "ontologenius/core/ontoGraphs/Branchs/ValuedNode.h"

namespace ontologenius {

  void GraphOwlWriter::writeBranchStart(const std::string& value)
  {
    writeString("<!-- " + ns_ + "#" + value + " -->\n\n", 1);
    writeString("<" + key_ +" rdf:about=\"" + ns_ + "#" + value + "\">\n", 1);
  }

  void GraphOwlWriter::writeBranchEnd()
  {
    writeString("</" + key_ + ">\n\n\n\n", 1);
  }

  std::string GraphOwlWriter::getRdfResource(const std::string& value)
  {
    return "rdf:resource=\"" + ns_ + "#" + value + "\"";
  }

  std::string GraphOwlWriter::getRdfDatatype(LiteralType* type)
  {
    return "rdf:datatype=\"" + type->getNamespace() + "#" + type->value() + "\"";
  }

  void GraphOwlWriter::writeDictionary(ValuedNode* node) const
  {
    for(auto& it : node->steady_dictionary_.spoken_)
    {
      for(const auto& label : it.second)
      {
        const std::string tmp = "<rdfs:label xml:lang=\"" +
                                it.first +
                                "\">" +
                                label +
                                +"</rdfs:label>\n";
        writeString(tmp, 2);
      }
    }
  }

  void GraphOwlWriter::writeMutedDictionary(ValuedNode* node) const
  {
    for(auto& it : node->steady_dictionary_.muted_)
    {
      for(const auto& label : it.second)
      {
        const std::string tmp = "<onto:label xml:lang=\"" +
                                it.first +
                                "\">" +
                                label +
                                +"</onto:label>\n";
        writeString(tmp, 2);
      }
    }
  }

  void GraphOwlWriter::writeString(const std::string& text, size_t level) const
  {
    if(file_ != nullptr)
    {
      std::string str(level * 4, ' ');
      str += text;
      fwrite(str.c_str(), sizeof(char), str.size(), file_);
    }
  }

} // namespace ontologenius
