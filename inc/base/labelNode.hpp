#ifndef LABEL_NODE_HPP
#define LABEL_NODE_HPP

#include "node.hpp"

class LabelNode : public Node
{
private:
  static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return {}; }
  static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return {}; }

  std::string mText = "";
  bool mEditing = false;
    
  virtual void onUpdate() override { }
  virtual void onDraw() override;
    
public:
  LabelNode(const std::string &text="");
  virtual std::string type() const { return "LabelNode"; }       
};

#endif // LABEL_NODE_HPP
