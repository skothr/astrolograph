#ifndef FIELD_DEL_OPERATORS_HPP
#define FIELD_DEL_OPERATORS_HPP

#include "node.hpp"
#include "vector.hpp"

#include "shader.hpp"
#include "shapeBuffer.hpp"
#include "cudaField.hpp"

                 
namespace astro
{
  // FIELD GRADIENT //
  
  //// node connector indices ////
  // inputs
#define FIELDGRADIENTNODE_INPUT_FIELD  0
  // outputs
#define FIELDGRADIENTNODE_OUTPUT_FIELDX 0
#define FIELDGRADIENTNODE_OUTPUT_FIELDY 1
  ////////////////////////////////
  
  class FieldGradNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return { new Connector<CudaFieldBase>("Field Input")  }; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { new Connector<CudaFieldBase>("dF/dX Output"),
                                                                      new Connector<CudaFieldBase>("dF/dY Output")}; }

    CudaFieldBase *mInputField = nullptr;
    CudaFieldBase *mResultX    = nullptr; // (dF/dX)
    CudaFieldBase *mResultY    = nullptr; // (dF/dY)
    // Vec2i     mFSize;
    // FieldType mFType = FIELDTYPE_INVALID;
    
    virtual void onUpdate() override;
    virtual void onDraw() override;

    void resizeField(const Vec2f &fSize);
    
  public:
    FieldGradNode();
    ~FieldGradNode();
    virtual std::string type() const { return "FieldGradNode"; }
    };

  // FIELD DIVERGENCE //
  
  //// node connector indices ////
  // inputs
#define FIELDDIVNODE_INPUT_FIELD  0
  // outputs
#define FIELDDIVNODE_OUTPUT_FIELD 0
  ////////////////////////////////
  
  class FieldDivNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return { new Connector<CudaFieldBase>("Field Input")  }; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { new Connector<CudaFieldBase>("Divergence Output") }; }

    CudaFieldBase *mInputField = nullptr;
    CudaFieldBase *mResult    = nullptr;
    
    virtual void onUpdate() override;
    virtual void onDraw() override;

    void resizeField(const Vec2f &fSize);
    
  public:
    FieldDivNode();
    ~FieldDivNode();
    virtual std::string type() const { return "FieldDivNode"; }
    };


  // FIELD CURL //
  
  //// node connector indices ////
  // inputs
#define FIELDCURLNODE_INPUT_FIELD  0
  // outputs
#define FIELDCURLNODE_OUTPUT_FIELD 0
  ////////////////////////////////
  
  class FieldCurlNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return { new Connector<CudaFieldBase>("Field Input")  }; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { new Connector<CudaFieldBase>("Curl Output")}; }

    CudaFieldBase *mInputField = nullptr;
    CudaFieldBase *mResult    = nullptr; // (dF/dX)
    //CudaFieldBase *mResultY    = nullptr; // (dF/dY)
    // Vec2i     mFSize;
    // FieldType mFType = FIELDTYPE_INVALID;
    
    virtual void onUpdate() override;
    virtual void onDraw() override;

    void resizeField(const Vec2f &fSize);
    
  public:
    FieldCurlNode();
    ~FieldCurlNode();
    virtual std::string type() const { return "FieldCurlNode"; }
    };



  
  }




#endif // FIELD_DEL_OPERATORS_HPP
