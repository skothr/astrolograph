#ifndef FFT_NODE_HPP
#define FFT_NODE_HPP

#include "node.hpp"
#include "vector.hpp"

#include "shader.hpp"
#include "shapeBuffer.hpp"
#include "cudaField.hpp"

#include <cufft.h>          // CUDA FFT Libraries


namespace astro
{
  // forward declarations
  class SettingForm;
  
  //// node connector indices ////
  // inputs
  #define FFTNODE_INPUT_FIELD   0
  // outputs
  // #define FFTNODE_OUTPUT_TEX  0
  #define FFTNODE_OUTPUT_FIELD  0
  // #define FFTNODE_OUTPUT_FFTFIELD  2
  ////////////////////////////////

  enum RenderType { RENDER_REAL = 0, RENDER_IMAGINARY, RENDER_MAGNITUDE, RENDER_PHASE, RENDER_COMBINED };
  static const std::vector<std::string> RENDER_TYPE_NAMES {{"Real values", "Imaginary values", "Complex Magnitude", "Complex Phase", "Magnitude+Phase" }};

  class FFTNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return { new Connector<CudaFieldBase>("Field Input")  }; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { //new Connector<CudaFieldTex>("Texture Output"),
                                                                      new Connector<CudaFieldBase>("Field Output") }; }
    std::string mSaveFileName = "";
    
    Vec2i mInputSize = Vec2i(512, 512);
    CudaField<float2> mField;    //
    CudaField<float2> mFftField; // used to store initial FFT output data (before shifting)
    // CudaFieldTex      mFieldTex; //
    
    cufftHandle mFftPlan;
    RenderType  mRenderType = RENDER_MAGNITUDE;
    bool        mDebug      = false;
    bool        mInverseFft = false;
    bool        mPreShift   = false;
    bool        mPreScale   = false;
    bool        mPostShift  = true;
    bool        mPostScale  = true;
    float       mMult    = 1.0f;
    bool        mHollow     = true;
    
    SettingForm *mSettingForm  = nullptr;
    bool         mSettingsOpen = false;
    Vec2f        mDisplaySize  = Vec2f(690, 690);
    
    virtual void onUpdate() override;
    virtual void onDraw() override;
    virtual void onResize(const Vec2f &dSize) override;

    void resizeField(const Vec2f &fSize);
    
  public:
    FFTNode();
    ~FFTNode();
    virtual std::string type() const { return "FFTNode"; }
  };
}



#endif // FFT_NODE_HPP
