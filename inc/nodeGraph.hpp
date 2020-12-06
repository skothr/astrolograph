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
struct ImDrawList;

#define DEFAULT_PROJECT_DIR "./projects"
#define SAVE_FILE_VERSION   "0.2"

#define FILE_DIALOG_SIZE Vec2f(960, 690)

namespace astro
{
  // forward declarations
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


  // TODO: Action stack for undoing with Ctrl-Z
  // enum ActionType
  //   {
  //     ACTION_INVALID = -1,
  //     ACTION_ADD_NODE,
  //     ACTION_REMOVE_NODE,
  //     ACTION_MOVE_NODE,   // (set after full node move -- mouse press, drag, release)
  //     ACTION_DELETE_NODE,
  //     ACTION_CUT,
  //     ACTION_COPY,
  //     ACTION_PASTE,
  //     ACTION_CHANGE_VALUE,
  //   };
  // struct GraphAction
  // {
  //   ActionType type = ACTION_INVALID;
  // };

  
  class NodeGraph
  {
  private:
    // TODO: Action stack for undoing with Ctrl-Z
    // std::deque<GraphAction*> mActionStack;  // for undoing

    ViewSettings *mViewSettings = nullptr;
    std::unordered_map<int, Node*> mNodes; // maps ID to pointer
    std::vector<Node*> mSelectedNodes; // set of nodes that are selected
    Vec2f  mGraphCenter = Vec2f(0,0); // graph-space point to be centered in view
    float  mGraphScale  = 1.0f;       // graph view scaling
    Vec2f  mViewPos;                  // screen-space position of nodeGraph view
    Vec2f  mViewSize;                 // screen-space size of nodeGraph view
    
    bool   mLocked  = false; // if true, nodes can't be selected or moved around
    bool   mDrawing = false; // set to true if between BeginDraw() and EndDraw()
    bool   mShowIds = false; // display id above each node
    
    bool   mSelecting = false;
    Vec2f  mSelectAnchor;
    Rect2f mSelectRect;
    
    bool   mPanning   = false;
    Vec2f  mPanClick;
    
    bool   mPasting   = false;   // true when pasting clipboard
    bool   mPlacing   = false;   // true when placing a new node
    std::string mPlaceType = "";
    Node* mPlaceNode  = nullptr;

    std::vector<Node*> mClipboard;
    bool mClickCopied = false; // set to true when selected nodes are copied (CTRL+click+drag). Reset when mouse released.

    std::string mProjectDir = DEFAULT_PROJECT_DIR;
    bool mChangedSinceSave  = false;
    bool mOpenSave          = false;
    bool mOpenLoad          = false;    
    bool mSaveDialogOpen    = false;
    bool mLoadDialogOpen    = false;
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
    
    NodeGraph(ViewSettings *viewSettings);
    ~NodeGraph();

    ViewSettings* getViewSettings() { return mViewSettings; }
    const std::unordered_map<int, Node*>& getNodes() const { return mNodes; }
    std::unordered_map<int, Node*>& getNodes() { return mNodes; }

    void addNode(Node *n, bool select=true);  // adds node to mNodes
    Node* addNode(const std::string &type, bool select=true); // if select is true, deselect other nodes)
    void placeNode(const std::string &type);   // starts placing a node type.
    void stopPlacing(); // stops placing.
    void clear();
    
    void cut();
    void copy();
    void paste();
    bool undo();
    
    // selection
    void selectNode(Node *n);
    void select(const std::vector<Node*> &nodes);
    void selectAll();
    void moveSelected(const Vec2f &dpos);
    void fixPositions();
    
    std::vector<Node*> getSelected();
    std::vector<Node*> makeCopies(const std::vector<Node*> &group, bool externalConnections=true); // returns new nodes
    void disconnectExternal(const std::vector<Node*> &group, bool disconnectInputs=true, bool disconnectOutputs=true);
    void copySelected();
    void deselectAll();
    void deselect(const std::vector<Node*> &nodes);

    bool isHovered() const;
    bool isSelectedHovered();  // returns true if any selected nodes are hovered
    bool isSelectedActive();   // returns true if any selected nodes are active
    bool isSelectedDragged();  // returns true if any selected nodes are dragged

    void setLocked(bool lock)
    {
      mLocked = lock;
      if(mLocked)
        {
          // mSelectRect = Rect2f(Vec2f(0,0),Vec2f(0,0));
          mSelecting = false;
          mPanning = false;
          mClickCopied = false;
        }
    }
    bool setLocked() const    { return mLocked; }

    Vec2f getCenter() const   { return mGraphCenter; }
    float getScale() const    { return mGraphScale; }
    bool isPanning() const    { return mPanning; }

    Vec2f viewPos() const     { return mViewPos; }
    Vec2f viewSize() const    { return mViewSize; }
    void setPos(const Vec2f &p);
    void setSize(const Vec2f &s);

    Vec2f graphToScreenVec(const Vec2f &v) const { return v*mGraphScale; }
    Vec2f screenToGraphVec(const Vec2f &v) const { return v/mGraphScale; }
    Vec2f graphToScreen(const Vec2f &p) const    { return (p + mGraphCenter)*mGraphScale + mViewSize/2.0f + mViewPos; }
    Vec2f screenToGraph(const Vec2f &p) const    { return (p - mViewSize/2.0f - mViewPos)/mGraphScale - mGraphCenter; }
    Rect2f screenToGraph(const Rect2f &r) const  { return Rect2f(screenToGraph(r.p1), screenToGraph(r.p2)); }
    Rect2f graphToScreen(const Rect2f &r) const  { return Rect2f(graphToScreen(r.p1), graphToScreen(r.p2)); }
    
    // finds a path from start to end point that doesn't intersect any node rects.
    std::vector<Vec2f> findOrthogonalPath(const Vec2f &start, const Vec2f &end);
    
    void draw();
    void update();

    void showIds(bool show) { mShowIds = show; }
    
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
