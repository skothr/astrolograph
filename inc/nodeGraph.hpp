#ifndef NODE_GRAPH_HPP
#define NODE_GRAPH_HPP

#include "astro.hpp"
#include "node.hpp"
#include "vector.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

class ImDrawList;

// forward declarations
namespace imgui_addons { class ImGuiFileBrowser; }

#define DEFAULT_PROJECT_DIR "./projects"
#define FILE_DIALOG_SIZE Vec2f(960, 690)

namespace astro
{
  class ViewSettings;

  
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
    std::unordered_map<int, Node*> mNodes;    // maps ID to pointer
    std::unordered_set<Node*> mSelectedNodes; // set of nodes that are selected
    Vec2f  mCenter = Vec2f(0,0); // graph-space point to be centered in view
    Vec2f  mViewPos;             // screen-space position of nodeGraph view
    Vec2f  mViewSize;            // screen-space size of nodeGraph view
    bool   mSelecting = false;
    Vec2f  mSelectAnchor;
    Rect2f mSelectRect;
    bool   mDrawing = false; // set to true if between BeginDraw() and EndDraw()
    bool   mLocked  = false;  // if true, nodes can't be selected or moved around
    ViewSettings *mSettings = nullptr;

    std::vector<Node*> mClipboard;
    //Vec2f mClipMousePos;
    
    //bool  mDrawLines   = true;
    //Vec4f mLineColor   = Vec4f(1, 1, 1, 0.4f);
    //Vec2f mLineSpacing = Vec2f(100, 100);
    //float mLineWidth   = 1.0f;

    std::string mProjectDir = DEFAULT_PROJECT_DIR;
    bool mChangedSinceSave  = false;
    bool mOpenSave          = false;
    bool mOpenLoad          = false;

    bool mClickCopied = false; // set to true when selected nodes are copied (CTRL+click+drag). Reset when mouse released.
    
    bool mSaveDialogOpen = false;
    bool mLoadDialogOpen = false;
    std::string mSaveFile = ""; // last saved/loaded file name
    imgui_addons::ImGuiFileBrowser *mFileDialog;
    
    bool saveToFile(const std::string &path);
    bool loadFromFile(const std::string &path);

    void BeginDraw();
    void EndDraw();
    void drawLines(ImDrawList *drawList);
    
  public:
    static const std::unordered_map<std::string, NodeType> NODE_TYPES;
    static const std::vector<NodeGroup>                    NODE_GROUPS;
    static Node* makeNode(const std::string &nodeType);
    
    NodeGraph(ViewSettings *settings);
    ~NodeGraph();
    
    Node* addNode(const std::string &type);
    void prepNode(Node *n); // prepares node to be added
    void addNode(Node *n);  // adds node to mNodes
    void clear();
    
    void cut();
    void copy();
    void paste();
    
    // selection
    void selectNode(Node *n);
    void moveSelected(const Vec2f &dpos);
    std::vector<Node*> getSelected();
    std::vector<Node*> makeCopies(const std::vector<Node*> &nodes); // returns new nodes
    void copySelected();
    void deselectAll();
    
    bool isSelectedHovered();  // returns true if any selected nodes are hovered
    bool isSelectedActive();   // returns true if any selected nodes are active
    bool isSelectedDragged(); // returns true if any selected nodes are dragged

    void setLocked(bool lock) { mLocked = lock; }
    bool setLocked() const    { return mLocked; }
    
    void setPos(const Vec2i &p);
    void setSize(const Vec2i &s);
    
    void draw();

    bool isSelecting() const        { return mSelecting; }
    bool isConnecting();//             { for(auto n : mNodes) { if(n.second->isConnecting()) { return true; } } return false; }
    bool isSaving() const           { return mOpenSave; }
    bool isLoading() const          { return mOpenLoad; }
    bool unsavedChanges() const     { return mChangedSinceSave; }
    std::string getSaveName() const { return mSaveFile; }

    bool saveOpen() const { return mSaveDialogOpen; }
    bool loadOpen() const { return mLoadDialogOpen; }
    
    void openSaveDialog();
    void openSaveAsDialog();
    void openLoadDialog();
    
  };
};


#endif // NODE_GRAPH_HPP
