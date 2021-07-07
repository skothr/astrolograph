#ifndef FIELD_CHANNELS_HPP
#define FIELD_CHANNELS_HPP

#include "node.hpp"
#include "cudaField.hpp"

namespace astro
{
  // FIELD CHANNEL COMBINE //
  
  //// node connector indices ////
//   // inputs
// #define FIELDCHANNELNODE_INPUT_FIELD  0
//   // outputs
#define FIELDCOMBINENODE_OUTPUT_FIELD  0
// #define FIELDCHANNELNODE_OUTPUT_FIELDY 1
//   ////////////////////////////////
  
  class ChannelCombineNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return { new Connector<CudaFieldBase>("X Field")  }; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { new Connector<CudaFieldBase>("Field") }; }

    std::vector<std::string> mTypeChoices;
    int mInputChoice    = (int)FIELDTYPE_FLOAT2;
    // int mOutputChoice   = 0;
    int mInputChannels  = 2;
    // int mOutputChannels = 1;
    
    CudaFieldBase *mInputFieldX = nullptr;
    CudaFieldBase *mInputFieldY = nullptr;
    CudaFieldBase *mInputFieldZ = nullptr;
    CudaFieldBase *mInputFieldW = nullptr;
    CudaFieldBase *mResult      = nullptr;
    // CudaFieldBase *mResultY     = nullptr;
    // CudaFieldBase *mResultZ     = nullptr;
    // CudaFieldBase *mResultW     = nullptr;
    
    virtual void onUpdate() override;
    virtual void onDraw() override;
    virtual void onLoad() override;

    void resizeField(const Vec2f &fSize);
    
  public:
    ChannelCombineNode();
    ~ChannelCombineNode();
    virtual std::string type() const { return "ChannelCombineNode"; }
    };



  // FIELD CHANNEL SPLIT //
  
  //// node connector indices ////
//   // inputs
// #define FIELDCHANNELNODE_INPUT_FIELD  0
//   // outputs
#define FIELDSPLITNODE_OUTPUT_FIELD  0
// #define FIELDCHANNELNODE_OUTPUT_FIELDY 1
//   ////////////////////////////////
  
  class ChannelSplitNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return { new Connector<CudaFieldBase>("Field")  }; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { new Connector<CudaFieldBase>("X Field") }; }

    std::vector<std::string> mTypeChoices;
    //int mInputChoice    = 0;
    int mOutputChoice   = (int)FIELDTYPE_FLOAT2;
    //int mInputChannels  = 1;
    int mOutputChannels = 2;
    
    CudaFieldBase *mInputField = nullptr;
    // CudaFieldBase *mInputFieldY = nullptr;
    // CudaFieldBase *mInputFieldZ = nullptr;
    // CudaFieldBase *mInputFieldW = nullptr;
    CudaFieldBase *mResultX     = nullptr;
    CudaFieldBase *mResultY     = nullptr;
    CudaFieldBase *mResultZ     = nullptr;
    CudaFieldBase *mResultW     = nullptr;
    
    virtual void onUpdate() override;
    virtual void onDraw() override;
    virtual void onLoad() override;

    void resizeField(const Vec2f &fSize);
    
  public:
    ChannelSplitNode();
    ~ChannelSplitNode();
    virtual std::string type() const { return "ChannelSplitNode"; }
  };

  
}


#endif // FIELD_CHANNELS_HPP
