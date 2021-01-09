#ifndef NODE_GRAPH_HPP
#define NODE_GRAPH_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <chrono>
#include "nlohmann/json_fwd.hpp" // json forward declarations
using json = nlohmann::json;

#include "astro.hpp"
#include "node.hpp"
#include "vector.hpp"
#include "keyBinding.hpp"
#include "undo.hpp"
#include "quadTree.hpp"


// forward declarations
struct ImDrawList;

#define SAVE_FILE_VERSION   "0.2"

#define GRAPH_SCALE_VEL 1.04f // (multiplier)
#define GRAPH_SCALE_MIN 0.25f
#define GRAPH_SCALE_MAX 4.0f

#define MAX_UNDO_STACK_SIZE 2048

namespace astro
{
  // forward declarations
  class AstroWindow;
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
    AstroWindow *mWindow = nullptr;

    // TEST
    QuadTree<Node> *mQuad = nullptr; // node quadtree test
    
    UndoStack mUndoStack;    // action stack for undoing (Ctrl-Z)
    UndoStack mRedoStack;    // action stack for redoing (Ctrl-Shift-Z)
    Vec2f     mNodeMoveDPos; // accumulates a full node move while dragging

    Node *mHoveredNode = nullptr;
    json *mAddNodeJSON = nullptr;

    std::vector<KeyBinding> mKeyBindings;
    std::vector<KeyBinding> mDefaultKeyBindings;
    
    double mDt = 0.0;
    ViewSettings *mViewSettings = nullptr;
    std::unordered_map<int, Node*> mNodes; // maps ID to pointer
    int NEXT_ID = 0;
    std::vector<Node*> mSelectedNodes; // set of nodes that are selected
    Vec2f  mGraphCenter = Vec2f(0,0);  // graph-space point to be centered in view
    float  mGraphScale  = 1.0f;        // graph view scaling
    Vec2f  mViewPos;                   // screen-space position of nodeGraph view
    Vec2f  mViewSize;                  // screen-space size of nodeGraph view

    bool   mSelected = false; // true when graph was the last thing clicked
    bool   mLocked   = false;  // if true, nodes can't be selected or moved around
    bool   mDrawing  = false;  // set to true if between BeginDraw() and EndDraw()
    bool   mShowIds  = false;  // display id above each node

    bool   mScaling = false;
    std::chrono::high_resolution_clock::time_point mLastT;
    
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
    bool mCopying     = false;
    bool mClickCopied = false; // set to true when selected nodes are copied (CTRL+click+drag). Reset when mouse released.

    bool mChangedSinceSave  = false;
    // bool mOpenSave          = false;
    // bool mOpenLoad          = false;    
    // bool mSaveDialogOpen    = false;
    // bool mLoadDialogOpen    = false;
    // std::string mSaveFile = ""; // last saved/loaded file name
    
    void BeginDraw();
    void EndDraw();
    void drawLines(ImDrawList *drawList);
    
  public:
    static const std::unordered_map<std::string, NodeType> NODE_TYPES;
    static const std::vector<NodeGroup>                    NODE_GROUPS;
    static Node* makeNode(const std::string &nodeType);
    
    NodeGraph(AstroWindow *window, ViewSettings *viewSettings);
    ~NodeGraph();

    // save/load graph
    json toJSON() const;
    bool fromJSON(json js);
    
    AstroWindow* getWindow()                         { return mWindow; }
    ViewSettings* getViewSettings()                  { return mViewSettings; }
    std::vector<KeyBinding>& getKeyBindings()        { return mKeyBindings; }
    std::vector<KeyBinding>& getDefaultKeyBindings() { return mDefaultKeyBindings; }
    
    const std::unordered_map<int, Node*>& getNodes() const { return mNodes; }
    std::unordered_map<int, Node*>& getNodes()             { return mNodes; }

    void addNode(Node *n, bool select=true);   // adds node to mNodes (sets id)
    void doneAdding();
    
    void placeNode(const std::string &type);   // starts placing a node type.
    void stopPlacing(); // stops placing node
    void stopPasting(); // stops pasting node(s)
    void clear();
    
    void cut();
    void copy();
    void paste();
    bool undo();
    bool redo();
    
    // selection
    void selectNode(Node *n);
    void select(const std::vector<Node*> &nodes);
    void selectAll();
    void moveSelected(const Vec2f &dpos);
    void doneMoving();
    void fixPositions();
    
    std::vector<Node*> getSelected();
    std::vector<Node*> makeCopies(const std::vector<Node*> &group, bool externalConnections=true); // returns new nodes
    void disconnectExternal(const std::vector<Node*> &group, bool disconnectInputs=true, bool disconnectOutputs=true);
    void deselectAll();
    void deselect(const std::vector<Node*> &nodes);

    void copySelected();
    
    Node* groupNodes(const std::vector<Node*> &nodes);
    Node* groupSelected();
    std::vector<Node*> ungroupNodes(const std::vector<Node*> &nodes);
    std::vector<Node*> ungroupSelected();

    Node* getHovered()
    { return mHoveredNode; }
    
    bool isHovered() const;
    bool isSelected() const { return mSelected; }
    bool isSelectedHovered();  // returns true if any selected nodes are hovered
    bool isSelectedActive();   // returns true if any selected nodes are active
    bool isSelectedDragged();  // returns true if any selected nodes are dragged

    void setLocked(bool lock)
    {
      mLocked = lock;
      if(mLocked)
        {
          mSelecting = false;
          mPanning = false;
          mClickCopied = false;
        }
    }
    bool isLocked() const     { return mLocked; }

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
    std::vector<Vec2f> findOrthogonalPath(const Vec2f &start, const Rect2f &startRect, const Vec2f &end, const Rect2f &endRect, Direction direction);
    
    void draw();
    void update(double dt);

    void showIds(bool show) { mShowIds = show; }
    
    bool isConnecting();
    ConnectorBase* getConnectingTo();
    bool isSelecting() const        { return mSelecting; }
    bool unsavedChanges() const     { return mChangedSinceSave; }
    void setUnsaved(bool unsaved)   { mChangedSinceSave = unsaved; }
  };
};


#endif // NODE_GRAPH_HPP
