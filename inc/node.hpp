#ifndef NODE_HPP
#define NODE_HPP

#include "imgui.h"

#include "astro.hpp"
#include "rect.hpp"

#include <vector>
#include <iomanip>
#include <sstream>
#include <unordered_set>
#include <map>

namespace astro
{


#define NODE_PADDING Vec2f(10.0f, 10.0f)
#define NODE_ROUNDING 6.0f
#define NODE_DEFAULT_BORDER_W 1.0f
#define NODE_DEFAULT_BORDER_COLOR Vec4f(0.5f, 0.5f, 0.5f, 1.0f)
#define NODE_SELECTED_BORDER_W 2.0f
#define NODE_SELECTED_BORDER_COLOR Vec4f(1.0f, 1.0f, 1.0f, 1.0f)
#define NODE_HIGHLIGHTED_BORDER_W 1.0f
#define NODE_HIGHLIGHTED_BORDER_COLOR Vec4f(1.0f, 0.25f, 0.25f, 1.0f)
  // (TEST) //
#define NODE_ACTIVE_BORDER_W 1.0f
#define NODE_ACTIVE_BORDER_COLOR Vec4f(0.25f, 0.25f, 1.0f, 1.0f)
  ////////////
#define NODE_TOP_Z 1000000.0f

  
#define CONNECTOR_SIZE    Vec2f(20.0f, 40.0f)
#define CONNECTOR_PADDING Vec2f(10.0f, 10.0f)
#define CONNECTOR_ROUNDING 6.0f

  enum Direction
    {
      CONNECTOR_INVALID = -1,
      
      CONNECTOR_INPUT = 0,
      CONNECTOR_OUTPUT // TODO: refine input vs. output (bi-directional?)
    };

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
    bool           mNeedReset  = false;
    bool           mConnecting = false;
    Direction      mDirection  = CONNECTOR_INVALID;
    
    std::vector<ConnectorBase*> mConnected;

    void BeginDraw();
    void EndDraw();
    
  public:
    Vec2f screenPos;
    
    ConnectorBase(std::string name="")
      : mName(name) { mThisPtr = this; }
    virtual ~ConnectorBase() { disconnectAll(); }
    virtual std::string type() const = 0;

    void setParent(Node *n, int cId) { mParent = n; mConId = cId; }
    Node* parent()    { return mParent; } // returns parent node
    int conId() const { return mConId; }  // returns connector index in parent node
    
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

    bool needsReset() { return mNeedReset; }
    void reset(bool rs=true);
    void draw();
    void drawConnections(ImDrawList *drawList);
  };
  
  // CONNECTOR //
  template<typename T>
  class Connector : public ConnectorBase
  {
    friend class ConnectorBase;
  protected:
    T *mData = nullptr;
  public:
    Connector(std::string name="", T *data=nullptr)
      : ConnectorBase(name), mData(data)
    { }
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

  
  // NODE //
  class Node
  {
  protected:
    // base class for Node parameters (?)
    struct Params
    {
      int         id = -1;
      std::string name;
      Rect2f      rect;   // rectangle around node
      float       z = -1; // for z ordering
    };

    // size of node components
    Vec2f mInputsSize;
    Vec2f mBodySize;
    Vec2f mOutputsSize;

    NodeGraph *mGraph  = nullptr; // parent node graph
    Params    *mParams = nullptr;
    bool mState        = true;    // state of node window (imgui)
    bool mSelected     = false;   // 
    bool mFirstFrame   = true;    // 
    bool mClicked      = false;   // whether mouse has clicked node window (and is still down)
    bool mHover        = false;   // whether mouse is over node window (window background)
    bool mActive       = false;   // whether mouse is over node window (interactive ui element)
    bool mDragging     = false;   // whether mouse is dragging window
    
    // Vec2f mNextPos = Vec2f(0,0);  // node window pos
    Vec2f mMinSize = Vec2f(1,1);  // min size (to be set by child class)
    
    std::vector<ConnectorBase*> mInputs;
    std::vector<ConnectorBase*> mOutputs;

    // override in child classes to draw node
    virtual bool onDraw() { return true; }

    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const { return params; }
    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params)       { return params; }

    void DrawOutputs();
    void DrawInputs();
    void BeginDraw();
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
    
    Node(const std::vector<ConnectorBase*> &inputs_, const std::vector<ConnectorBase*> &outputs_, const std::string &name="", Params *params=nullptr);
    Node(const Node &other);
    virtual ~Node();
    virtual std::string type() const = 0;
    
    // Copies child class data to other node (must be same type)
    //  --> override in child class if data needs to be copied
    virtual bool copyTo(Node *other)
    {
      if(other && (type() == other->type()))
        {
          std::cout << "NAME: " << name() << ", SIZE: " << size() << ", MINSIZE: " << getMinSize() << ", POS: " << pos() << "\n";
          other->setName(name());
          other->setSize(size());
          other->setMinSize(getMinSize());
          other->setPos(pos());
          // other->mSelected = mSelected;
          // other->mClicked = mClicked;
          // other->mHover = mHover;
          // other->mActive = mActive;
          // other->mDragging = mDragging;
          return true;
        }
      else { return false; }
    }
    
    
    // Copies child class flag data (for UI interaction) to other node (must be same type)
    bool copyFlagsTo(Node *other)
    {
      if(other && (type() == other->type()))
        {
          other->mSelected = mSelected;
          other->mClicked = mClicked;
          other->mHover = mHover;
          other->mActive = mActive;
          other->mDragging = mDragging;
          return true;
        }
      else { return false; }
    }
    
    void setGraph(NodeGraph *graph) { mGraph = graph; }
    
    static std::map<std::string, std::string> getSaveHeader(const std::string &saveStr);    
    std::string toSaveString() const;
    // returns remaining string after base class parameters
    std::string fromSaveString(const std::string &saveStr);
    
    int id() const     { return mParams->id; }
    void setid(int id) { mParams->id = id; }
    const std::string& name() const       { return mParams->name; }
    void setName(const std::string &name) { mParams->name = name; }
    
    void notFirstFrame() { mFirstFrame = false; }
    void bringToFront();
    
    std::vector<ConnectorBase*>& outputs()             { return mOutputs; }
    const std::vector<ConnectorBase*>& outputs() const { return mOutputs; }
    std::vector<ConnectorBase*>& inputs()              { return mInputs;  }
    const std::vector<ConnectorBase*>& inputs() const  { return mInputs;  }

    std::vector<Connection> getInputConnections()
    {
      std::vector<Connection> connections;
      for(int i = 0; i < inputs().size(); i++)
        {
          std::vector<ConnectorBase*> connected = inputs()[i]->getConnected();
          for(int j = 0; j < connected.size(); j++)
            { connections.push_back(Connection{ connected[j]->parent()->id(), connected[j]->conId(), id(), inputs()[i]->conId() }); }
        }
      return connections;
    }
    std::vector<Connection> getOutputConnections()
    {
      std::vector<Connection> connections;
      for(int i = 0; i < outputs().size(); i++)
        {
          std::vector<ConnectorBase*> connected = outputs()[i]->getConnected();
          for(int j = 0; j < connected.size(); j++)
            { connections.push_back(Connection{ id(), outputs()[i]->conId(), connected[j]->parent()->id(), connected[j]->conId() }); }
        }
      return connections;
    }
    std::vector<Connection> getConnections()
    {
      std::vector<Connection> connections  = getInputConnections();
      std::vector<Connection> oConnections = getOutputConnections();
      connections.insert(connections.end(), oConnections.begin(), oConnections.end());
      return connections;
    }

    void disconnectAll() { for(auto c : inputs()) { c->disconnectAll(); } for(auto c : outputs()) { c->disconnectAll(); } }
    bool isConnecting() const;
    bool isSelected() const { return mSelected; }
    void setSelected(bool selected, bool stopDragging = false);

    bool isActive() const   { return mActive; }
    bool isHovered() const  { return mHover; }
    bool isDragging() const { return mDragging; }
    void setDragging(bool drag) { mDragging = drag; }
    
    void setMinSize(const Vec2f &s) { mMinSize = s; }
    Vec2f getMinSize() const        { return mMinSize; }
    // void setNextPos(const Vec2f &p) { mNextPos = p; }
    void setRect(const Rect2f &r) { mParams->rect = r; }
    void setPos(const Vec2f &p);
    void setSize(const Vec2f &s)  { mParams->rect.setSize(s); }
    const Rect2f& rect() const { return mParams->rect; }
    const Vec2f& pos() const   { return mParams->rect.p1; }
    Vec2f size() const         { return mParams->rect.size(); }
    const float& getZ() const  { return mParams->z; }
    float& getZ()              { return mParams->z; }

    void drawConnections(ImDrawList *drawList);
    bool draw(ViewSettings *settings, bool blocked);
  };
}

#endif // NODE_HPP
