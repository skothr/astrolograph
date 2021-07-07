#ifndef FIELD_OPERATORS_HPP
#define FIELD_OPERATORS_HPP

// nodes to apply various operators/processing to cuda fields and fluids

#include "node.hpp"
#include "vector.hpp"

#include "shader.hpp"
#include "shapeBuffer.hpp"
#include "cudaField.hpp"

                 
namespace astro
{
  // forward declarations
  class SettingForm;

  // FIELD MULTIPLY //
  
  //// node connector indices ////
  // inputs
#define FIELDMULTNODE_INPUT_FIELD1  0
#define FIELDMULTNODE_INPUT_FIELD2  1
  // outputs
#define FIELDMULTNODE_OUTPUT_FIELD  0
  ////////////////////////////////
  
  class FieldMultNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return { new Connector<CudaFieldBase>("Field Input 1"),
                                                                      new Connector<CudaFieldBase>("Field Input 2"), }; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { new Connector<CudaFieldBase>("Field Output"), }; }

    float mMult = 1.0f; // constant multiplier
    CudaFieldBase *mResultField = nullptr;
    
    Vec2i mF1Size; Vec2i mF2Size;
    FieldType mF1Type = FIELDTYPE_INVALID; FieldType mF2Type = FIELDTYPE_INVALID;
    
    SettingForm *mSettingForm     = nullptr;
    bool         mSettingsOpen    = false;
    
    virtual void onUpdate() override;
    virtual void onDraw() override;

    void resizeField(const Vec2f &fSize);
    
  public:
    FieldMultNode();
    ~FieldMultNode();
    virtual std::string type() const { return "FieldMultNode"; }
  };





  
  // FIELD ADD //
  
  //// node connector indices ////
  // inputs
#define FIELDADDNODE_INPUT_FIELD1  0
#define FIELDADDNODE_INPUT_FIELD2  1
  // outputs
#define FIELDADDNODE_OUTPUT_FIELD  0
  ////////////////////////////////
  
  class FieldAddNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return { new Connector<CudaFieldBase>("Field Input 1"),
                                                                      new Connector<CudaFieldBase>("Field Input 2"), }; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { new Connector<CudaFieldBase>("Field Output"), }; }

    float mAdd = 1.0f; // constant offset
    CudaFieldBase *mResultField = nullptr;
    
    Vec2i mF1Size; Vec2i mF2Size;
    FieldType mF1Type = FIELDTYPE_INVALID; FieldType mF2Type = FIELDTYPE_INVALID;
    
    SettingForm *mSettingForm     = nullptr;
    bool         mSettingsOpen    = false;
    
    virtual void onUpdate() override;
    virtual void onDraw() override;

    void resizeField(const Vec2f &fSize);
    
  public:
    FieldAddNode();
    ~FieldAddNode();
    virtual std::string type() const { return "FieldAddNode"; }
  };


  
  // FIELD NEGATION //
  
  //// node connector indices ////
  // inputs
#define FIELDNEGNODE_INPUT_FIELD  0
  // outputs
#define FIELDNEGNODE_OUTPUT_FIELD  0
  ////////////////////////////////
  
  class FieldNegNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return { new Connector<CudaFieldBase>("Field Input")  }; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { new Connector<CudaFieldBase>("Field Output") }; }

    CudaFieldBase   *mInputField  = nullptr;
    CudaFieldBase   *mResultField = nullptr;
    
    Vec2i     mFSize;
    FieldType mFType = FIELDTYPE_INVALID;
    
    virtual void onUpdate() override;
    virtual void onDraw() override;

    void resizeField(const Vec2f &fSize);
    
  public:
    FieldNegNode();
    ~FieldNegNode();
    virtual std::string type() const { return "FieldNegNode"; }
  };


  
  // FIELD ABSOLUTE VALUE //
  //// node connector indices ////
  // inputs
#define FIELDABSNODE_INPUT_FIELD  0
  // outputs
#define FIELDABSNODE_OUTPUT_FIELD  0
  ////////////////////////////////
  class FieldAbsNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return { new Connector<CudaFieldBase>("Field Input")  }; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { new Connector<CudaFieldBase>("Field Output") }; }
    CudaFieldBase *mResultField = nullptr;
    Vec2i     mFSize;
    FieldType mFType = FIELDTYPE_INVALID;    
    virtual void onUpdate() override;
    virtual void onDraw() override;
    void resizeField(const Vec2f &fSize);
  public:
    FieldAbsNode();
    ~FieldAbsNode();
    virtual std::string type() const { return "FieldAbsNode"; }
  };
  

  // FIELD LOG //
  //// node connector indices ////
  // inputs
#define FIELDLOGNODE_INPUT_FIELD   0
  // outputs
#define FIELDLOGNODE_OUTPUT_FIELD  0
  ////////////////////////////////
  class FieldLogNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return { new Connector<CudaFieldBase>("Field Input")  }; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { new Connector<CudaFieldBase>("Field Output") }; }
    CudaFieldBase *mResultField = nullptr;
    Vec2i     mFSize;
    FieldType mFType = FIELDTYPE_INVALID;
    virtual void onUpdate() override;
    virtual void onDraw() override;
    void resizeField(const Vec2f &fSize);
  public:
    FieldLogNode();
    ~FieldLogNode();
    virtual std::string type() const { return "FieldLogNode"; }
  };

  
  // FIELD EXP //
  //// node connector indices ////
  // inputs
#define FIELDEXPNODE_INPUT_FIELD   0
  // outputs
#define FIELDEXPNODE_OUTPUT_FIELD  0
  ////////////////////////////////
  class FieldExpNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return { new Connector<CudaFieldBase>("Field Input")  }; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { new Connector<CudaFieldBase>("Field Output") }; }
    CudaFieldBase *mResultField = nullptr;
    Vec2i     mFSize;
    FieldType mFType = FIELDTYPE_INVALID;
    virtual void onUpdate() override;
    virtual void onDraw() override;
    void resizeField(const Vec2f &fSize);
  public:
    FieldExpNode();
    ~FieldExpNode();
    virtual std::string type() const { return "FieldExpNode"; }
  };
  

  


  // FIELD MAXIMUM VALUE //
  
  //// node connector indices ////
  // inputs
#define FIELDMAXNODE_INPUT_FIELD  0
  // outputs
#define FIELDMAXNODE_OUTPUT_FIELD  0
  ////////////////////////////////
  
  class FieldMaxNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return { new Connector<CudaFieldBase>("Field Input")  }; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { new Connector<CudaFieldBase>("Field Output") }; }

    CudaFieldBase   *mInputField  = nullptr;
    CudaFieldBase   *mResultField = nullptr;
    CudaField<float> mMaxField;
    float            mMaxValue = 0.0;
    
    Vec2i     mFSize;
    FieldType mFType = FIELDTYPE_INVALID;
    
    virtual void onUpdate() override;
    virtual void onDraw() override;

    void resizeField(const Vec2f &fSize);
    
  public:
    FieldMaxNode();
    ~FieldMaxNode();
    virtual std::string type() const { return "FieldMaxNode"; }
  };

  
  
  // FIELD NORMALIZATION VALUE //
  
  //// node connector indices ////
  // inputs
#define FIELDNORMNODE_INPUT_FIELD   0
  // outputs
#define FIELDNORMNODE_OUTPUT_FIELD  0
  ////////////////////////////////
  
  class FieldNormNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return { new Connector<CudaFieldBase>("Field Input")  }; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { new Connector<CudaFieldBase>("Field Output") }; }

    CudaFieldBase   *mInputField  = nullptr;
    CudaFieldBase   *mResultField = nullptr;
    CudaField<float> mNormField;
    float            mAvgLength = 0.0;
    
    Vec2i     mFSize;
    FieldType mFType = FIELDTYPE_INVALID;
    
    virtual void onUpdate() override;
    virtual void onDraw() override;

    void resizeField(const Vec2f &fSize);
    
  public:
    FieldNormNode();
    ~FieldNormNode();
    virtual std::string type() const { return "FieldNormNode"; }
  };


  
}



#endif //FIELD_OPERATORS_HPP
