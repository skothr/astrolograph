#ifndef NODE_GRAPH_HPP
#define NODE_GRAPH_HPP

#include "astro.hpp"
#include "node.hpp"
#include "vector.hpp"

#include <string>
#include <unordered_map>
#include <functional>

// forward declarations
namespace imgui_addons { class ImGuiFileBrowser; }

#define DEFAULT_PROJECT_DIR "./projects"
#define FILE_DIALOG_SIZE Vec2f(960, 690)

namespace astro
{
  class NodeGraph
  {
  private:
    static const std::unordered_map<std::string, std::function<Node*()>> NODE_TYPES;
    static Node* makeNode(const std::string &nodeType);
    
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
    NodeGraph();
    ~NodeGraph();

    void addNode(Node *n);
    void clear();
    
    void draw();

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
