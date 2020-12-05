#ifndef GROUP_NODE_HPP
#define GROUP_NODE_HPP

#include "node.hpp"

namespace astro
{
  class GroupNode : public Node
  {
  protected:
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return {}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return {new Connector<Location>("Location Output")}; }

    std::vector<Node*> mContents;
    
    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const override
    {
      // params.emplace("savedName", mWidget.getName());
      // if(mWidget.getName().empty() || mWidget.get() != mWidget.getSaved())
      //   { params.emplace("location",  mWidget.get().toSaveString()); }
      // //params.emplace("dst",  (mWidget.getDST() ? "1" : "0"));
      return params;
    };
    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    {
      // std::string saveName = params["savedName"];
      // std::cout << "LOC: " << params["location"] << " | NAME: " << saveName << "\n";
      // if(saveName.empty())
      //   {
      //     mWidget.get().fromSaveString(params["location"]);
      //     mWidget.setSaved(mWidget.get());
      //     mWidget.setName("");
      //     mWidget.setSaveName("");
      //   }
      // else
      //   {
      //     mWidget.load(saveName);
      //     mWidget.setName(saveName);
      //     mWidget.setSaveName(saveName);
      //   }
      return params;
    };
    virtual void onUpdate() override;
    virtual void onDraw() override;
    
  public:
    GroupNode();
    ~GroupNode();
    virtual std::string type() const { return "GroupNode"; }
    
    virtual bool copyTo(Node *other) override
    { // copy settings
      if(Node::copyTo(other))
        {
          // // TODO: copy node contents and internal connections
          // ((GroupNode*)other)->mContents.clear();
          // for(auto n : mContents) { ((GroupNode*)other)->mContents.push_back(); }
          return true;
        }
      else { return false; }
    }
    
  };
}
#endif // GROUP_NODE_HPP
