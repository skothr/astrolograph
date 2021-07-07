#ifndef MANDELBROT_NODE_HPP
#define MANDELBROT_NODE_HPP

#include <thread>

#include "node.hpp"
#include "vector.hpp"

#include "shader.hpp"
#include "shapeBuffer.hpp"
#include "cudaField.hpp"

namespace quantum
{
  typedef int FieldType;
  enum FieldType_
    {
     FIELD_MANDELBROT=0, // complex field based on the Mandelbrot Set
     FIELD_FLUID,
    };
  static const std::vector<std::string> FIELD_NAMES = {  "mandelbrot", "fluid" };
}



namespace astro
{
  // forward declarations
  class SettingForm;
  class FileDialog;

  //// node connector indices ////
  // inputs
  //#define FIELDNODE_INPUT_      0
  // outputs
#define MANDELBROTNODE_OUTPUT_TEX   0
#define MANDELBROTNODE_OUTPUT_FIELD 1
  ////////////////////////////////

  class MandelbrotNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return {}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { new Connector<CudaFieldTex>("Texture Output"),
                                                                      new Connector<CudaFieldBase>("Field Output") }; }

    FileDialog *mFileDialog = nullptr;
    std::thread mSaveThread;
    std::string mSavePath   = "";
    bool        mSaving     = false;
    double      mSaveTime   = -1.0;
    
    // std::string mSaveFileName = "";
    Vec2i             mFieldSize = Vec2i(512, 512);
    CudaField<float2> mField;
    CudaFieldTex      mFieldTex;
    
    Vec2f mDisplaySize = Vec2f(690, 690);

    bool mFieldHovered = false;
    bool mFieldClicked = false;
    
    SettingForm *mSettingForm  = nullptr;
    SettingForm *mContextForm  = nullptr;
    bool         mSettingsOpen = false;
    bool         mHollow       = true;
    bool         mDrawPath     = false;
    
    bool handleIO(const Vec2f &cp);
    
    Vec2f fieldToScreen(const Vec2f &fp, const Vec2f *p0=nullptr);
    Vec2f screenToField(const Vec2f &sp, const Vec2f *p0=nullptr);
    
    virtual void onUpdate() override;
    virtual void onDraw() override;
    virtual void onResize(const Vec2f &dSize) override;

    void resizeField(const Vec2f &fSize);
    
  public:
    MandelbrotNode();
    ~MandelbrotNode();
    virtual std::string type() const { return "MandelbrotNode"; }
  };
}



#endif // MANDELBROT_NODE_HPP
