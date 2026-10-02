#include "hyperFluidNode.hpp"

#include <imgui.h>

#include "matrix.hpp"
#include "setting.hpp"
#include "settingForm.hpp"
#include "fileDialog.hpp"
#include "glfwKeys.hpp"
#include "complex.hpp"
#include "imtools.hpp"
#include "nodeGraph.hpp"
#include "vector-operators.h"

#define SETTING_LABEL_W 150.0f
#define SETTING_INPUT_W 255.0f


HyperFluidNode::HyperFluidNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS<3>(), "HyperFluid Node", true)
    //, mFileDialog(new FileDialog())
{
  // settings
  mSettings.push_back(new Setting<bool>           ("Settings Open", "settingsOpen", &mSettingsOpen));
  SettingBase *fSizeSetting   = new Setting<Vec3i>("Field Size",    "fSize",        &mFluidSize,  mFluidSize);
  ((Setting<Vec3i>*)fSizeSetting)->setFormat(128, 256, "%d");
  mSettings.push_back(fSizeSetting);
  mSettings.push_back(new Setting<Vec2f>("Display Size",  "dispSize",     &mDisplaySize));
  mSettings.push_back(new Setting<Vec2i>("Tex Size",      "texSize",      &mTexSize));
  
  mSettings.push_back(new Setting<Vec3d> ("Pos", "camPos", &mCamPos));
  mSettings.push_back(new Setting<Vec3d> ("Dir", "camDir", &mCamDir));
  mSettings.push_back(new Setting<double>("FOV", "fov",    &mCamFov));

  // physics settings
  mSettings.push_back(new Setting<bool> ("Fill Circle",       "fillCircle",   &mFillCircle,  mFillCircle));
  mSettings.push_back(new Setting<bool> ("Physics",           "physics",      &mPhysics,     mPhysics   ));
  mSettings.push_back(new Setting<float>("Time Step",         "timeStep",     &mTimeStep,    mTimeStep  ));
  mSettings.push_back(new Setting<float>("MForce Radius",     "forceRadius",  &mMForceRad,   mMForceRad ));
  mSettings.push_back(new Setting<float>("Push VMult",        "vfPush",       &mPushVMult,   mPushVMult ));
  mSettings.push_back(new Setting<float>("Out VMult",         "vfOut",        &mOutVMult,    mOutVMult  ));
  mSettings.push_back(new Setting<float>("In VMult",          "vfIn",         &mInVMult,     mInVMult   ));
  mSettings.push_back(new Setting<float>("Cw VMult",          "vfCw",         &mCwVMult,     mCwVMult   ));
  mSettings.push_back(new Setting<float>("Ccw VMult",         "vfCcw",        &mCcwVMult,    mCcwVMult  ));
  mSettings.push_back(new Setting<float>("MForce DMult",      "df",           &mDMult,       mDMult     ));
  mSettings.push_back(new Setting<float>("MForce PMult",      "pf",           &mPMult,       mPMult     ));
  mSettings.push_back(new Setting<float>("MForce WVMult",     "wvf",          &mWVMult,      mWVMult    ));
  mSettings.push_back(new Setting<float>("Gravity",           "gravity",      &mGravity,     mGravity   ));
  mSettings.push_back(new Setting<bool> ("Apply Chaos",       "applyChaos",   &mApplyChaos,  mApplyChaos));
  mSettings.push_back(new Setting<bool> ("Apply Viscosity",   "applyVisc",    &mApplyVisc,   mApplyVisc ));
  
  mSettings.push_back(new Setting<float>("Chaos",             "chaos",        &mChaos,       mChaos     ));
  mSettings.push_back(new Setting<float>("Viscosity",         "viscosity",    &mViscosity,   mViscosity ));
  // force settings
  mSettings.push_back(new Setting<bool> ("Push",     "pushForce",     &mFPush,     mFPush    ));
  mSettings.push_back(new Setting<bool> ("Out",      "outForce",      &mFOut,      mFOut     ));
  mSettings.push_back(new Setting<bool> ("In",       "inForce",       &mFIn,       mFIn      ));
  mSettings.push_back(new Setting<bool> ("CW",       "cwForce",       &mFCW,       mFCW      ));
  mSettings.push_back(new Setting<bool> ("CCW",      "ccwForce",      &mFCCW,      mFCCW     ));
  mSettings.push_back(new Setting<bool> ("Density",  "densityForce",  &mFDensity,  mFDensity ));
  mSettings.push_back(new Setting<bool> ("Pressure", "pressureForce", &mFPressure, mFPressure));
  // vector field
  mSettings.push_back(new Setting<bool> ("Vector Field",      "drawVField",   &mDrawVectorField, mDrawVectorField ));
  mSettings.push_back(new Setting<Vec4f>("VColor",            "vColor",       &mVColor,          mVColor          ));
  mSettings.push_back(new Setting<float>("VMult",             "vMult",        &mVMult,           mVMult           ));
  mSettings.push_back(new Setting<bool> ("VConst",            "vConst",       &mVConst,          mVConst          ));
  mSettings.push_back(new Setting<float>("VectorW",           "vWidth",       &mVWidth,          mVWidth          ));
  mSettings.push_back(new Setting<float>("BorderW",           "bWidth",       &mVBWidth,         mVBWidth         ));
  mSettings.push_back(new Setting<float>("VOpacity",          "vOpacity",     &mVOpacity,        mVOpacity        ));

  resizeField(mFluidSize);
  {
    //std::lock_guard<std::mutex> lock(mTexLock);
    mFluidTex.create(mTexSize); mDisplayTex.create(mTexSize);
    outputs()[HFLUIDNODE_OUTPUT_VELFIELD]->set(&m3Fluid1->vel);
    outputs()[HFLUIDNODE_OUTPUT_WVFIELD ]->set(&m3Fluid1->div);
    outputs()[HFLUIDNODE_OUTPUT_DFIELD  ]->set(&m3Fluid1->d);
    outputs()[HFLUIDNODE_OUTPUT_PFIELD  ]->set(&m3Fluid1->p);
    outputs()[HFLUIDNODE_OUTPUT_WVFIELD ]->set(&m3Fluid1->wv);
    outputs()[HFLUIDNODE_OUTPUT_TEXTURE ]->set(&mFluidTex);
  }

  setTitle("Hyper Fluid");
  setMinSize(Vec2f(512, 512));
}

HyperFluidNode::~HyperFluidNode()
{
  if(m2Fluid1) { m2Fluid1->destroy(); delete m2Fluid1; }
  if(m2Fluid2) { m2Fluid2->destroy(); delete m2Fluid2; }
  if(m3Fluid1) { m3Fluid1->destroy(); delete m3Fluid1; }
  if(m3Fluid2) { m3Fluid2->destroy(); delete m3Fluid2; }
  mFluidTex.destroy();
  
  //if(mFileDialog) { delete mFileDialog; }
}

bool HyperFluidNode::setDims(int dims)
{
  if(dims != mDims)
    {
      mDims = dims;
      Node::clearOutputs();
      Node::addOutputs((mDims == 2) ? CONNECTOR_OUTPUTS<2>() : CONNECTOR_OUTPUTS<3>());
      
      if(mDims == 2)
        {
          outputs()[HFLUIDNODE_OUTPUT_VELFIELD]->set(&m2Fluid1->vel);
          outputs()[HFLUIDNODE_OUTPUT_DIVFIELD]->set(&m2Fluid1->div);
          outputs()[HFLUIDNODE_OUTPUT_DFIELD  ]->set(&m2Fluid1->d);
          outputs()[HFLUIDNODE_OUTPUT_PFIELD  ]->set(&m2Fluid1->p);
          outputs()[HFLUIDNODE_OUTPUT_WVFIELD ]->set(&m2Fluid1->wv);
        }
      else if(mDims == 3)
        {
          outputs()[HFLUIDNODE_OUTPUT_VELFIELD]->set(&m3Fluid1->vel);
          outputs()[HFLUIDNODE_OUTPUT_DIVFIELD]->set(&m3Fluid1->div);
          outputs()[HFLUIDNODE_OUTPUT_DFIELD  ]->set(&m3Fluid1->d);
          outputs()[HFLUIDNODE_OUTPUT_PFIELD  ]->set(&m3Fluid1->p);
          outputs()[HFLUIDNODE_OUTPUT_WVFIELD ]->set(&m3Fluid1->wv);
        }
      return true;
    }
  else
    { return false; }
}


void HyperFluidNode::resizeField(const Vec3i &fSize)
{
  Vec2i fSize2(fSize.x, fSize.y);
  if(mDims == 2)
    {
      if(fSize.x > 0 && fSize.y > 0)
        {
          Vec2i old(mFluidSize.x, mFluidSize.y);
          if(!m2Fluid1 || !m2Fluid2 || fSize2 != old || fSize2 != m2Fluid1->size || fSize2 != m2Fluid2->size)
            {
              {
                //std::lock_guard<std::mutex> lock(mTexLock);
                mFluidSize = fSize;
                if(!m2Fluid1) { m2Fluid1 = new HyperFluid<2>(); } m2Fluid1->create(fSize2); // TODO: other dimensionalities
                if(!m2Fluid2) { m2Fluid2 = new HyperFluid<2>(); } m2Fluid2->create(fSize2);
              }
              clearField(Vec4f(0, 0, 0, 1));
            }
        }
    }
  else if(mDims == 3)
    {
      if(fSize.x > 0 && fSize.y > 0 && fSize.z > 0)
        {
          if(!m3Fluid1 || !m3Fluid2 || fSize != mFluidSize || fSize != m3Fluid1->size || fSize != m3Fluid2->size)
            {
              {
                //std::lock_guard<std::mutex> lock(mTexLock);
                mFluidSize = fSize;
                if(!m3Fluid1) { m3Fluid1 = new HyperFluid<3>(); } m3Fluid1->create(mFluidSize); // TODO: other dimensionalities
                if(!m3Fluid2) { m3Fluid2 = new HyperFluid<3>(); } m3Fluid2->create(mFluidSize);
              }
              clearField(Vec4f(0, 0, 0, 1));
            }
        }
    }
}


void HyperFluidNode::clearField(const Vec4f &color)
{
  //std::lock_guard<std::mutex> lock(mTexLock);
  
  if(mDims == 2)      { if(m2Fluid1) { clearHFluid2(*m2Fluid1); } if(m2Fluid2) { clearHFluid2(*m2Fluid2); } }
  else if(mDims == 3) { if(m3Fluid1) { clearHFluid3(*m3Fluid1); } if(m3Fluid2) { clearHFluid3(*m3Fluid2); } }

  if(mFillCircle)
    {
      if(mDims == 2)      { if(m2Fluid1) { fillHFluidCircle2(*m2Fluid1); } }
      else if(mDims == 3) { if(m3Fluid1) { fillHFluidCircle3(*m3Fluid1, mCircleBounded); } }
    }
  if(mDensityPattern)
    {
      if(mDims == 2)      { if(m2Fluid1) { fillHFluidPattern2(*m2Fluid1); } }
      else if(mDims == 3) { if(m3Fluid1) { fillHFluidPattern3(*m3Fluid1); } }
    }

  if(mTexSize != mFluidTex.size) { mFluidTex.create(mTexSize); mDisplayTex.create(mTexSize); }
  mFluidTex.map();   fillTex(mFluidTex.dData,   mFluidTex.size.x,   mFluidTex.size.y,   float4{color.x,color.y,color.z,color.w}); mFluidTex.unmap();
  mDisplayTex.map(); fillTex(mDisplayTex.dData, mDisplayTex.size.x, mDisplayTex.size.y, float4{color.x,color.y,color.z,color.w}); mDisplayTex.unmap();
}

bool HyperFluidNode::handleIO(const Vec2f &p0)
{
  // io / control
  ImGuiIO &io  = ImGui::GetIO();
  float scale  = getScale();
  bool blocked = isBlocked();
  bool changed = false;
  bool ctrlDown = io.KeyCtrl;
  Vec2f mp = ImGui::GetMousePos();
  Vec2f gp = screenToField(mp, &p0);

  mFieldHovered &= !blocked && !mPlacing && !mClicked;
  
  if((mFieldLeftClicked || (mFieldHovered && !ctrlDown)) && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    { mFieldLeftClicked = true; mActive = true; }
  else if(ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    { mFieldLeftClicked = false; mActive = true; }

  if((mFieldRightClicked || (mFieldHovered && !ctrlDown)) && ImGui::IsMouseDown(ImGuiMouseButton_Right))
    { mFieldRightClicked = true; mActive = true; }
  else if(ImGui::IsMouseReleased(ImGuiMouseButton_Right))
    { mFieldRightClicked = false; mActive = true; }

  
  if(mFieldHovered)
    {
      // reset hyperfluid with ESCAPE
      if(ImGui::IsKeyPressed(GLFW_KEY_ESCAPE) && !ctrlDown) { clearField(Vec4f(0.0f, 0.0f, 0.0f, 1.0f)); }
      // toggle physics with SPACE 
      if(ImGui::IsKeyPressed(GLFW_KEY_SPACE)) { mPhysics = !mPhysics; }

      //// TODO: Context menu
      // std::string contextName = "##plotContext";
      // if(BeginContext(contextName, CONTEXT_WINDOW_RCLICK, mFieldHovered))
      //   {
      //     mActive = true;
      //     mContextForm->draw(1.0f, false);
      //     EndContext(true);
      //   }
      // else
      //   {
      //     EndContext(false);
      //     if(mFieldHovered)
      //       {
      //         // draw tooltip
      //         BeginTooltip();
      //         {
      //           Vec2f tp = (mp - p0) / scale + Vec2f(0.5, 0.5); // position in texture
      //           if(tp.x >= 0.0 && tp.x < mFluid.vx.x && tp.y >= 0.0 && tp.y < mFluid.size.y)
      //             {
      //               int i = ((int)tp.y)*mFluid.vx.size.x + (int)tp.x;
      //               Vec2f lp(f.hData[i].x, mField.hData[i].y);
      //               ImGui::Text("Position:    < %s%.8f + %s%.8f i >", (gp.x < 0.0 ? "-" : " "), abs(gp.x), (gp.y < 0.0 ? "-" : " "), abs(gp.y));
      //               ImGui::Text("Final Value: < %s%.8f + %s%.8f i >", (lp.x < 0.0 ? "-" : " "), abs(lp.x), (lp.y < 0.0 ? "-" : " "), abs(lp.y));
      //             }
      //           else
      //             { ImGui::Text("< N/A >"); }
      //         }
      //         EndTooltip();
      //       }
      //   }

      // scroll
      if(!io.KeyCtrl && io.MouseWheel != 0.0f)
        { // CAMERA DISTANCE
          double dist = mCamPos.length();
          mCamPos *= (1.0f - io.MouseWheel/20.0f);
        }
    }

  // dragging
  if(mFieldRightClicked && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
    { // PAN
      Vec2f dmp = -Vec2f(ImGui::GetMouseDragDelta(ImGuiMouseButton_Right));
      ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);

      Vec3d up(0,0,1);
      Vec3d right = normalize(cross(mCamDir, up));
      up = normalize(cross(right, mCamDir));
      mCamPos += right*dmp.x/mDisplaySize.x + up*dmp.y/mDisplaySize.y;
      mCamRight = right;
      mCamUp    = up;
      changed = true;
    }
  // rotating
  if(mFieldLeftClicked && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    { // ROTATE
      Vec2f dmp = -Vec2f(ImGui::GetMouseDragDelta(ImGuiMouseButton_Left));
      ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);

      double lrRotate = dmp.x/mDisplaySize.x;
      double upRotate = -dmp.y/mDisplaySize.y;

      if(abs(mCamUp.z) <= 0.05 && ((upRotate < 0 ? -1 : 1) != (mCamPos.z < 0 ? -1 : 1))) { upRotate = 0.0; }

      Vec3d up(0,0,1);
      Vec3d newDir = normalize(lrRotate == 0.0 ? mCamDir : rotate(normalize(mCamDir), Vec3d(0,0,1), lrRotate));
      Vec3d right  = normalize(cross(newDir, up));
      up           = normalize(cross(right, newDir));
      newDir       = normalize(upRotate == 0.0 ? newDir  : rotate(newDir, right, upRotate));
      
      Vec3d newPos = normalize(lrRotate == 0.0 ? mCamPos : rotate(normalize(mCamPos), Vec3d(0,0,1), lrRotate));
      Vec3d r      = normalize(cross(newPos, Vec3d(0,0,1)));
      newPos       = normalize(upRotate == 0.0 ? newPos  : rotate(newPos, r, -upRotate));
      mCamDir   = newDir;
      mCamRight = right;
      mCamUp    = up;
      mCamPos   = newPos*length(mCamPos);
      changed = true;
    }

  return changed;
}

Vec2f HyperFluidNode::fieldToScreen(const Vec2f &fp, const Vec2f *p0)
{
  float scale = getScale();
  Vec2f sp = (Vec2f(fp.x, fp.y) * Vec2f(mFluidSize.x, mFluidSize.y))*scale;
  if(p0) { sp += *p0; }
  return (sp + (p0 ? (*p0) : Vec2f(0,0)));
}

Vec2f HyperFluidNode::screenToField(const Vec2f &sp, const Vec2f *p0)
{
  float scale = getScale();
  Vec2f tp = (sp - (p0 ? *p0 : Vec2f())) / scale / mDisplaySize * Vec2f(mFluidSize.x, mFluidSize.y);
  return Vec2f((tp.x/(float)mFluidSize.x), (tp.y/(float)mFluidSize.y));
}

void HyperFluidNode::onUpdate()
{
  ImGuiIO &io = ImGui::GetIO();
  bool ctrlDown  = io.KeyCtrl;  // (ImGui::IsKeyDown(GLFW_KEY_LEFT_CONTROL) || ImGui::IsKeyDown(GLFW_KEY_RIGHT_CONTROL));
  bool shiftDown = io.KeyShift; // (ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT)   || ImGui::IsKeyDown(GLFW_KEY_RIGHT_SHIFT));
  bool altDown   = io.KeyAlt;   // (ImGui::IsKeyDown(GLFW_KEY_LEFT_ALT)     || ImGui::IsKeyDown(GLFW_KEY_RIGHT_ALT));

  // step physics or set time direction with arrow keys
  bool   firstPress = !mManualStep;
  bool   repeat     = firstPress;
  double delay      = SHIFT_STEP_DELAY;
  double ctrlMult   = 1.0f; double altMult  = 1.0f;
  double keyMult    = (ctrlDown ? CTRL_SPEED_MULT : 1.0f) * (altDown ? ALT_SPEED_MULT : 1.0f);

  mManualStep = false;
  float tsMult = 1.0f;
  if(mFieldHovered      && ImGui::IsKeyDown(GLFW_KEY_RIGHT)) { mManualStep = true; tsMult = 1.0f;  }
  else if(mFieldHovered && ImGui::IsKeyDown(GLFW_KEY_LEFT))  { mManualStep = true; tsMult = -1.0f; }
  else if(!mManualStep) { repeat = false; firstPress = false; }

  if(mManualStep && (mFieldHovered && mPhysics)) { } //tsMult *= keyMult; }
  else if(mManualStep)                           { delay  /= keyMult; }
  
  if(mManualStep && shiftDown)
    {
      auto now  = CLOCK::now();
      double dt = std::chrono::duration_cast<std::chrono::nanoseconds>(now - mLastStepT).count() / 1000000000.0;
      if(dt >= delay) { repeat = true; mLastStepT = now; }
    }
  else
    {
      mLastStepT = CLOCK::now();
      if(mManualStep) { repeat = true; }
    }
  
  bool stepping = mPhysics || mStepOnce || repeat;
  
  {
    //std::lock_guard<std::mutex> lock(mTexLock);
    if(mTexSize != mFluidTex.size) { mFluidTex.create(mTexSize); mDisplayTex.create(mTexSize); }
  }

  // RENDER TO TEXTURE //
  if(mDims == 2 && m2Fluid1)
    {
      {
        //std::lock_guard<std::mutex> lock(mTexLock);
        
        m2Fluid1->params.dt          = mTimeStep*tsMult;
        m2Fluid1->params.gravity     = mGravity;
        m2Fluid1->params.applyChaos  = mApplyChaos;
        m2Fluid1->params.applyChaos  = mApplyVisc;
        m2Fluid1->params.chaos       = mChaos;
        m2Fluid1->params.viscosity   = mViscosity;
        m2Fluid1->params.forceRad    = mMForceRad;
        m2Fluid1->params.diffuseRad  = mDiffuseRad;
        m2Fluid1->params.projectIter = mProjectIter;

        m2Fluid1->params.mdown = mFieldClicked;
        ForceType ftype = FLUIDFORCE_NONE;
        if(mFPush   ) { ftype |= FLUIDFORCE_PUSH;    }
        if(mFOut    ) { ftype |= FLUIDFORCE_OUT;     } if(mFIn      ) { ftype |= FLUIDFORCE_IN;       }
        if(mFCW     ) { ftype |= FLUIDFORCE_CW;      } if(mFCCW     ) { ftype |= FLUIDFORCE_CCW;      }
        if(mFDensity) { ftype |= FLUIDFORCE_DENSITY; } if(mFPressure) { ftype |= FLUIDFORCE_PRESSURE; }
        if(mFWv)      { ftype |= FLUIDFORCE_WV; }

        m2Fluid1->params.forceRad = mMForceRad;  
        m2Fluid1->params.ftype    = ftype;
        m2Fluid1->params.vfPush   = mPushVMult;
        m2Fluid1->params.vfOut    = mOutVMult;
        m2Fluid1->params.vfIn     = mInVMult;
        m2Fluid1->params.vfCw     = mCwVMult;
        m2Fluid1->params.vfCcw    = mCcwVMult;
        m2Fluid1->params.df       = mDMult;
        m2Fluid1->params.pf       = mPMult;
        m2Fluid1->params.wvf      = mWVMult;
      
        m2Fluid2->params = m2Fluid1->params;
        if(m2Fluid1->size != Vec2f(mFluidSize.x, mFluidSize.y)) { resizeField(mFluidSize); }
      
        if(stepping)
          {
            mStepOnce = false;
            addForcesHyper(*m2Fluid1, *m2Fluid2);                     std::swap(m2Fluid2, m2Fluid1);
            diffuseHyper(*m2Fluid1, *m2Fluid2);                       std::swap(m2Fluid2, m2Fluid1);
            // if(mIncompressible) { projectHyper(*m2Fluid1, *m2Fluid2); std::swap(m2Fluid2, m2Fluid1); }
            advectHyper(*m2Fluid1, *m2Fluid2);                        std::swap(m2Fluid2, m2Fluid1);
            if(mIncompressible) { projectHyper(*m2Fluid1, *m2Fluid2); std::swap(m2Fluid2, m2Fluid1); }
          }
        else
          { // add manual mouse forces, but prevent gravity
            m2Fluid1->params.dt = 0.0;       m2Fluid2->params.dt = 0.0;
            addForcesHyper(*m2Fluid1, *m2Fluid2); std::swap(m2Fluid2, m2Fluid1);
            m2Fluid1->params.dt = mTimeStep*tsMult; m2Fluid2->params.dt = mTimeStep*tsMult;
          }
        // renderHFluid2(*m2Fluid1, mFluidTex);
        //}

        //cudaDeviceSynchronize();
      //{
      //std::lock_guard<std::mutex> lock(mTexLock);
        // mFluidTex.copyTo(mDisplayTex);
        mTexUpdate = true;
        
        outputs()[HFLUIDNODE_OUTPUT_VELFIELD]->set(&m2Fluid1->vel);
        outputs()[HFLUIDNODE_OUTPUT_DIVFIELD]->set(&m2Fluid1->div);
        outputs()[HFLUIDNODE_OUTPUT_DFIELD  ]->set(&m2Fluid1->d);
        outputs()[HFLUIDNODE_OUTPUT_PFIELD  ]->set(&m2Fluid1->p);
        outputs()[HFLUIDNODE_OUTPUT_WVFIELD ]->set(&m2Fluid1->wv);
        // outputs()[HFLUIDNODE_OUTPUT_TEXTURE ]->set(&mDisplayTex);
      }
    }
  else if(mDims == 3 && m3Fluid1)
    {
      {
        //std::lock_guard<std::mutex> lock(mTexLock);
        m3Fluid1->params.dt          = mTimeStep*tsMult;
        m3Fluid1->params.gravity     = mGravity;
        m3Fluid1->params.applyChaos  = mApplyChaos;
        m3Fluid1->params.chaos       = mChaos;
        m3Fluid1->params.viscosity   = mViscosity;
        m3Fluid1->params.forceRad    = mMForceRad;
        m3Fluid1->params.diffuseRad  = mDiffuseRad;
        m3Fluid1->params.projectIter = mProjectIter;

        m3Fluid1->params.mdown = mFieldClicked;
        ForceType ftype = FLUIDFORCE_NONE;
        if(mFPush   ) { ftype |= FLUIDFORCE_PUSH;    }
        if(mFOut    ) { ftype |= FLUIDFORCE_OUT;     } if(mFIn      ) { ftype |= FLUIDFORCE_IN;       }
        if(mFCW     ) { ftype |= FLUIDFORCE_CW;      } if(mFCCW     ) { ftype |= FLUIDFORCE_CCW;      }
        if(mFDensity) { ftype |= FLUIDFORCE_DENSITY; } if(mFPressure) { ftype |= FLUIDFORCE_PRESSURE; }
        if(mFWv)      { ftype |= FLUIDFORCE_WV; }

        m3Fluid1->params.forceRad = mMForceRad;  
        m3Fluid1->params.ftype    = ftype;
        m3Fluid1->params.vfPush   = mPushVMult;
        m3Fluid1->params.vfOut    = mOutVMult;
        m3Fluid1->params.vfIn     = mInVMult;
        m3Fluid1->params.vfCw     = mCwVMult;
        m3Fluid1->params.vfCcw    = mCcwVMult;
        m3Fluid1->params.df       = mDMult;
        m3Fluid1->params.pf       = mPMult;
        m3Fluid1->params.wvf      = mWVMult;

        m3Fluid2->params = m3Fluid1->params;
        if(m3Fluid1->size != mFluidSize) { resizeField(mFluidSize); }
      
        if(stepping)
          {
            mStepOnce = false;
            if(mIncompressible) { projectHyper3(*m3Fluid1, *m3Fluid2); std::swap(m3Fluid2, m3Fluid1); }
            advectHyper3(*m3Fluid1, *m3Fluid2);                        std::swap(m3Fluid2, m3Fluid1);
            if(mIncompressible) { projectHyper3(*m3Fluid1, *m3Fluid2); std::swap(m3Fluid2, m3Fluid1); }
            
            diffuseHyper3(*m3Fluid1, *m3Fluid2);                       std::swap(m3Fluid2, m3Fluid1);
            addForcesHyper3(*m3Fluid1, *m3Fluid2);                     std::swap(m3Fluid2, m3Fluid1);
          }
        else
          { // add manual mouse forces, but prevent gravity
            m3Fluid1->params.dt = 0.0; m3Fluid2->params.dt = 0.0;
            addForcesHyper3(*m3Fluid1, *m3Fluid2); std::swap(m3Fluid2, m3Fluid1);
            m3Fluid1->params.dt = mTimeStep*tsMult; m3Fluid2->params.dt = mTimeStep*tsMult;
          }

        // if(mRenderSlice)
        //   {
        //     renderHFluid3Slice(*m3Fluid1, mFluidTex, to_cuda(mFPos), to_cuda(mFSize),
        //                        to_cuda(mCamPos), to_cuda(mCamDir), to_cuda(mCamUp), to_cuda(mCamRight), mCamFov*(M_PI/180),
        //                        mRenderR, mRenderG, mRenderB,
        //                        mSliceDim, mSliceIndex);
        //   }
        // else
        //   {
        //     renderHFluid3(*m3Fluid1, mFluidTex, to_cuda(mFPos), to_cuda(mFSize),
        //                   to_cuda(mCamPos), to_cuda(mCamDir), to_cuda(mCamUp), to_cuda(mCamRight), mCamFov*(M_PI/180),
        //                   mRenderR, mRenderG, mRenderB);
        //   }
        //}

        // cudaDeviceSynchronize();
        //std::lock_guard<std::mutex> lock(mTexLock);
        mTexUpdate = true;
        
        //mFluidTex.copyTo(mDisplayTex);
        //std::swap(mFluidTex.dData, mDisplayTex.dData);
        // mDisplayTex.map(); mFluidTex.map();
        // cudaMemcpy(mDisplayTex.dData, mFluidTex.dData, mFluidTex.dataSize, cudaMemcpyDeviceToDevice);
        // mDisplayTex.unmap(); mFluidTex.unmap();
      
        outputs()[HFLUIDNODE_OUTPUT_VELFIELD]->set(&m3Fluid1->vel);
        outputs()[HFLUIDNODE_OUTPUT_DIVFIELD]->set(&m3Fluid1->div);
        outputs()[HFLUIDNODE_OUTPUT_DFIELD  ]->set(&m3Fluid1->d);
        outputs()[HFLUIDNODE_OUTPUT_PFIELD  ]->set(&m3Fluid1->p);
        outputs()[HFLUIDNODE_OUTPUT_WVFIELD ]->set(&m3Fluid1->wv);
        // outputs()[HFLUIDNODE_OUTPUT_TEXTURE ]->set(&mDisplayTex);
      }
    }

  if(mTexUpdate)
    {
      // std::lock_guard<std::mutex> lock(mTexLock);
      mTexUpdate = false;
      
      if(mDims == 2 && m2Fluid1)
        { renderHFluid2(*m2Fluid1, mFluidTex); }
      else if(mDims == 3 && m3Fluid1)
        {
          if(mRenderSlice)
            {
              renderHFluid3Slice(*m3Fluid1, mFluidTex, to_cuda(mFPos), to_cuda(mFSize),
                                 to_cuda(mCamPos), to_cuda(mCamDir), to_cuda(mCamUp), to_cuda(mCamRight), mCamFov*(M_PI/180),
                                 mRenderR, mRenderG, mRenderB,
                                 mSliceDim, mSliceIndex);
            }
          else
            {
              renderHFluid3(*m3Fluid1, mFluidTex, to_cuda(mFPos), to_cuda(mFSize),
                            to_cuda(mCamPos), to_cuda(mCamDir), to_cuda(mCamUp), to_cuda(mCamRight), mCamFov*(M_PI/180),
                            mRenderR, mRenderG, mRenderB);
            }
        }
      
      if(mOfflineRender)
        {
          std::cout << "Frame " << mSaveFrame << "...\n";
          
          std::stringstream ss;
          ss << mSavePath << "-" << std::setfill('0') << std::setw(4) << mSaveFrame << ".hdr";
          std::cout << std::setfill(' ');
          std::string savePath = ss.str();

          // pull data from GPU
          auto t0 = std::chrono::high_resolution_clock::now();
          mFluidTex.pullData();
          auto t1 = std::chrono::high_resolution_clock::now();
          std::cout << " --> Pull Time: " << (std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count() / 1000000000.0) << "\n";

          // write to file
          t0 = std::chrono::high_resolution_clock::now();
          writeTexture(savePath, &mFluidTex);
          t1 = std::chrono::high_resolution_clock::now();
          mSaveTime = (std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count() / 1000000000.0);
          std::cout << " --> Save Time: " << (std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count() / 1000000000.0) << "\n";
          
          mSaveFrame++;
          if(mSaveFrame >= mTotalFrames) { mOfflineRender = false; }
        }      
    }
}

void HyperFluidNode::onDraw()
{  
  bool  blocked = isBlocked();
  float scale   = getScale();
  float inputW  = 150.0f*scale;
  
  ImGui::BeginGroup();
  {
    ImGui::BeginGroup();
    // settings collapsing header
    ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth);
    ImGui::SetNextTreeNodeOpen(mSettingsOpen);
    if(ImGui::CollapsingHeader("Settings", nullptr, flags))
      {
        mSettingsOpen = true;

        ImGui::BeginGroup();
        {
          ImGui::BeginGroup();
          {
            ImGui::TextUnformatted("Field");
            ImGui::Indent();

            ImGui::TextUnformatted("Dimensions"); ImGui::SameLine();
            ImGui::SetNextItemWidth(75.0f*scale);
            if(ImGui::InputInt("##fDims", &mDims, 1, 1) && setDims(mDims)) { resizeField(mFluidSize); }
            ImGui::Spacing();

            int maxFluidSize = 256*256*256;
            
            ImGui::TextUnformatted("FX"); ImGui::SameLine();
            ImGui::SetNextItemWidth(100.0f*scale);
            if(ImGui::InputInt("##fieldSX", &mFluidSize.x, 8, 32))
              {
                mFluidSize.x = std::max(0, mFluidSize.x);
                if(mFluidSize.x*mFluidSize.y*mFluidSize.z > maxFluidSize) { mFluidSize.x = maxFluidSize / (mFluidSize.y*mFluidSize.z); }
                resizeField(mFluidSize);
              }
            if(mDims >= 2)
              {
                ImGui::SameLine(); ImGui::TextUnformatted("FY"); ImGui::SameLine();
                ImGui::SetNextItemWidth(100.0f*scale);
                if(ImGui::InputInt("##fieldSY", &mFluidSize.y, 8, 32))
                  {
                    mFluidSize.y = std::max(0, mFluidSize.y);
                    if(mFluidSize.x*mFluidSize.y*mFluidSize.z > maxFluidSize) { mFluidSize.y = maxFluidSize / (mFluidSize.x*mFluidSize.z); }
                    resizeField(mFluidSize);
                  }
              }
            if(mDims >= 3)
              {
                ImGui::SameLine(); ImGui::TextUnformatted("FZ"); ImGui::SameLine();
                ImGui::SetNextItemWidth(100.0f*scale);
                if(ImGui::InputInt("##fieldSZ", &mFluidSize.z, 8, 32))
                  {
                    mFluidSize.z = std::max(0, mFluidSize.z);
                    if(mFluidSize.x*mFluidSize.y*mFluidSize.z > maxFluidSize) { mFluidSize.z = maxFluidSize / (mFluidSize.x*mFluidSize.y); }
                    resizeField(mFluidSize);
                  }
              }

            // display size
            ImGui::TextUnformatted("TX"); ImGui::SameLine();
            ImGui::SetNextItemWidth(100.0f*scale);
            if(ImGui::InputInt("##viewX", &mTexSize.x, 128, 256))
              {
                //std::lock_guard<std::mutex> lock(mTexLock);
                mTexSize.x = std::max(1, std::min(4096, mTexSize.x));
                mFluidTex.create(mTexSize); mDisplayTex.create(mTexSize);
              }
            ImGui::SameLine(); 
            ImGui::TextUnformatted("TY"); ImGui::SameLine();
            ImGui::SetNextItemWidth(100.0f*scale);
            if(ImGui::InputInt("##viewY", &mTexSize.y, 128, 256))
              {
                //std::lock_guard<std::mutex> lock(mTexLock);
                mTexSize.y = std::max(1, std::min(4096, mTexSize.y));
                mFluidTex.create(mTexSize); mDisplayTex.create(mTexSize);
              }
            
            if(mDims == 3)
              { // 3D camera
                ImGui::BeginGroup();
                {
                  ImGui::TextUnformatted("Pos");
                  ImGui::SameLine(); ImGui::SetNextItemWidth(100.0f*scale); ImGui::InputDouble("##camPosX", &mCamPos.x, 0.1, 1.0, "%.1f");
                  ImGui::SameLine(); ImGui::SetNextItemWidth(100.0f*scale); ImGui::InputDouble("##camPosY", &mCamPos.y, 0.1, 1.0, "%.1f");
                  ImGui::SameLine(); ImGui::SetNextItemWidth(100.0f*scale); ImGui::InputDouble("##camPosZ", &mCamPos.z, 0.1, 1.0, "%.1f");
                  ImGui::SetNextItemWidth(250.0f*scale);
                  ImGui::TextUnformatted("Dir");
                  bool changed = false;
                  ImGui::SameLine(); ImGui::SetNextItemWidth(100.0f*scale); changed |= ImGui::InputDouble("##camDirX", &mCamDir.x, 0.01, 0.1, "%.2f");
                  ImGui::SameLine(); ImGui::SetNextItemWidth(100.0f*scale); changed |= ImGui::InputDouble("##camDirY", &mCamDir.y, 0.01, 0.1, "%.2f");
                  ImGui::SameLine(); ImGui::SetNextItemWidth(100.0f*scale); changed |= ImGui::InputDouble("##camDirZ", &mCamDir.z, 0.01, 0.1, "%.2f");
                  if(changed) { mCamDir.normalize(); }
                }
                ImGui::EndGroup();
                ImGui::SameLine(); ImGui::BeginGroup();
                {
                  double dist = mCamPos.length();
                  ImGui::SetNextItemWidth(100.0f*scale);
                  if(ImGui::InputDouble("Dist", &dist, 0.1, 1.0, "%.1f"))
                    { mCamPos = mCamPos.normalized() * dist; }
                  ImGui::SetNextItemWidth(100.0f*scale);
                  ImGui::InputDouble("FOV", &mCamFov, 0.1, 1.0, "%.1f");
                }
                ImGui::EndGroup();
              }
            
            ImGui::Unindent();
          }
          ImGui::EndGroup();
          
          ImGui::BeginGroup();
          {
            ImGui::TextUnformatted("Physics");
            ImGui::Indent();
            if(ImGui::Button("Reset")) { clearField(Vec4f(0.0f, 0.0f, 0.0f, 1.0f)); }
            ImGui::SameLine(); ImGui::BeginGroup();
            {
              ImGui::Checkbox("Circle",         &mFillCircle);
              ImGui::Checkbox("Circle Bounded", &mCircleBounded);
            }
            ImGui::EndGroup();
            ImGui::SameLine(); ImGui::Checkbox("Checker", &mDensityPattern);
            if(ImGui::Button("Step")) { mStepOnce = true; }
            ImGui::SameLine(); ImGui::Checkbox("Physics", &mPhysics);
            ImGui::SetNextItemWidth(inputW);
            if(ImGui::InputFloat("Time Step", &mTimeStep, 0.01f, 0.1f, "%.8f")) { if(mDims == 3) { m3Fluid1->params.dt = mTimeStep; } }
            ImGui::SetNextItemWidth(inputW);
            if(ImGui::InputFloat("Chaos##mult",     &mChaos,    0.01f, 0.1f, "%.8f"))  { if(mDims == 3) { m3Fluid1->params.chaos = mChaos; } }
            ImGui::SameLine(); if(ImGui::Checkbox("##chaosApply", &mApplyChaos))       { if(mDims == 3) { m3Fluid1->params.applyChaos = mApplyChaos; } }
            ImGui::SetNextItemWidth(inputW);
            if(ImGui::InputFloat("Viscosity##mult",     &mViscosity,    0.01f, 0.1f, "%.8f"))  { if(mDims == 3) { m3Fluid1->params.viscosity = mViscosity; } }
            ImGui::SameLine(); if(ImGui::Checkbox("##viscApply", &mApplyVisc)) { if(mDims == 3) { m3Fluid1->params.applyVisc = mApplyVisc; } }
            ImGui::SetNextItemWidth(inputW);
            if(ImGui::InputInt("Diffuse Radius", &mDiffuseRad, 1, 2))
              { mDiffuseRad = std::max(0, mDiffuseRad); if(mDims == 3) { m3Fluid1->params.diffuseRad = mDiffuseRad; } }
            
            ImGui::TextUnformatted("Incompressible");
            ImGui::SameLine(); ImGui::SetNextItemWidth(inputW);
            ImGui::Checkbox("##incompress", &mIncompressible);
            ImGui::SameLine();
            ImGui::TextUnformatted("Iterations");
            ImGui::SameLine(); ImGui::SetNextItemWidth(100.0f*scale);
            if(ImGui::InputInt("##projectIter", &mProjectIter, 1, 2))
             { mProjectIter = std::max(0, mProjectIter); if(mDims == 3) { m3Fluid1->params.projectIter = mProjectIter; } }
            ImGui::Unindent();
          }
          ImGui::EndGroup();

          ImGui::SameLine(); ImGui::BeginGroup();
          {
            ImGui::TextUnformatted("Forces");
            ImGui::Indent();
            ImGui::SetNextItemWidth(inputW);
            if(ImGui::InputFloat("Gravity",   &mGravity, 0.01f, 0.1f, "%.8f"))  { if(mDims == 3) { m3Fluid1->params.gravity = mGravity; } }
            ImGui::Unindent();
          }
          ImGui::EndGroup();

          if(mDims == 3)
            {
              ImGui::BeginGroup();
              {
                ImGui::TextUnformatted("Rendering");
                ImGui::Indent();
                
                ImGui::TextUnformatted("Draw Slice");
                ImGui::SameLine(); ImGui::Checkbox("##rSlice", &mRenderSlice);
                if(mRenderSlice)
                  {
                    ImGui::SameLine(); bool checked = (mSliceDim == 0); if(ImGui::Checkbox("X", &checked) && checked) { mSliceDim = 0; }
                    ImGui::SameLine(); checked      = (mSliceDim == 1); if(ImGui::Checkbox("Y", &checked) && checked) { mSliceDim = 1; }
                    ImGui::SameLine(); checked      = (mSliceDim == 2); if(ImGui::Checkbox("Z", &checked) && checked) { mSliceDim = 2; }
                    ImGui::SameLine(); ImGui::TextUnformatted("Index");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(100.0f*scale);
                    int maxIndex = (mSliceDim == 0 ? mFluidSize.x : (mSliceDim == 1 ? mFluidSize.y : mFluidSize.z)) - 1;
                    if(ImGui::InputInt("##sliceIndex", &mSliceIndex, 1, maxIndex/8))
                      { mSliceIndex = std::max(0, std::min(maxIndex, mSliceIndex)); }
                  }
                ImGui::TextUnformatted("Draw Axes");
                ImGui::SameLine(); ImGui::Checkbox("##drawAxes", &mDrawAxes);
                ImGui::Unindent();
              }
              ImGui::EndGroup();
            }
        }
        ImGui::EndGroup();

        // ImGui::SameLine(); ImGui::BeginGroup();
        // {
        //   ImGui::TextUnformatted("Mouse Force");
        //   ImGui::SetNextItemWidth(inputW);
        //   if(ImGui::InputFloat("Radius",  &mMForceRad, 0.01f, 0.1f, "%.8f")) { mFluid1->params.forceRad = mMForceRad; }
        //   // multipliers column
        //   ImGui::BeginGroup();
        //   {
        //     ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##vmultPush",&mPushVMult, 0.1f, 1.0f, "%.8f")) { mFluid1->params.vfPush = mPushVMult; }
        //     ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##vmultOut", &mOutVMult,  0.1f, 1.0f, "%.8f")) { mFluid1->params.vfOut  = mOutVMult;  }
        //     ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##vmultIn",  &mInVMult,   0.1f, 1.0f, "%.8f")) { mFluid1->params.vfIn   = mInVMult;   }
        //     ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##vmultCw",  &mCwVMult,   0.1f, 1.0f, "%.8f")) { mFluid1->params.vfCw   = mCwVMult;   }
        //     ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##vmultCcw", &mCcwVMult,  0.1f, 1.0f, "%.8f")) { mFluid1->params.vfCcw  = mCcwVMult;  }
        //     ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##dmult",    &mDMult,     0.1f, 1.0f, "%.8f")) { mFluid1->params.df     = mDMult;     }
        //     ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##pmult",    &mPMult,     0.1f, 1.0f, "%.8f")) { mFluid1->params.pf     = mPMult;     }
        //     ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##wvmult",   &mWVMult,    0.1f, 1.0f, "%.8f")) { mFluid1->params.wvf    = mWVMult;    }
        //   }

        //   ImGui::EndGroup();
        //   // enabled checkbox column
        //   ImGui::SameLine(); ImGui::BeginGroup();
        //   {
        //     ForceType ftype = HYPERFLUIDFORCE_PUSH; if(ImGui::Checkbox("Push",     &mFPush))     { if(mFPush)     { mFType |= ftype; } else { mFType &= ~ftype; } }
        //     ftype = HYPERFLUIDFORCE_OUT;            if(ImGui::Checkbox("Out",      &mFOut))      { if(mFOut)      { mFType |= ftype; } else { mFType &= ~ftype; } }
        //     ftype = HYPERFLUIDFORCE_IN;             if(ImGui::Checkbox("In",       &mFIn))       { if(mFIn)       { mFType |= ftype; } else { mFType &= ~ftype; } }
        //     ftype = HYPERFLUIDFORCE_CW;             if(ImGui::Checkbox("CW",       &mFCW))       { if(mFCW)       { mFType |= ftype; } else { mFType &= ~ftype; } }
        //     ftype = HYPERFLUIDFORCE_CCW;            if(ImGui::Checkbox("CCW",      &mFCCW))      { if(mFCCW)      { mFType |= ftype; } else { mFType &= ~ftype; } }
        //     ftype = HYPERFLUIDFORCE_DENSITY;        if(ImGui::Checkbox("Density",  &mFDensity))  { if(mFDensity)  { mFType |= ftype; } else { mFType &= ~ftype; } }
        //     ftype = HYPERFLUIDFORCE_PRESSURE;       if(ImGui::Checkbox("Pressure", &mFPressure)) { if(mFPressure) { mFType |= ftype; } else { mFType &= ~ftype; } }
        //     ftype = HYPERFLUIDFORCE_WV;             if(ImGui::Checkbox("WV/Chaos", &mFWv))       { if(mFWv)       { mFType |= ftype; } else { mFType &= ~ftype; } }
        //   }
        //   ImGui::EndGroup();
        // }
        // ImGui::EndGroup();

        // ImGui::Checkbox("Vector Field", &mDrawVectorField);
        // ImGui::SameLine(); ImGui::SetNextItemWidth(100.0f*scale);
        // if(ImGui::InputInt("Samples", &mVSamples.x, 1, 2))
        //   {
        //     if(mVSamples.x > mFluid1->size.x) { mVSamples.x = mFluid1->size.x; }
        //     mVSamples.x = std::max(1, mVSamples.x); mVSamples.y = mVSamples.x;
        //   }
        // ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Color");
        // ImGui::SameLine();
        // if(ColorSelect("Color", mVColor, mVColorLast, Vec4f(1,1,1,1), scale))
        //   { mActive = true; mVBColor.w = mVColor.w; }
        // ImGui::SameLine();
        // ImGui::TextUnformatted("Border");
        // ImGui::SameLine();
        // if(ColorSelect("BColor", mVBColor, mVBColorLast, Vec4f(0,0,0,1), scale))
        //   { mActive = true; mVColor.w = mVBColor.w; }
        // ImGui::SetNextItemWidth(100.0f*scale);
        // if(ImGui::InputFloat("VMult",    &mVMult,  0.01f, 0.1f, "%.4f"))      { mVMult = std::max(0.0f, mVMult); }
        // ImGui::SameLine(); ImGui::Checkbox("Constant Length", &mVConst);
        // ImGui::SetNextItemWidth(100.0f*scale);
        // if(ImGui::InputFloat("Width",  &mVWidth,   0.1f, 0.5f, "%.3f"))     { mVWidth = std::max(0.1f, std::min(5.0f, mVWidth)); }
        // ImGui::SameLine(); ImGui::SetNextItemWidth(100.0f*scale);
        // if(ImGui::InputFloat("Border",  &mVBWidth, 0.1f, 0.5f, "%.3f"))    { mVBWidth = std::max(0.0f, std::min(5.0f, mVBWidth)); }
        // ImGui::Text("     MPos: %1.4f, %1.4f", mFluid1->params.mp.x,     mFluid1->params.mp.y);
        // ImGui::Text("Last MPos: %1.4f, %1.4f", mFluid1->params.mpLast.x, mFluid1->params.mpLast.y);

        
        static char prefix[256] = "";
        strcpy(prefix, mPrefix.c_str());
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
        if(!mOfflineRender && ImGui::InputText("Prefix", prefix, 256, flags))
          {
            mPrefix   = prefix;
            mSavePath = "./rendered/" + mPrefix; // (rest of path added during update)
          }
        if(ImGui::InputInt("Number of Frames", &mTotalFrames, 1, 24)) { mTotalFrames = std::max(0, mTotalFrames); }

        if(mTotalFrames > 0 && ImGui::Button("Render Offline")) { mSaveFrame = 0; mOfflineRender = true; }

        
        // mFileDialog->setGraph(mGraph);
        // std::string savePath = mFileDialog->drawButton("Save##img", mFileDialog, "Save Image", DIALOG_SAVE, ".", {"*.png", "*.raw", "*.jpg", "*.bmp"});
        // if(!savePath.empty())
        //   {
        //     mSavePath = savePath; mSaving = true;
        //     mFieldTex.pullData();
        //     mSaveThread = std::thread([&]()
        //                               {
        //                               });
        //   }
        // else if(mSaveThread.joinable())
        //   { mSaveThread.join(); mSaving = false; }

        // if(!mSavePath.empty())
        //   {
        //     ImGui::SameLine();
        //     if(mSaving) { ImGui::Text("Writing image to %s...",  mSavePath.c_str()); }
        //     else        { ImGui::Text("Wrote image to %s (%.2f seconds)", mSavePath.c_str(), mSaveTime); }
        //   }
    
      }
    else if(mBodyVisible) { mSettingsOpen = false; }
    ImGui::EndGroup();
    
    Vec2f settingsSize = (Vec2f(ImGui::GetItemRectMax()) - ImGui::GetItemRectMin())/scale;
    if(mDisplaySize.x <= settingsSize.x || mDisplaySize.y <= 512.0f)
      { ImGui::SetCursorScreenPos(Vec2f(ImGui::GetCursorScreenPos()) + Vec2f((std::max(settingsSize.x, 512.0f)-mDisplaySize.x)/2.0f, 0.0f)*scale); }
    
    Vec2f p0 = ImGui::GetCursorScreenPos(); // top-left point after settings
    handleIO(p0);
    
    // draw cuda texture on screen
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(0,0));
    ImGui::BeginChild("##fDispChild", mDisplaySize*scale, true, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse);
    {
      Vec2f dpos = ImGui::GetCursorScreenPos();
      {
        //std::lock_guard<std::mutex> lock(mTexLock);
        mFluidTex.bind();
        ImGui::Image(mFluidTex.texId(), mDisplaySize*scale, Vec2f(0.0f, 0.0f), Vec2f(1.0f, 1.0f), ImColor(Vec4f(1,1,1,1)), Vec4f(0,0,0,1));
        mFluidTex.release();
      }
      
      mFieldHovered = ImGui::IsItemHovered() && !blocked;
      if(mDrawAxes)
        {
          // axis points
          Vec3d Worigin;
          Vec3d Wpx = Worigin + Vec3f(0.1, 0, 0);
          Vec3d Wpy = Worigin + Vec3f(0, 0.1, 0);
          Vec3d Wpz = Worigin + Vec3f(0, 0, 0.1);

          // recalculate camera orthonormal basis
          mCamDir   = normalize(mCamDir);
          mCamRight = normalize(cross(mCamDir,   Vec3d(0,0,1)));
          mCamUp    = normalize(cross(mCamRight, mCamDir));
          
          // view matrix
          Matrix<double> viewMat(4, 4); viewMat.identity();
          viewMat.setRow(0, {mCamRight.x, mCamRight.y, mCamRight.z, 0.0});
          viewMat.setRow(1, {mCamUp.x,    mCamUp.y,    mCamUp.z,    0.0});
          viewMat.setRow(2, {mCamDir.x,   mCamDir.y,   mCamDir.z,   0.0});
          Matrix<double> transMat(4, 4); transMat.identity();
          transMat[0][3] = mCamPos.x; transMat[1][3] = mCamPos.y; transMat[2][3] = mCamPos.z;

          // perspective projection matrix
          Matrix<double> projMat(4, 4);
          double S    = 1.0 / tan(mCamFov/2.0 * M_PI/180.0);
          double far  = 100.0;
          double near = 0.01;
          projMat[0][0] = S;
          projMat[1][1] = S;
          projMat[2][2] = -(far+near)/(far-near);
          projMat[2][3] = -2*(far*near)/(far-near);
          projMat[3][2] = -1.0;

          // combine matrices
          Matrix<double> VP = projMat ^ (viewMat ^ transMat);
          
          // transform points
          Matrix<double> Porigin = VP ^ Matrix<double>({0.0,   0.0,   0.0,   1.0});
          Matrix<double> Ppx     = VP ^ Matrix<double>({Wpx.x, Wpx.y, Wpx.z, 1.0});
          Matrix<double> Ppy     = VP ^ Matrix<double>({Wpy.x, Wpy.y, Wpy.z, 1.0});
          Matrix<double> Ppz     = VP ^ Matrix<double>({Wpz.x, Wpz.y, Wpz.z, 1.0});

          // normalize and offset
          Vec2f Sorigin = dpos + (Vec2f(-Porigin[0][0], -Porigin[1][0]) / Porigin[3][0] + Vec2f(1,1)/2) * mDisplaySize*scale;
          Vec2f Spx     = dpos + (Vec2f(-Ppx[0][0],     -Ppx[1][0])     / Ppx[3][0]     + Vec2f(1,1)/2) * mDisplaySize*scale;
          Vec2f Spy     = dpos + (Vec2f(-Ppy[0][0],     -Ppy[1][0])     / Ppy[3][0]     + Vec2f(1,1)/2) * mDisplaySize*scale;
          Vec2f Spz     = dpos + (Vec2f(-Ppz[0][0],     -Ppz[1][0])     / Ppz[3][0]     + Vec2f(1,1)/2) * mDisplaySize*scale;
          
          // draw axes
          ImDrawList *drawList = ImGui::GetWindowDrawList();
          drawList->AddLine(Sorigin, Spx, ImColor(Vec4f(1, 0, 0, 1)), 2.0f);
          drawList->AddLine(Sorigin, Spy, ImColor(Vec4f(0, 1, 0, 1)), 2.0f);
          drawList->AddLine(Sorigin, Spz, ImColor(Vec4f(0, 0, 1, 1)), 2.0f);
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
        
    // disable node interaction while interacting with field
    if(ImGui::IsMouseDown(ImGuiMouseButton_Left) && (mFieldHovered || mFieldLeftClicked || mFieldRightClicked)) { mActive = true; }

    // if(mDrawVectorField && mFluid1->allocated())
    //   {
    //     ImDrawList *drawList = ImGui::GetWindowDrawList();
    //     mFluid1->vx.pullData(); mFluid1->vy.pullData();
    //     Vec2f halfPix = Vec2f(0.5f,0.5f)/mFluid1->size; // normalized size of half a hyperfluid texel

    //     Vec2f  vs  = mGraph->viewSize();
    //     Vec2f  vp  = mGraph->viewPos() - Vec2f(vs.x, vs.y)/2.0f;
    //     Rect2f gRect(vp, vp+vs);
    //     Rect2f tRect(p0, p0+(Vec2f(ImGui::GetItemRectMax())-ImGui::GetItemRectMin()));        
        
    //     // only sample on-screen texels
    //     for(int sx = 0; sx <= mVSamples.x; sx++)
    //       for(int sy = 0; sy <= mVSamples.y; sy++)
    //         {
    //           Vec2f t1  = (Vec2f(sx, sy) + 0.5f) / mVSamples;  // texture coordinate [0.0, 1.0] offset by half a sample to center
    //           t1 *= mFluid1->size;  // scale to hyperfluid array coordinates
    //           Vec2f v = mFluid1->sampleVel(t1)/mFluid1->size; // sample hyperfluid
    //           if(mVConst) { v = v.normalized()/40.0f; } else { v *= 40.0f; }
    //           v = v*mVMult;
    //           t1 /= mFluid1->size; // scale back to normalized

    //           Vec2f t2  = t1 + v;
    //           Vec2f sp1 = p0 + scale*(t1 * mDisplaySize);
    //           Vec2f sp2 = p0 + scale*(t2 * mDisplaySize);
    //           // if(tRect.contains(sp1) || tRect.contains(sp2) || intersects(tRect, sp1, sp2))
    //             {
    //               drawLine(drawList, sp1, sp2, mVColor, mVWidth, mVBColor, mVBWidth,
    //                        0.5f/std::max(0.5f, (sp2-sp1).length()), 2.0f);
    //             }
    //         }
    //   }
  }
  ImGui::EndGroup();
  
  // std::cout << "HF DRAW DONE\n";
}

void HyperFluidNode::onResize(const Vec2f &dSize)
{
  mDisplaySize.y += dSize.y;
  mDisplaySize.x = mDisplaySize.y * ((float)mFluidSize.x / (float)mFluidSize.y);  // preserve hyperfluid aspect ratio
}
