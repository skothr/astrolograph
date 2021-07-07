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
#include "undo.hpp"
#include "quadTree.hpp"

// forward declarations
struct ImDrawList;

#define SAVE_FILE_VERSION   "0.2"

#define GRAPH_SCALE_VEL 1.04f // (multiplier)
#define GRAPH_SCALE_MIN 0.1f
#define GRAPH_SCALE_MAX 25.0f

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
    ////////////
    // (QUADTREE TEST) //
    QuadTree<Node> *mQuad = nullptr; // node quadtree test
    ////////////

    AstroWindow  *mWindow       = nullptr; // parent window
    ViewSettings *mViewSettings = nullptr; // view settings
    ImDrawList   *mWinDrawList  = nullptr; // draw list for graph window
    
    Vec2f  mGraphCenter = Vec2f(0,0);      // graph-space point to be centered in view
    float  mGraphScale  = 1.0f;            // graph view scaling
    Vec2f  mViewPos;                       // screen-space position of nodeGraph view
    Vec2f  mViewSize;                      // screen-space size of nodeGraph view
    std::unordered_map<int, Node*> mNodes; // maps node ID to pointer
    std::vector<Node*> mSelectedNodes;     // nodes that are selected
    Node  *mHoveredNode = nullptr;         // top node being hovered by the mouse
    int NEXT_ID = 0;  // id to give next created node (used for connection consistency)

    // undo/redo
    UndoStack mUndoStack;    // action stack for undoing (Ctrl-Z)
    UndoStack mRedoStack;    // action stack for redoing (Ctrl-Shift-Z)
    Vec2f     mNodeMoveDPos; // accumulates a full node move while dragging
    json *mAddNodeJSON = nullptr;
    
    bool mDrawing        = false; // set to true if between BeginDraw() and EndDraw() to avoid accidental recursion
    bool mLocked         = false; // if true, nodes can't be selected or moved around
    bool mSelected       = false; // true when graph was the last thing clicked
    bool mScaling        = false; // true when scaling graph (ctrl+scroll)
    bool mPanning        = false; // true when panning graph center (middleclick / shift+leftclick)
    bool mSelecting      = false; // true when selecting nodes with select rect (click+drag on background)
    bool mMovingNodes    = false; // true when moving nodes (click+drag)
    bool mCopying        = false; // true when directly copying selection (ctrl+click+drag selected) 
    bool mClickCopied    = false; // true if nodes were directly copied (ctrl+click+drag, while still dragging)
    bool mShowIds        = false; // display id above each node (debug/info)
    bool mUnsavedChanges = false; // true if anything has changed that needs to be saved

    Vec2f  mSelectAnchor;         // point where mouse first pressed when selecting
    Rect2f mSelectRect;           // when selecting -- rect within which nodes will be selected/toggled
    Vec2f  mPanClick;             // point where mouse first pressed when panning
    
    void drawLines(ImDrawList *drawList);
    
  public:
    static const std::unordered_map<std::string, NodeType> NODE_TYPES;
    static const std::vector<NodeGroup>                    NODE_GROUPS;
    static Node* makeNode(const std::string &nodeType);

    // std::vector<KeyBinding>& getKeyBindings()
    NodeGraph(AstroWindow *window, ViewSettings *viewSettings);
    ~NodeGraph();

    // begin/end imgui window
    void BeginDraw();
    void EndDraw();

    // save/load graph
    json toJSON() const;
    bool fromJSON(json js);

    int& nextId() { return NEXT_ID; }
    
    AstroWindow* getWindow()                         { return mWindow; }
    ViewSettings* getViewSettings()                  { return mViewSettings; }

    ImDrawList* getWinDrawList() { return mWinDrawList; }
    
    void setLocked(bool lock)
    {
      mLocked = lock;
      if(mLocked) { mSelecting = false; mPanning = false; mClickCopied = false; }
    }
    bool isLocked() const     { return mLocked; }

    Vec2f getCenter() const   { return mGraphCenter; }
    float getScale() const    { return mGraphScale; }
    bool isPanning() const    { return mPanning; }

    Vec2f viewPos() const     { return mViewPos; }
    Vec2f viewSize() const    { return mViewSize; }
    void setPos(const Vec2f &p);
    void setSize(const Vec2f &s);

    // coordinate conversion
    Vec2f graphToScreenVec(const Vec2f &v) const { return v*mGraphScale; }
    Vec2f screenToGraphVec(const Vec2f &v) const { return v/mGraphScale; }
    Vec2f graphToScreen(const Vec2f &p) const    { return (p + mGraphCenter)*mGraphScale + mViewSize/2.0f + mViewPos; }
    Vec2f screenToGraph(const Vec2f &p) const    { return (p - mViewSize/2.0f - mViewPos)/mGraphScale - mGraphCenter; }
    Rect2f screenToGraph(const Rect2f &r) const  { return Rect2f(screenToGraph(r.p1), screenToGraph(r.p2)); }
    Rect2f graphToScreen(const Rect2f &r) const  { return Rect2f(graphToScreen(r.p1), graphToScreen(r.p2)); }

    const std::unordered_map<int, Node*>& getNodes() const { return mNodes; }
    std::unordered_map<int, Node*>& getNodes()             { return mNodes; }
    void addNode(Node *n, bool select=true);   // adds node to mNodes (sets id)
    void addNodes(std::vector<Node*> &nodes, bool select=true);   // adds node to mNodes (sets id)
    void doneAdding();
    
    void removeNodes(std::vector<Node*> &nodes, bool deleteNodes=true);   // removes nodes from mNodes
    void clear();

    bool undo();
    bool redo();
    
    // selection
    void selectNode(Node *n);
    void select(const std::vector<Node*> &nodes);
    void selectAll();
    void moveSelected(const Vec2f &dpos);
    void doneMoving();
    void fixPositions();
    
    std::vector<Node*> getSelected() { return mSelectedNodes; }
    std::vector<Node*> makeCopies(const std::vector<Node*> &group, bool externalConnections=true); // returns new nodes
    void disconnectExternal(const std::vector<Node*> &group, bool disconnectInputs=true, bool disconnectOutputs=true);
    void deselectAll();
    void deselect(const std::vector<Node*> &nodes);

    void copySelected();
    Node* getHovered() { return mHoveredNode; }
    
    bool isHovered() const;
    bool isSelected() const { return mSelected; }
    bool isSelectedHovered();  // returns true if any selected nodes are hovered
    bool isSelectedActive();   // returns true if any selected nodes are active
    bool isSelectedDragged();  // returns true if any selected nodes are dragged

    bool isScaling() const   { return mScaling; }
    bool isSelecting() const { return mSelecting; }
    
    Node* groupNodes(const std::vector<Node*> &nodes);
    Node* groupSelected();
    std::vector<Node*> ungroupNodes(const std::vector<Node*> &nodes);
    std::vector<Node*> ungroupSelected();
    
    // finds a path from start to end point that doesn't intersect any node rects.
    std::vector<Vec2f> findOrthogonalPath(const Vec2f &start, const Rect2f &startRect, const Vec2f &end, const Rect2f &endRect, Direction direction);
    
    void draw();
    void update(double dt);

    void showIds(bool show) { mShowIds = show; }
    
    bool isConnecting();
    ConnectorBase* getConnectingFrom();
    bool unsavedChanges() const     { return mUnsavedChanges; }
    void setUnsaved(bool unsaved)   { mUnsavedChanges = unsaved; }
  };
};


#endif // NODE_GRAPH_HPP
