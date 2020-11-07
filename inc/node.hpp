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
#define CONNECTOR_PADDING Vec2f(10.0f, 10.0f)
#define CONNECTOR_SIZE    Vec2f(20.0f, 40.0f)

  enum ConnectorDir
    {
      CONNECTOR_INVALID = -1,
      
      CONNECTOR_INPUT = 0,
      CONNECTOR_OUTPUT // TODO: refine input vs. output (bi-directional?)
    };

  // class ImDrawList;
  
  class Node;
  template<typename T> class Connector;
  class NodeGraph;

  // CONNECTOR BASE //
  class ConnectorBase
  {
  protected:
    ConnectorBase *mThisPtr = nullptr;
    Node          *mParent  = nullptr;
    int            mConId   = -1;      // index of this connector in parent node
    std::vector<ConnectorBase*> mConnected;
    std::string mName;
    bool mConnecting = false;
    ConnectorDir mDirection = CONNECTOR_INVALID;
    bool mNeedReset = false;

    void BeginDraw();
    void EndDraw();
    
  public:
    Vec2f screenPos;
    
    ConnectorBase(std::string name="")
      : mName(name) { mThisPtr = this; }
    virtual ~ConnectorBase() { disconnectAll(); }
    virtual std::string type() const = 0;

    void setParent(Node *n, int cId) { mParent = n; mConId = cId; }
    Node* parent()     { return mParent; } // returns parent node
    int conId() const  { return mConId; }  // returns connector index in parent node
    
    template<typename T>
    T* get() { return ((Connector<T>*)this)->get(); }
    template<typename T>
    void set(T *data) { ((Connector<T>*)this)->set(data); }
    
    void setDirection(ConnectorDir dir) { mDirection = dir; }    
    bool connect(ConnectorBase *other);
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
      : ConnectorBase(name), mData(data) { }
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

  // base class for Node parameters that should be saved/loaded
  struct NodeParams
  {
    int         id = -1;
    std::string name;
    Rect2f      rect;
  };
  
  // NODE //
  class Node
  {
  protected:
    NodeGraph  *mGraph  = nullptr; // parent node graph
    NodeParams *mParams = nullptr;
    bool mState      = true;
    bool mSelected   = false;
    bool mFirstFrame = true;
    bool mDragging   = false;
    bool mActive     = false;
    bool mHover     = false;

    
    // Vec2f mNextPos = Vec2f(0,0);  // node window pos
    Vec2f mMinSize = Vec2f(0,0);  // min size (to be set by child class)
    
    std::vector<ConnectorBase*> mInputs;
    std::vector<ConnectorBase*> mOutputs;

    // override in child classes to draw node
    virtual bool onDraw() { return true; }

    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const { return params; }
    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params)       { return params; }

    bool drawOutputs();
    bool drawInputs();

    void BeginDraw();
    void EndDraw();
    
  public:
    static int NEXT_ID;
    static std::unordered_set<int> ACTIVE_IDS;
    
    Node(const std::vector<ConnectorBase*> &inputs_, const std::vector<ConnectorBase*> &outputs_, const std::string &name="", NodeParams *params=nullptr);
    virtual ~Node();
    virtual std::string type() const = 0;
    
    void setGraph(NodeGraph *graph) { mGraph = graph; }

    static std::map<std::string, std::string> getSaveHeader(const std::string &saveStr);    
    std::string toSaveString() const;
    // returns remaining string after base class parameters
    std::string fromSaveString(const std::string &saveStr);
    
    int id() const { return mParams->id; }
    const std::string& name() const { return mParams->name; }
    
    std::vector<ConnectorBase*>& outputs()             { return mOutputs; }
    const std::vector<ConnectorBase*>& outputs() const { return mOutputs; }
    std::vector<ConnectorBase*>& inputs()              { return mInputs;  }
    const std::vector<ConnectorBase*>& inputs() const  { return mInputs;  }

    bool isConnecting() const;
    bool isSelected() const { return mSelected; }
    void setSelected(bool selected) { mSelected = selected; }

    bool isActive() const { return mActive; }
    bool isHovered() const { return mHover; }
    
    void setMinSize(const Vec2f &s) { mMinSize = s; }
    // void setNextPos(const Vec2f &p) { mNextPos = p; }
    void setRect(const Rect2f &r) { mParams->rect = r; }
    void setPos(const Vec2f &p);
    void setSize(const Vec2f &s)  { mParams->rect.setSize(s); }
    const Rect2f& rect() const { return mParams->rect; }
    const Vec2f& pos() const   { return mParams->rect.p1; }
    Vec2f size() const         { return mParams->rect.size(); }

    void drawConnections(ImDrawList *drawList);
    bool draw(ImDrawList *drawList);
  };
}

#endif // NODE_HPP
