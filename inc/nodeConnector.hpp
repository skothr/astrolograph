#ifndef NODE_CONNECTOR_HPP
#define NODE_CONNECTOR_HPP

#include <vector>
#include <iomanip>
#include <sstream>
#include <unordered_set>
#include <map>
#include "nlohmann/json_fwd.hpp" // json forward declarations
using json = nlohmann::json;
#define JSON_SPACES 4

#include "astro.hpp"
#include "rect.hpp"

// connector params
#define CONNECTOR_SIZE          Vec2f(18.0f, 36.0f)
#define CONNECTOR_POINT_RADIUS  4.0f
#define CONNECTOR_PADDING       Vec2f(10.0f, 10.0f)
#define CONNECTOR_PROTRUDE      (Vec2f(CONNECTOR_SIZE.x/2,0) + Vec2f(CONNECTOR_PADDING.x,0) + Vec2f(12.0f, 0.0f)) // vector from connection center to start point
#define CONNECTOR_ROUNDING      5.0f
#define CONNECTOR_SEGMENT_ERROR 0.1f

// forward declarations
struct ImDrawList;

namespace astro
{
  // direction of node connector (for now just input/output)
  enum Direction
    {
      CONNECTOR_INVALID = -1,
      CONNECTOR_INPUT = 0,
      CONNECTOR_OUTPUT // TODO: refine (e.g. enforce data directionality)
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
  class SettingBase;
  
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
}


#endif // NODE_CONNECTOR_HPP
