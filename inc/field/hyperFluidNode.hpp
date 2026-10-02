#ifndef HYPER_FLUID_NODE_HPP
#define HYPER_FLUID_NODE_HPP

#include <chrono>
#include <mutex>
#include <thread>

#include "node.hpp"
#include "vector.hpp"
#include "shader.hpp"
#include "shapeBuffer.hpp"
#include "cudaField.hpp"
#include "hyper-field.hpp"
#include "cudaField.hpp"

#define CLOCK std::chrono::high_resolution_clock
#define SHIFT_STEP_DELAY 0.2   // timestep multiplier when shift is held with arrow key
#define CTRL_SPEED_MULT  10.0  // timestep multiplier when shift is held with arrow key
#define ALT_SPEED_MULT   100.0 // timestep multiplier when shift is held with arrow key

// forward declarations
class SettingForm;
//class FileDialog;

//// node connector indices ////
// inputs
// outputs
#define HFLUIDNODE_OUTPUT_VELFIELD 0
#define HFLUIDNODE_OUTPUT_DIVFIELD 1
#define HFLUIDNODE_OUTPUT_DFIELD   2
#define HFLUIDNODE_OUTPUT_PFIELD   3
#define HFLUIDNODE_OUTPUT_WVFIELD  4
#define HFLUIDNODE_OUTPUT_TEXTURE  5

class HyperFluidNode : public Node
{
private:
  // node connectors
  static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return {}; }
  template<int N>
  static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { new Connector<HyperField<float2, N>>("Velocity Output"),
                                                                    new Connector<HyperField<float,  N>>("Divergence Output"),
                                                                    new Connector<HyperField<float,  N>>("Density Output"),
                                                                    new Connector<HyperField<float,  N>>("Pressure Output"),
                                                                    new Connector<HyperField<float2, N>>("Wave Vector Output"),
                                                                    new Connector<CudaFieldTex>         ("Texture Output"),
    }; }
  
  int mDims   = 3;
  HyperFluid<2> *m2Fluid1 = nullptr;
  HyperFluid<2> *m2Fluid2 = nullptr;
  HyperFluid<3> *m3Fluid1 = nullptr;
  HyperFluid<3> *m3Fluid2 = nullptr;
  CudaFieldTex mFluidTex;
  CudaFieldTex mDisplayTex;
  bool mTexUpdate = false;  

  int mZLevel = 0;
  Vec3d  mCamPos   = Vec3d(-5.0, 0.0, 0.0);
  Vec3d  mCamDir   = Vec3d(1.0,  0.0, 0.0);
  Vec3d  mCamUp    = Vec3d(0.0,  0.0, 1.0);
  Vec3d  mCamRight = Vec3d(0.0, -1.0, 0.0);
  double mCamFov   = 60.0; // (degrees)

  Vec3d mFPos  = Vec3d(-0.5, -0.5, -0.5);
  Vec3d mFSize = Vec3d( 1.0,  1.0,  1.0);

  bool mDrawAxes = false;
    
  Vec3i mFluidSize        = Vec3i(128, 128, 128);
  bool mSettingsOpen      = false;
  bool mFillCircle        = false;
  bool mCircleBounded     = false;
  bool mPhysics           = true;
  bool mManualStep        = false;
  bool mStepOnce          = false;
  bool mFieldHovered      = false;
  bool mFieldClicked      = false;
  bool mFieldLeftClicked  = false;
  bool mFieldRightClicked = false;
    
  float mTimeStep    = 0.01f;
  bool mDensityPattern = false;

  CLOCK::time_point mLastStepT = CLOCK::now();

  bool mRenderSlice = false;
  int  mSliceDim    = 0; // 0 --> x, 1 --> y, 2 --> z
  int  mSliceIndex  = 0;

  FluidRenderType mRenderR = HF_RENDER_V | HF_RENDER_DSCALE;// | HF_RENDER_VNORM;
  FluidRenderType mRenderG = HF_RENDER_V | HF_RENDER_DSCALE; // | HF_RENDER_VNORM;
  FluidRenderType mRenderB = HF_RENDER_V | HF_RENDER_DSCALE; // | HF_RENDER_VNORM;
    
  bool  mDrawVectorField = true;
  Vec2i mVSamples    = Vec2i(32, 32);
  Vec4f mVColor      = Vec4f(1.0f, 1.0f, 1.0f, 0.5f);
  Vec4f mVBColor     = Vec4f(0.0f, 0.0f, 0.0f, 0.5f);
  Vec4f mVColorLast  = Vec4f(1.0f, 1.0f, 1.0f, 0.5f);
  Vec4f mVBColorLast = Vec4f(0.0f, 0.0f, 0.0f, 0.5f);
  float mVMult       = 1.0f;
  bool  mVConst      = true;
  float mVOpacity    = 1.0f;
  float mVWidth      = 2.0f;
  float mVBWidth     = 1.0f;
    
  ForceType mFType   = FLUIDFORCE_PUSH | FLUIDFORCE_CW | FLUIDFORCE_OUT;
  float mGravity     = 0.0f;
  bool  mApplyChaos  = false;
  float mChaos       = 0.01f;   // wave vector chaotic calculation multplier
  bool  mApplyVisc   = false;
  float mViscosity   = 0.01f;   // viscosity multplier
  bool  mIncompressible = true; // if true, velocity will be projected to conserve mass (incompressible fluid)
  int   mDiffuseRad  = 0;       // diffusion kernel radius
  int   mProjectIter = 40;      // projection iterations
  float mWVMult      = 0.1f;    // wave vector mult (mouse force)
    
  // external/manual mouse force multipliers
  float mMForceRad = 0.075f; // radius
  float mPushVMult = 1.0f;   // push velocity mult
  float mOutVMult  = 1.0f;   // out velocity mult
  float mInVMult   = 1.0f;   // in velocity mult
  float mCwVMult   = 1.0f;   // cw velocity mult
  float mCcwVMult  = 1.0f;   // ccw velocity mult
  float mDMult     = 0.1f;   // density mult
  float mPMult     = 0.1f;   // pressure mult
  // active mouse forces
  bool mFPush     = false;
  bool mFOut      = false;
  bool mFIn       = false;
  bool mFCW       = false;
  bool mFCCW      = true;
  bool mFDensity  = false;
  bool mFPressure = false;
  bool mFWv       = true;
    
  Vec2f mDisplaySize = Vec2i(512, 512); // graph space
  Vec2i mTexSize     = Vec2i(512, 512); // texture size

  std::mutex mTexLock;

  bool        mOfflineRender = false;
  // FileDialog *mFileDialog = nullptr;
  // std::thread mSaveThread;
  // bool        mSaving     = false;
  std::string mPrefix      = "";
  std::string mSavePath    = "";
  int         mTotalFrames = 1;
  int         mSaveFrame   = 0;
  double      mSaveTime    = -1.0;
  
  bool setDims(int dims);
  void resizeField(const Vec3i &fSize);
  void clearField(const Vec4f &color);

  bool handleIO(const Vec2f &cp);
    
  Vec2f fieldToScreen(const Vec2f &fp, const Vec2f *p0=nullptr);
  Vec2f screenToField(const Vec2f &sp, const Vec2f *p0=nullptr);
    
  virtual void onUpdate() override;
  virtual void onDraw() override;
  virtual void onResize(const Vec2f &dSize) override;
    
public:
  HyperFluidNode();
  ~HyperFluidNode();
  virtual std::string type() const { return "HyperFluidNode"; }
};


#endif // HYPER_FLUID_NODE_HPP
