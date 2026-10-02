#ifndef NODE_HPP
#define NODE_HPP

#include <vector>
#include <iomanip>
#include <sstream>
#include <unordered_set>
#include <map>
#include "nlohmann/json_fwd.hpp" // json forward declarations
using json = nlohmann::json;
#define JSON_SPACES 4


#include "rect.hpp"
#include "nodeConnector.hpp"

// node params
#define NODE_PADDING Vec2f(10.0f, 10.0f)
#define NODE_ROUNDING 6.0f
#define NODE_DEFAULT_BORDER_W 1.0f
#define NODE_DEFAULT_BORDER_COLOR Vec4f(0.5f, 0.5f, 0.5f, 1.0f)
#define NODE_SELECTED_BORDER_W 2.0f
#define NODE_SELECTED_BORDER_COLOR Vec4f(1.0f, 1.0f, 1.0f, 1.0f)
#define NODE_HIGHLIGHTED_BORDER_W 1.0f
#define NODE_HIGHLIGHTED_BORDER_COLOR Vec4f(1.0f, 0.25f, 0.25f, 1.0f)
#define NODE_ACTIVE_BORDER_W 1.0f
#define NODE_ACTIVE_BORDER_COLOR Vec4f(0.25f, 0.25f, 1.0f, 1.0f)
#define NODE_CONNECTING_BORDER_W 2.0f
#define NODE_CONNECTING_BORDER_COLOR Vec4f(0.25f, 0.25f, 1.0f, 1.0f)
  
#define NODE_TOP_Z  1000000.0f // z value to bring a node to the top
#define GHOST_ALPHA 0.2f // alpha value for drawing ghosts

#define RESIZE_HOTSPOT_SIZE 20.0f
#define RESIZE_HOTSPOT_COLOR_INACTIVE Vec4f(0.05f, 0.1f, 1.0f, 0.7f)
#define RESIZE_HOTSPOT_COLOR_HOVERED  Vec4f(0.1f,  0.2f, 1.0f, 1.0f)
#define RESIZE_HOTSPOT_COLOR_CLICKED  Vec4f(0.4f,  0.6f, 1.0f, 1.0f)


// forward declarations
struct ImDrawList;

// forward declarations
//class NodeGraph;
class ViewSettings;
class SettingBase;
  
//// NODE ////
class Node
{
private:
  static int NEXT_INTERNAL_ID;
  int mInternalId = -1; // internal ID in case of mId overlap
protected:
  int         mId = -1;
  std::string mName = "";
  Rect2f      mRect;                 // node rect (graph space)
  Vec2f       mMinSize = Vec2f(1,1); // minimum size (to be set by child class)
  float       mZLevel  = -1;         // for z ordering
  Vec4f       mColorMask  = Vec4f(1.0f, 1.0f, 1.0f, 1.0f); // color mask
    
  // size of node components
  // TODO: improve?
  Vec2f mInputsSize;
  Vec2f mBodySize;
  Vec2f mOutputsSize;

  Vec2f mTitlePos;
  Vec2f mTitleSize;
    
  ImDrawList *mBodyDrawList = nullptr;
  ImDrawList *mDrawList     = nullptr;

  std::vector<ConnectorBase*>  mInputs;
  std::vector<ConnectorBase*>  mOutputs;
  std::vector<SettingBase*>    mSettings;

  std::string mTitle = "";

  NodeGraph *mGraph  = nullptr; // parent node graph
  int mGroupLevel = 0;
    
  bool mFirstFrame        = true;    // true only on first frame
  bool mPlacing           = false;   // true as node is being placed, and before mouse is released (to avoid interaction while placing)
  bool mVisible           = true;    // whether node is drawn on screen
  bool mBodyVisible       = true;    // whether node body is drawn on screen
  bool mChanged           = false;   // true if node has changed since last save
  bool mSelected          = false;   // true if node is selected
  bool mClicked           = false;   // whether mouse has clicked node window (and is still down)
  bool mClickedUnselected = false; // whether node was clicked while it was unselected
  bool mResizeClicked     = false;   // whether mouse has clicked resize hotspot (and is still down)
  bool mHover             = false;   // whether mouse is over node window (window background)
  bool mActive            = false;   // whether mouse is over node window (interactive ui element)
  bool mDragging          = false;   // whether mouse is dragging window
  bool mDrawing           = false;   // set to true by BeginDraw() if visible, set to false by EndDraw()
  bool mBlocked           = false;   // whether mouse is blocked by other nodes
  bool mShowConnections   = true;    // set to false to hide connections when pasting
  ConnectorBase *mConnectingTo = nullptr; // valid if connecting a different node and hovering over this one
  ConnectorBase *mHoveredCon   = nullptr; // valid if node connector is being hovered over

  bool  mResizable      = false; // true if node can be resized via lower-right corner
  Vec2f mResizeOffset;           // offset from mouse pos when resize was started to mRect.p2

  bool  mUpdateComputed = false; // set to true each frame once node is updated.

  Vec2f titlePos() const  { return mTitlePos; }
  Vec2f titleSize() const { return mTitleSize; }

  void addInput (ConnectorBase *con);
  void addOutput(ConnectorBase *con);
  void addInputs (const std::vector<ConnectorBase*> &cons);
  void addOutputs(const std::vector<ConnectorBase*> &cons);
  void removeInput (int i);
  void removeOutput(int i);
  void clearInputs ();
  void clearOutputs();

  void handleInput(); // (user input)
  
  // override in child classes to draw node
  virtual void onDraw()   { }
  virtual void onUpdate() { }
  virtual void onResize(const Vec2f &dSize) { }
  virtual void onLoad() { }
    
  bool DrawOutputs(bool blocked);
  bool DrawInputs(bool blocked);
    
public:
  float getBorderWidth() const;
  Vec4f getBorderColor() const;
  
  bool BeginDraw();
  void EndDraw();

  // base class for a Node connection (output --> input)
  struct Connection
  {
    int nodeOut = -1; // output node id
    int conOut  = -1; // output connector id
    int nodeIn  = -1; // input node id
    int conIn   = -1; // input connector id
  };
    
  Node(const std::vector<ConnectorBase*> &inputs_, const std::vector<ConnectorBase*> &outputs_, const std::string &name="", bool resizable=false);
  virtual ~Node();
  virtual std::string type() const = 0;

  json toJSON() const;           // convert settings to json for saving to file
  bool fromJSON(const json &js); // load settings json for reading from file

  void setTitle(const std::string &title) { mTitle = title; }
    
  void setPlacing(bool placing=true) { mPlacing = placing; }
    
  virtual bool onConnect(ConnectorBase *con) { return true; } // return false if connection refused (?)

  // set parent NodeGraph
  void setGraph(NodeGraph *graph) { mGraph = graph; }
  NodeGraph* getGraph() { return mGraph; }
  ViewSettings* getViewSettings();
  float getScale() const; // returns graph scaling
  bool isVisible() const { return mVisible; }
  bool isBodyVisible() const { return mVisible && mBodyVisible; }

  void setVisible(bool visible) { mVisible = visible; } // (for group node --> drawing only body)
  void setGroupLevel(int level) { mGroupLevel = level; }
  bool getGroupLevel() const    { return mGroupLevel; }
    
  // Copies child class data to other node (must be same type)
  //  --> override in child class if data needs to be copied
  bool copyTo(Node *other);
  bool hasChanged() const       { return mChanged; }
  void setChanged(bool changed) { mChanged = changed; }
    
  int id() const     { return mId; }
  void setId(int id) { mId = id; }
    
  const std::string& name() const       { return mName; }
  void setName(const std::string &name) { mName = name; }
    
  std::vector<SettingBase*>& getNodeSettings()             { return mSettings; }
  const std::vector<SettingBase*>& getNodeSettings() const { return mSettings; }
    
  void setMinSize(const Vec2f &s) { mMinSize = s; }
  Vec2f getMinSize() const        { return mMinSize; }
  void setRect(const Rect2f &r)   { mRect = r; }
  void setPos(const Vec2f &p);
  void setSize(const Vec2f &s);//    { mRect.setSize(s); }
    
  const Rect2f& rect() const      { return mRect; }
  const Vec2f& pos() const        { return mRect.p1; }
  Vec2f size() const              { return mRect.size(); }
  float getZ() const              { return mZLevel; }
  void setZ(float z)              { mZLevel = z; }

  Vec2f getBodySize() const { return mBodySize; }
    
  void setFirstFrame(bool firstFrame) { mFirstFrame = firstFrame; }
  void bringToFront();
    
  std::vector<ConnectorBase*>& outputs()             { return mOutputs; }
  const std::vector<ConnectorBase*>& outputs() const { return mOutputs; }
  std::vector<ConnectorBase*>& inputs()              { return mInputs;  }
  const std::vector<ConnectorBase*>& inputs() const  { return mInputs;  }
  std::vector<ConnectorBase*> connectors() const
  {
    std::vector<ConnectorBase*> all = mInputs;
    all.insert(all.end(), mOutputs.begin(), mOutputs.end());
    return all;
  }

  std::vector<Connection> getInputConnections(int conId=-1);
  std::vector<Connection> getOutputConnections(int conId=-1);
  std::vector<Connection> getConnections();
  ConnectorBase* connectingTo()   { return mConnectingTo; }

  void disconnectAll();
  bool isConnecting() const;
  bool isSelected() const         { return mSelected;     }
  void setSelected(bool selected) { mSelected = selected; }
  bool isActive() const           { return mActive; }
  bool isHovered() const          { return mHover; }
  bool isDragging() const         { return mDragging; }
  void setDragging(bool drag)     { mDragging = drag; }
  bool isBlocked() const          { return mBlocked; }
  void setBlocked(bool blocked)   { mBlocked = blocked; }

  void setColorMask(const Vec4f &mask) { mColorMask = mask; }
  Vec4f getColorMask() const { return mColorMask; }

  void setShowConnections(bool show) { mShowConnections = show; }
  bool getShowConnections() const    { return mShowConnections; }

  void bodySeparator(ImDrawList *drawList);
    
  void drawBody();
  bool draw(ImDrawList *graphDrawList, bool blocked);
  void drawConnections(ImDrawList *graphDrawList);

  ImDrawList* drawList()     { return mDrawList; }
  ImDrawList* bodyDrawList() { return mBodyDrawList; }

  bool updateReady();
  bool updated() const { return mUpdateComputed; }  // returns true once all input dependencies are met
  void resetUpdate()   { mUpdateComputed = false; } // returns true once all input dependencies are met
  void update(); // updates node if all dependencies are met

  std::ostream& print(std::ostream &os) const;
    
  friend std::ostream& operator<<(std::ostream &os, const Node *n);
};

inline std::ostream& operator<<(std::ostream &os, const Node *n)
{ return n->print(os); }

#endif // NODE_HPP
