#ifndef STATIC_FIELD_NODE_HPP
#define STATIC_FIELD_NODE_HPP

#include "node.hpp"
#include "vector.hpp"

// #include "shader.hpp"
// #include "shapeBuffer.hpp"
#include "cudaField.hpp"

namespace astro
{
  // forward declarations
  //class SettingForm;

  

  
  //// node connector indices ////
  // inputs
  //#define FIELDNODE_INPUT_      0
  // outputs
#define STATICFIELDNODE_OUTPUT_FIELD 0
  ////////////////////////////////

  class StaticFieldNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return {}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { new Connector<CudaFieldBase>("Field Output") }; }

    Vec2i             mFieldSize = Vec2i(512, 512);
    CudaField<float2> mField;
    
    // CudaFieldTex mFieldTex;
    Vec2f mDisplaySize = Vec2f(690, 690);

    bool mFieldHovered = false;
    bool mFieldClicked = false;
    
    //SettingForm *mSettingForm  = nullptr;
    //SettingForm *mContextForm  = nullptr;
    bool         mSettingsOpen = false;
    
    bool handleIO(const Vec2f &cp);
    
    Vec2f fieldToScreen(const Vec2f &fp, const Vec2f *p0=nullptr);
    Vec2f screenToField(const Vec2f &sp, const Vec2f *p0=nullptr);
    
    virtual void onUpdate() override;
    virtual void onDraw() override;
    virtual void onResize(const Vec2f &dSize) override;

    void resizeField(const Vec2f &fSize);
    
  public:
    StaticFieldNode();
    ~StaticFieldNode();
    virtual std::string type() const { return "StaticFieldNode"; }
  };
}

#endif // STATIC_FIELD_NODE_HPP
