#ifndef NODE_HPP
#define NODE_HPP

#include "astro.hpp"
#include "vector.hpp"
#include "typelessMap.hpp"

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
    
  public:
    Vec2f screenPos;
    
    ConnectorBase(std::string name="")
      : mName(name) { mThisPtr = this; }
    virtual ~ConnectorBase() { disconnectAll(); }
    virtual std::string type() const = 0;

    Node* parent()     { return mParent; } // returns parent node
    int conId() const  { return mConId; }  // returns connector index in parent node
    void setParent(Node *n, int cId) { mParent = n; mConId = cId; }
    
    // template<typename T>
    // T* getDefault() { return ((Connector<T>*)this)->getDefault(); }
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
    void reset(bool rs=true)
    {
      mNeedReset = rs;
      if(rs && (mDirection & CONNECTOR_INPUT))
        { for(auto con : mConnected) { con->reset(rs); } }
      else if(!rs && (mDirection & CONNECTOR_OUTPUT))
        { for(auto con : mConnected) { con->reset(rs); } }
    }
    
    void draw();
    void drawConnections();
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
      if(mDirection == CONNECTOR_INPUT)
        { return (mConnected.size() > 0 ? ((Connector<T>*)mConnected[0])->mData : mData); }
      else
        { return mData; }
    }
    void set(T *data)
    {
      mData = data;
      if(mDirection == CONNECTOR_INPUT && mConnected.size() > 0) { ((Connector<T>*)mConnected[0])->mData = data; }
    }
  };

  // base class for Node parameters that should be saved/loaded
  struct NodeParams
  {
    int         id   = -1;
    std::string name = "";
    Vec2f       pos  = Vec2f(0,0);
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
    
    Vec2f mNextPos = Vec2f(0,0);  // node window pos
    Vec2f mMinSize = Vec2f(0,0);  // min size (to be set by child class)
    
    std::vector<ConnectorBase*> mInputs;
    std::vector<ConnectorBase*> mOutputs;

    // override in child classes to draw node
    virtual bool onDraw() { return true; }

    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const { return params; }
    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params)       { return params; }
    
  public:
    static int NEXT_ID;
    static std::unordered_set<int> ACTIVE_IDS;
    
    Node(const std::vector<ConnectorBase*> &inputs_, const std::vector<ConnectorBase*> &outputs_, const std::string &name="", NodeParams *params=nullptr)
      : mInputs(inputs_), mOutputs(outputs_), mParams(params)
    {
      if(!mParams) { mParams = new NodeParams(); }
      mParams->name = name;
      for(int i = 0; i < mInputs.size(); i++)  { mInputs[i]->setParent(this, i);  mInputs[i]->setDirection(CONNECTOR_INPUT); }
      for(int i = 0; i < mOutputs.size(); i++) { mOutputs[i]->setParent(this, i); mOutputs[i]->setDirection(CONNECTOR_OUTPUT); }

      if(mParams->id < 0)
        {
          while(ACTIVE_IDS.count(NEXT_ID) > 0) { NEXT_ID++; } // no id overlap
          mParams->id = NEXT_ID++;
          ACTIVE_IDS.emplace(mParams->id);
        }
    }
    virtual ~Node()
    {
      for(auto c : mInputs)  { delete c; }
      for(auto c : mOutputs) { delete c; }
      ACTIVE_IDS.erase(mParams->id);
      // if(mParams->id == NEXT_ID-1) { NEXT_ID--; }
      delete mParams;
    }
    virtual std::string type() const = 0;

    void setGraph(NodeGraph *graph) { mGraph = graph; }
    

    static std::map<std::string, std::string> getSaveHeader(const std::string &saveStr)
    {
      std::map<std::string, std::string> header;
      std::istringstream ss(saveStr);
      std::string temp;
      ss >> temp;
      ss.ignore(2, '{');
      std::string name = "";
      std::string value = "";
      do
        {
          ss >> name;
          ss.ignore(2, ':');
          ss >> std::quoted(value);
          ss.ignore(2, ',');

          if(name == "nodeType" || name == "nodeName" || name == "nodeId" || name == "nodePos")
            { header.emplace(name, value); }
        } while(!name.empty() && name != "\n" && name.find("}") == std::string::npos && value.find("}") == std::string::npos);
      
      return header;
    }
    
    std::string toSaveString() const
    {
      std::map<std::string, std::string> params;
      params.emplace("nodeType", type());
      params.emplace("nodeName", mParams->name);
      params.emplace("nodeId",   std::to_string(id()));
      params.emplace("nodePos",  mParams->pos.toString());
      getSaveParams(params);
      
      std::ostringstream ss;
      ss << "NODE { ";// << type() << ": " << std::quoted(mParams->name) << " " << id() << " " << mParams->pos;
      for(auto &p : params)
        {
          ss << p.first << " : " << std::quoted(p.second) << ", ";
        }
      ss << "}";
      return ss.str();
    }

    // returns remaining string after base class parameters
    std::string fromSaveString(const std::string &saveStr)
    {
      // convert string to params
      std::map<std::string, std::string> params;
      std::istringstream ss(saveStr);
      std::string temp, nodeType;
      ss >> temp; // "NODE {"
      ss.ignore(2, '{');
      std::string name, value;
      do
        {
          ss >> name;
          ss.ignore(2, ':');
          ss >> std::quoted(value);
          ss.ignore(2, ',');
          if(!name.empty() && name != "\n" && name.find("}") == std::string::npos && value.find("}") == std::string::npos)
            {
              std::cout << "     -->  READ PARAM '" << name << "' : '" << value << "'\n";
              params.emplace(name, value);
            }
        } while(!name.empty() && name != "\n" && name.find("}") == std::string::npos && value.find("}") == std::string::npos);
      
      // erase current id
      if(mParams->id >= 0)
        { ACTIVE_IDS.erase(mParams->id); }

      // load stored values
      ss.str(params["nodeType"]); ss.clear();
      ss >> nodeType;
      ss.str(params["nodeName"]); ss.clear();
      ss >> mParams->name;
      ss.str(params["nodeId"]); ss.clear();
      ss >> mParams->id;
      mNextPos.fromString(params["nodePos"]);

      setSaveParams(params);
      
      // check active id
      if(ACTIVE_IDS.count(mParams->id) > 0)
        { std::cout << "WARNING: Overlapping Node ID! (fromSaveString()) --> " << mParams->id << "\n"; }
      ACTIVE_IDS.emplace(mParams->id);
      // while(ACTIVE_IDS.count(NEXT_ID) > 0) { NEXT_ID++; } // no id overlap
      
      // std::stringstream tmp; tmp << ss.rdbuf();
      // std::string remaining = tmp.str();
      
      // return remaining;
      return "";
    }
    
    int id() const { return mParams->id; }
    // std::string& name() { return mParams->name; }
    const std::string& name() const { return mParams->name; }
    
    std::vector<ConnectorBase*>& outputs() { return mOutputs; }
    const std::vector<ConnectorBase*>& outputs() const { return mOutputs; }
    std::vector<ConnectorBase*>& inputs()  { return mInputs; }
    const std::vector<ConnectorBase*>& inputs() const  { return mInputs; }

    bool isSelected() const { return mSelected; }
    
    void setMinSize(const Vec2f &s) { mMinSize = s; }

    void setNextPos(const Vec2f &p) { mNextPos = p; }
    Vec2f pos() const  { return mParams->pos; }
    // Vec2f size() const { return mSize; }
    
    bool draw();
    bool drawOutputs();
    bool drawInputs();
  };
}

#endif // NODE_HPP
