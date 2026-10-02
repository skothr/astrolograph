#ifndef FIELD_VIEW_NODE_HPP
#define FIELD_VIEW_NODE_HPP

#include "node.hpp"
#include "vector.hpp"

#include "shader.hpp"
#include "shapeBuffer.hpp"
#include "cudaField.hpp"


// forward declarations
class SettingForm;
  
//// node connector indices ////
// inputs
#define FIELDVIEWNODE_INPUT_FIELD 0
// outputs
#define FIELDVIEWNODE_OUTPUT_TEX  0
////////////////////////////////
  
class FieldViewNode : public Node
{
private:
  // node connectors
  static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return { new Connector<CudaFieldBase>("Field Input") }; } // TODO: R, G, B channels?
  static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { new Connector<CudaFieldTex>("Texture Output") }; }

  float        mTexMult    = 1.0f;
  //Vec2i        mTexSize;
  CudaFieldTex mTex;
    
  CudaFieldBase *mInputField = nullptr;
    
  SettingForm *mSettingForm   = nullptr;
  bool         mSettingsOpen  = false;
  Vec2f        mDisplaySize   = Vec2f(420, 420);
    
  Vec2f fieldToScreen(const Vec2f &fp, const Vec2f *p0=nullptr);
  Vec2f screenToField(const Vec2f &sp, const Vec2f *p0=nullptr);
    
  virtual void onUpdate() override;
  virtual void onDraw() override;
  virtual void onResize(const Vec2f &dSize) override;

  void resizeField(const Vec2f &fSize);
    
public:
  FieldViewNode();
  ~FieldViewNode();
  virtual std::string type() const { return "FieldViewNode"; }
};




//// node connector indices ////
// inputs
#define FIELDCHANNELVIEWNODE_INPUT_RFIELD 0
#define FIELDCHANNELVIEWNODE_INPUT_GFIELD 1
#define FIELDCHANNELVIEWNODE_INPUT_BFIELD 2
#define FIELDCHANNELVIEWNODE_INPUT_AFIELD 3
// outputs
#define FIELDCHANNELVIEWNODE_OUTPUT_TEX   0
////////////////////////////////
  
class FieldChannelViewNode : public Node
{
private:
  // node connectors
  static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return { new Connector<CudaFieldBase>("R Input"),
                                                                    new Connector<CudaFieldBase>("G Input"),
                                                                    new Connector<CudaFieldBase>("B Input"),
                                                                    new Connector<CudaFieldBase>("A Input") }; }
  static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { new Connector<CudaFieldTex>("Texture Output") }; }

  CudaFieldTex   mTex;
  CudaFieldBase *mRField = nullptr;
  CudaFieldBase *mGField = nullptr;
  CudaFieldBase *mBField = nullptr;
  CudaFieldBase *mAField = nullptr;

  Vec2f mRRange = Vec2f(0.0f, 1.0f);
  Vec2f mGRange = Vec2f(0.0f, 1.0f);
  Vec2f mBRange = Vec2f(0.0f, 1.0f);
  Vec2f mARange = Vec2f(0.0f, 1.0f);

  bool mSettingsOpen = false;
  Vec2f mDisplaySize = Vec2f(420, 420);
    
  Vec2f fieldToScreen(const Vec2f &fp, const Vec2f *p0=nullptr);
  Vec2f screenToField(const Vec2f &sp, const Vec2f *p0=nullptr);
    
  virtual void onUpdate() override;
  virtual void onDraw() override;
  virtual void onResize(const Vec2f &dSize) override;

  void resizeField(const Vec2f &fSize);
    
public:
  FieldChannelViewNode();
  ~FieldChannelViewNode();
  virtual std::string type() const { return "FieldChannelViewNode"; }
};


#endif // FIELD_VIEW_NODE_HPP
