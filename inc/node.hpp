#ifndef NODE_HPP
#define NODE_HPP

#include "astro.hpp"
#include "rect.hpp"

#include "param.hpp"
#include "viewSettings.hpp"
#include "setting.hpp"

#include <vector>
#include <iomanip>
#include <sstream>
#include <unordered_set>
#include <map>

// forward declarations
struct ImDrawList;

namespace astro
{
  // connector params
#define CONNECTOR_SIZE          Vec2f(18.0f, 36.0f)
#define CONNECTOR_POINT_RADIUS  4.0f
#define CONNECTOR_PADDING       Vec2f(10.0f, 10.0f)
#define CONNECTOR_PROTRUDE      (Vec2f(CONNECTOR_SIZE.x/2,0) + Vec2f(CONNECTOR_PADDING.x,0) + Vec2f(16.0f, 0.0f)) // vector from connection center to start point
#define CONNECTOR_ROUNDING      5.0f
#define CONNECTOR_SEGMENT_ERROR 0.1f
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
  
#define NODE_TOP_Z 1000000.0f // z value to bring a node to the top

#define GHOST_ALPHA 0.2f // alpha value for drawing ghosts

// direction of node connector (for now just input/output)
  enum Direction
    {
      CONNECTOR_INVALID = -1,
      CONNECTOR_INPUT = 0,
      CONNECTOR_OUTPUT // TODO: refine input vs. output (bi-directional?)
    };

  enum NodeSignal
    {
      NODE_SIGNAL_INVALID = -1,
      NODE_SIGNAL_NONE    = 0,
      NODE_SIGNAL_RESET   = 0x01,
      NODE_SIGNAL_CHANGED = 0x02,
    };

  // forward declarations
  class Node;
  template<typename T> class Connector;
  class NodeGraph;
  class ViewSettings;
  
  // CONNECTOR BASE //
  class ConnectorBase
  {
  protected:
    ConnectorBase *mThisPtr = nullptr;
    Node          *mParent  = nullptr;
    int            mConId   = -1;      // index of this connector in parent node
    std::string    mName;
    NodeSignal     mSignals    = NODE_SIGNAL_NONE;
    bool           mConnecting = false;
    Direction      mDirection  = CONNECTOR_INVALID;
    
    std::vector<ConnectorBase*> mConnected;
    
  public:
    Vec2f graphPos;
    Vec2f getProtrudePos() { return graphPos + (mDirection == CONNECTOR_INPUT ? -1.0f : 1.0f)*CONNECTOR_PROTRUDE; }
    
    ConnectorBase(std::string name="")
      : mName(name) { mThisPtr = this; }
    virtual ~ConnectorBase() { disconnectAll(); }
    virtual std::string type() const = 0;

    void setParent(Node *n, int cId) { mParent = n; mConId = cId; }
    Node* parent()    { return mParent; } // returns parent node
    int conId() const { return mConId; }  // returns connector index in parent node
    Direction direction() const { return mDirection; }  // returns connector direction (input/output)
    
    template<typename T>
    T* get() { return ((Connector<T>*)this)->get(); }
    template<typename T>
    void set(T *data) { ((Connector<T>*)this)->set(data); }
    
    void setDirection(Direction dir) { mDirection = dir; }    
    bool connect(ConnectorBase *other, bool force=false);
    void disconnect(ConnectorBase *other);
    void disconnectAll();
    
    std::vector<ConnectorBase*> getConnected() { return mConnected; }
    bool isConnected() const { return (mConnected.size() > 0); }
    bool isConnecting()      { return mConnecting; }
    void beginConnecting()   { mConnecting = true; }
    void endConnecting()     { mConnecting = false; }

    void sendSignal(NodeSignal signal);
    
    void draw(bool blocked);
    void drawConnections(ImDrawList *nodeDrawList, ImDrawList *graphDrawList);
  };


  
  //// CONNECTOR ////
  template<typename T>
  class Connector : public ConnectorBase
  {
    friend class ConnectorBase;
  protected:
    T *mData = nullptr;
  public:
    Connector(std::string name="", T *data=nullptr) : ConnectorBase(name), mData(data) { }
    virtual ~Connector() { }
    virtual std::string type() const override { return std::string(typeid(T).name()); }
    
    T* get()
    {
      if(mDirection == CONNECTOR_INPUT) { return (mConnected.size() > 0 ? ((Connector<T>*)mConnected[0])->mData : mData); }
      else                              { return mData; }
    }
    void set(T *data)
    {
      mData = data;
      if(mDirection == CONNECTOR_INPUT && mConnected.size() > 0)
        { ((Connector<T>*)mConnected[0])->mData = data; }
    }
  };
  ///////////////////


  
  
  //// NODE ////
  class Node
  {
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

    std::vector<ConnectorBase*> mInputs;
    std::vector<ConnectorBase*> mOutputs;
    std::vector<SettingBase*> mSettings;

    NodeGraph *mGraph  = nullptr; // parent node graph
    
    bool mFirstFrame   = true;    // true only on first frame
    bool mVisible      = true;    // whether node is drawn on screen
    bool mBodyVisible  = true;    // whether node body is drawn on screen
    bool mChanged      = false;   // true if node has changed since last save
    bool mSelected     = false;   // true of node is selected
    bool mClicked      = false;   // whether mouse has clicked node window (and is still down)
    bool mHover        = false;   // whether mouse is over node window (window background)
    bool mActive       = false;   // whether mouse is over node window (interactive ui element)
    bool mDragging     = false;   // whether mouse is dragging window
    bool mDrawing      = false;   // set to true by BeginDraw() if visible, set to false by EndDraw()
    bool mBlocked      = false;   // whether mouse is blocked by other nodes
    bool mShowConnections = true; // set to false to hide connections when pasting

    // override in child classes to draw node
    virtual void onDraw()   { }
    virtual void onUpdate() { }

    float getBorderWidth() const
    {
      float borderW = NODE_DEFAULT_BORDER_W;
      if(mActive)        { borderW = NODE_ACTIVE_BORDER_W;      }
      else if(mSelected) { borderW = NODE_SELECTED_BORDER_W;    }
      else if(mHover)    { borderW = NODE_HIGHLIGHTED_BORDER_W; }
      return borderW;
    }

    Vec4f getBorderColor() const
    {
      Vec4f borderColor = NODE_DEFAULT_BORDER_COLOR;
      if(mActive)        { borderColor = NODE_ACTIVE_BORDER_COLOR;      }
      else if(mSelected) { borderColor = NODE_SELECTED_BORDER_COLOR;    }
      else if(mHover)    { borderColor = NODE_HIGHLIGHTED_BORDER_COLOR; }
      return borderColor;
    }

    void DrawOutputs(bool blocked);
    void DrawInputs(bool blocked);
    bool BeginDraw();
    void EndDraw();
    
  public:
    static int NEXT_ID;

    // base class for a Node connection (output --> input)
    struct Connection
    {
      int nodeOut = -1; // output node id
      int conOut  = -1; // output connector id
      int nodeIn  = -1; // input node id
      int conIn   = -1; // input connector id
    };
    
    Node(const std::vector<ConnectorBase*> &inputs_, const std::vector<ConnectorBase*> &outputs_, const std::string &name="");
    Node(const Node &other);
    virtual ~Node();
    virtual std::string type() const = 0;

    json toJson() const;           // convert settings to json for saving to file
    bool fromJson(const json &js); // load settings json for reading from file
    
    virtual bool onConnect(ConnectorBase *con) { return true; } // return false if connection refused (?)

    // set parent NodeGraph
    void setGraph(NodeGraph *graph) { mGraph = graph; }
    NodeGraph* getGraph() { return mGraph; }
    ViewSettings* getViewSettings();
    float getScale() const; // returns graph scaling
    bool isVisible() const { return mVisible; }
    bool isBodyVisible() const { return mVisible && mBodyVisible; }
    
    // Copies child class data to other node (must be same type)
    //  --> override in child class if data needs to be copied
    bool copyTo(Node *other)
    {
      if(other && (type() == other->type()))
        {
          std::cout << "COPYING NODE --> " << toJson() << "\n";
          std::cout << "          TO --> " << other->toJson() << "\n";
          int oldId = other->id();
          // other->setGraph(mGraph);
          other->fromJson(toJson());
          other->setId(oldId);
          std::cout << "COPIED TO    --> " << other->toJson() << "\n";
          return true;
        }
      else { return false; }
    }

    bool hasChanged() const       { return mChanged; }
    void setChanged(bool changed) { mChanged = changed; }
    
    int id() const     { return mId; }
    void setId(int id) { mId = id; }
    
    const std::string& name() const       { return mName; }
    void setName(const std::string &name) { mName = name; }
    
    void setMinSize(const Vec2f &s) { mMinSize = s; }
    Vec2f getMinSize() const        { return mMinSize; }
    void setRect(const Rect2f &r)   { mRect = r; }
    void setPos(const Vec2f &p);
    void setSize(const Vec2f &s)    { mRect.setSize(s); }
    
    const Rect2f& rect() const      { return mRect; }
    const Vec2f& pos() const        { return mRect.p1; }
    Vec2f size() const              { return mRect.size(); }
    float getZ() const              { return mZLevel; }
    void setZ(float z)              { mZLevel = z; }
    
    void setFirstFrame(bool firstFrame) { mFirstFrame = firstFrame; }
    void bringToFront();
    
    std::vector<ConnectorBase*>& outputs()             { return mOutputs; }
    const std::vector<ConnectorBase*>& outputs() const { return mOutputs; }
    std::vector<ConnectorBase*>& inputs()              { return mInputs;  }
    const std::vector<ConnectorBase*>& inputs() const  { return mInputs;  }

    std::vector<Connection> getInputConnections();
    std::vector<Connection> getOutputConnections();
    std::vector<Connection> getConnections();

    void disconnectAll();
    bool isConnecting() const;
    bool isSelected() const         { return mSelected; }
    void setSelected(bool selected) { mSelected = selected; }
    bool isActive() const           { return mActive; }
    bool isHovered() const          { return mHover; }
    bool isDragging() const         { return mDragging; }
    void setDragging(bool drag)     { mDragging = drag; }
    bool isBlocked() const          { return mBlocked; }

    void setColorMask(const Vec4f &mask) { mColorMask = mask; }
    Vec4f getColorMask() const { return mColorMask; }

    void setShowConnections(bool show) { mShowConnections = show; }
    bool getShowConnections() const    { return mShowConnections; }

    void drawBody(bool blocked);
    bool draw(ImDrawList *graphDrawList, bool blocked);
    void drawConnections(ImDrawList *graphDrawList);
    void update();
  };
}

#endif // NODE_HPP
