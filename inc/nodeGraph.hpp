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
    //std::vector<Node::Connection> mSelectedNodes; // set of nodes that are selected
    Vec2f  mGraphCenter = Vec2f(0,0); // graph-space point to be centered in view
    Vec2f  mGraphScale  = Vec2f(1,1); // graph view scaling
    Vec2f  mViewPos;                  // screen-space position of nodeGraph view
    Vec2f  mViewSize;                 // screen-space size of nodeGraph view
    bool   mSelecting = false;
    Vec2f  mSelectAnchor;
    Rect2f mSelectRect;
    bool   mDrawing = false; // set to true if between BeginDraw() and EndDraw()
    bool   mLocked  = false;  // if true, nodes can't be selected or moved around
    ViewSettings *mSettings = nullptr;

    std::vector<Node*> mClipboard;
    //Vec2f mClipMousePos;
    
    std::string mProjectDir = DEFAULT_PROJECT_DIR;
    bool mChangedSinceSave  = false;
    bool mOpenSave          = false;
    bool mOpenLoad          = false;

    bool mPanning     = false;
    Vec2f mPanClick;
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
    
    void addNode(Node *n, bool select=true);  // adds node to mNodes
    Node* addNode(const std::string &type, bool select=true);
    void prepNode(Node *n); // prepares node to be added
    void clear();
    
    void cut();
    void copy();
    void paste();
    
    // selection
    void selectNode(Node *n);
    void select(const std::vector<Node*> &nodes);
    void moveSelected(const Vec2f &dpos);
    std::vector<Node*> getSelected();
    std::vector<Node*> makeCopies(const std::vector<Node*> &group, bool externalConnections=true); // returns new nodes
    void disconnectExternal(const std::vector<Node*> &group, bool disconnectInputs=true, bool disconnectOutputs=true);
    void copySelected();
    void deselectAll();
    void deselect(const std::vector<Node*> &nodes);

    bool isHovered() const { return Rect2f(mViewPos, mViewSize).contains(ImGui::GetMousePos()); }
    
    bool isSelectedHovered();  // returns true if any selected nodes are hovered
    bool isSelectedActive();   // returns true if any selected nodes are active
    bool isSelectedDragged();  // returns true if any selected nodes are dragged

    void setLocked(bool lock) { mLocked = lock; }
    bool setLocked() const    { return mLocked; }

    Vec2f getCenter() const   { return mGraphCenter; }
    bool isPanning() const    { return mPanning; }

    Vec2f viewPos() const     { return mViewPos; }
    Vec2f viewSize() const    { return mViewSize; }
    void setPos(const Vec2i &p);
    void setSize(const Vec2i &s);

    Vec2f graphToScreen(const Vec2f &p) const   { return p + mGraphCenter + mViewSize/2.0f + mViewPos; }
    Vec2f screenToGraph(const Vec2f &p) const   { return p - mGraphCenter - mViewSize/2.0f - mViewPos; }
    Rect2f graphToScreen(const Rect2f &r) const { return r + mGraphCenter + mViewSize/2.0f + mViewPos; }
    Rect2f screenToGraph(const Rect2f &r) const { return r - mGraphCenter - mViewSize/2.0f - mViewPos; }
    
    // finds a path from start to end point that doesn't intersect any node rects.
    std::vector<Vec2f> findOrthogonalPath(const Vec2f &start, const Vec2f &end);
    
    void draw();

    bool isConnecting();
    bool isSelecting() const        { return mSelecting; }
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
