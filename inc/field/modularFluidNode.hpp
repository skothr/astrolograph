#ifndef MODULAR_FLUID_NODE_HPP
#define MODULAR_FLUID_NODE_HPP

#include "node.hpp"
#include "vector.hpp"

#include "shader.hpp"
#include "shapeBuffer.hpp"
#include "cudaField.hpp"

namespace astro
{
  // forward declarations
  class SettingForm;

  //// node connector indices ////
  // inputs
  //#define FLUIDNODE_INPUT_      0
  // outputs
#define FLUIDSTATE_OUTPUT_FLUID    0
#define FLUIDSTATE_OUTPUT_VXFIELD  1
#define FLUIDSTATE_OUTPUT_VYFIELD  2
#define FLUIDSTATE_OUTPUT_DFIELD   3
#define FLUIDSTATE_OUTPUT_PFIELD   4
#define FLUIDSTATE_OUTPUT_WVFIELD  5
  ////////////////////////////////

  class FluidStateNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return {}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
    { return { new Connector<CudaFluid<float>>("Fluid Output"),
               new Connector<CudaFieldBase>("VX Output"),      new Connector<CudaFieldBase>("VY Output"),
               new Connector<CudaFieldBase>("Density Output"), new Connector<CudaFieldBase>("Pressure Output"),
               new Connector<CudaFieldBase>("Wave Vector Output") }; }
    
    CudaFluid<float> mFluid;
    CudaFieldTex     mFluidTex;
    Vec2i mFieldSize   = Vec2i(512, 512);
    
    bool mFillCircle   = false;
    bool mPhysics      = true;
    bool mStepOnce     = false;
    bool mFieldHovered = false;
    bool mFieldClicked = false;
    
    bool mSettingsOpen    = false;
    
    bool mDrawVectorField = false;
    Vec2i mVSampleSpacing = Vec2i();
    
    ForceType mFType = FLUIDFORCE_PUSH | FLUIDFORCE_CW | FLUIDFORCE_OUT;
    float mTimeStep  = 0.01f;
    float mGravity   = 0.0f;
    bool  mApplyChaos   = true;
    // external/manual mouse force multipliers
    float mMForceRad = 0.075f; // radius
    float mPushVMult = 1.0f;   // push velocity mult
    float mOutVMult  = 1.0f;   // out velocity mult
    float mInVMult   = 1.0f;   // in velocity mult
    float mCwVMult   = 1.0f;   // cw velocity mult
    float mCcwVMult  = 1.0f;   // ccw velocity mult
    float mDMult     = 0.1f;   // density mult
    float mPMult     = 0.1f;   // pressure mult
    float mWVMult    = 0.15f;  // CW/CCW wave vector mult
    // active mouse forces
    bool mFPush     = false;
    bool mFOut      = false;
    bool mFIn       = false;
    bool mFCW       = false;
    bool mFCCW      = true;
    bool mFDensity  = false;
    bool mFPressure = false;
    
    Vec2f mDisplaySize = Vec2f(690, 690);
    
    void resizeField(const Vec2i &fSize);
    void clearField(const Vec4f &color);

    bool handleIO(const Vec2f &cp);
    
    Vec2f fieldToScreen(const Vec2f &fp, const Vec2f *p0=nullptr);
    Vec2f screenToField(const Vec2f &sp, const Vec2f *p0=nullptr);
    
    virtual void onUpdate() override;
    virtual void onDraw() override;
    virtual void onResize(const Vec2f &dSize) override;
    
  public:
    FluidStateNode();
    ~FluidStateNode();
    virtual std::string type() const { return "FluidStateNode"; }
  };
}



#endif // MODULAR_FLUID_NODE_HPP
