#ifndef NODE_GRAPH_HPP
#define NODE_GRAPH_HPP

#include "astro.hpp"
#include "node.hpp"
#include "vector.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

// forward declarations
namespace imgui_addons { class ImGuiFileBrowser; }

#define DEFAULT_PROJECT_DIR "./projects"
#define FILE_DIALOG_SIZE Vec2f(960, 690)

namespace astro
{

  struct NodeType
  {
    std::string            typeName; // string returned by <NodeDerived>::type()>
    std::string            name;     // display name for node type
    std::function<Node*()> get;      // function to create node
  };
  struct NodeGroup
  {
    std::string name;               // group name
    std::vector<std::string> types; // names of types within group
  };

  class NodeGraph
  {
  private:
    std::unordered_map<int, Node*> mNodes; // maps ID to pointer
    Vec2f mCenter = Vec2f(0,0); // graph-space point to be centered in view


    std::string mProjectDir = DEFAULT_PROJECT_DIR;
    bool mChangedSinceSave  = false;
    bool mOpenSave          = false;
    bool mOpenLoad          = false;
    
    bool mSaveDialogOpen = false;
    std::string mSaveFile = ""; // last saved/loaded file name
    imgui_addons::ImGuiFileBrowser *mFileDialog;
    
    bool saveToFile(const std::string &path);
    bool loadFromFile(const std::string &path);
    
  public:
    static const std::unordered_map<std::string, NodeType> NODE_TYPES;
    static const std::vector<NodeGroup>                    NODE_GROUPS;
    static Node* makeNode(const std::string &nodeType);
    
    NodeGraph();
    ~NodeGraph();

    void addNode(Node *n);
    void addNode(const std::string &type);
    void clear();
    
    void draw(const Vec2i &viewSize);

    bool isSaving() const { return mOpenSave; }
    bool unsavedChanges() const { return mChangedSinceSave; }
    std::string getSaveName() const { return mSaveFile; }

    bool saveOpen() const { return mSaveDialogOpen; }
    
    void openSaveDialog();
    void openSaveAsDialog();
    void openLoadDialog();
    
  };
};


#endif // NODE_GRAPH_HPP
