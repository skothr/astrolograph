#include "fluidNode.hpp"

#include <imgui.h>

#include "setting.hpp"
#include "settingForm.hpp"
#include "cudaField.hpp"
#include "glfwKeys.hpp"
#include "complex.hpp"
#include "imtools.hpp"
#include "nodeGraph.hpp"
#include "vector-operators.h"

#define SETTING_LABEL_W 150.0f
#define SETTING_INPUT_W 255.0f


FluidNode::FluidNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Fluid Node", true)
{
  // settings
  mSettings.push_back(new Setting<bool>           ("Settings Open", "settingsOpen", &mSettingsOpen));
  SettingBase *fSizeSetting   = new Setting<Vec2i>("Field Size",    "fSize",        &mFieldSize,  mFieldSize);
  ((Setting<Vec2i>*)fSizeSetting)->setFormat(128, 256, "%d");
  mSettings.push_back(fSizeSetting);
  mSettings.push_back(new Setting<Vec2f>("Display Size",  "dispSize",     &mDisplaySize));

  // physics settings
  mSettings.push_back(new Setting<bool> ("Fill Circle",       "fillCircle",   &mFillCircle,     mFillCircle));
  mSettings.push_back(new Setting<bool> ("Density Pattern",   "dPattern",     &mDensityPattern, mDensityPattern));
  mSettings.push_back(new Setting<bool> ("Physics",           "physics",      &mPhysics,        mPhysics   ));
  mSettings.push_back(new Setting<float>("Time Step",         "timeStep",     &mTimeStep,       mTimeStep  ));
  mSettings.push_back(new Setting<float>("MForce Radius",     "forceRadius",  &mMForceRad,      mMForceRad ));
  mSettings.push_back(new Setting<float>("Push VMult",        "vfPush",       &mPushVMult,      mPushVMult ));
  mSettings.push_back(new Setting<float>("Out VMult",         "vfOut",        &mOutVMult,       mOutVMult  ));
  mSettings.push_back(new Setting<float>("In VMult",          "vfIn",         &mInVMult,        mInVMult   ));
  mSettings.push_back(new Setting<float>("Cw VMult",          "vfCw",         &mCwVMult,        mCwVMult   ));
  mSettings.push_back(new Setting<float>("Ccw VMult",         "vfCcw",        &mCcwVMult,       mCcwVMult  ));
  mSettings.push_back(new Setting<float>("MForce DMult",      "df",           &mDMult,          mDMult     ));
  mSettings.push_back(new Setting<float>("MForce PMult",      "pf",           &mPMult,          mPMult     ));
  mSettings.push_back(new Setting<float>("MForce WVMult",     "wvf",          &mWVMult,         mWVMult    ));
  mSettings.push_back(new Setting<float>("Gravity",           "gravity",      &mGravity,        mGravity   ));
  mSettings.push_back(new Setting<bool> ("Apply Chaos",       "applyChaos",   &mApplyChaos,     mApplyChaos));
  mSettings.push_back(new Setting<bool> ("Apply Viscosity",   "applyVisc",    &mApplyVisc,      mApplyVisc ));

  mSettings.push_back(new Setting<float>("Chaos",             "chaos",        &mChaos,          mChaos     ));
  mSettings.push_back(new Setting<float>("Viscosity",         "viscosity",    &mViscosity,      mViscosity ));
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

  // initialize fluid resources
  mFluid1 = new CudaFluid<float>();
  mFluid2 = new CudaFluid<float>();
  resizeField(mFieldSize);

  outputs()[FLUIDNODE_OUTPUT_VXFIELD]->set(&mFluid1->vx);
  outputs()[FLUIDNODE_OUTPUT_VYFIELD]->set(&mFluid1->vy);
  outputs()[FLUIDNODE_OUTPUT_DFIELD ]->set(&mFluid1->d);
  outputs()[FLUIDNODE_OUTPUT_PFIELD ]->set(&mFluid1->p);
  outputs()[FLUIDNODE_OUTPUT_DIV]->set(&mFluid1->div);
  outputs()[FLUIDNODE_OUTPUT_WVFIELD]->set(&mFluid1->wv);
  setTitle("Fluid Field");
  setMinSize(Vec2f(512, 512));
}

FluidNode::~FluidNode()
{
  //cudaDeviceSynchronize();
  if(mFluid1)    { mFluid1->destroy();    delete mFluid1; }
  if(mFluid2)    { mFluid2->destroy();    delete mFluid2; }
  if(mFluidLast) { mFluidLast->destroy(); delete mFluidLast; }
  mFluidTex.destroy();
}

void FluidNode::resizeField(const Vec2i &fSize)
{
  if(fSize.x > 0 && fSize.y > 0 && (fSize != mFluid1->size || fSize != mFluid2->size || fSize != mFluidTex.size || fSize != mFieldSize))
    {
      //cudaDeviceSynchronize();
      mFieldSize = fSize;
      if(!mFluid1)    { mFluid1    = new CudaFluid<float>(); } mFluid1->create(mFieldSize);
      if(!mFluid2)    { mFluid2    = new CudaFluid<float>(); } mFluid2->create(mFieldSize);
      if(!mFluidLast) { mFluidLast = new CudaFluid<float>(); } mFluidLast->create(mFieldSize);
      mFluidTex.create(mFieldSize);

      //cudaDeviceSynchronize();
      clearField(Vec4f(0, 0, 0, 1));
      //cudaDeviceSynchronize();

      //mDisplaySize.x = mDisplaySize.y * ((float)fSize.x / (float)fSize.y);
    }
}

void FluidNode::clearField(const Vec4f &color)
{
  if(mFillCircle)
    { fillFluidCircle(*mFluid1); fillFluidCircle(*mFluid2); fillFluidCircle(*mFluidLast); }
  else
    {
      if(mFluid1)         { clearFluid(*mFluid1); }
      if(mFluid2)         { clearFluid(*mFluid2); }
      if(mFluidLast)      { clearFluid(*mFluidLast); }
      if(mFluidTex.map()) { fillTex(mFluidTex.dData, mFluidTex.size.x, mFluidTex.size.y, float4{color.x,color.y,color.z,color.w}); mFluidTex.unmap(); }
    }
  if(mDensityPattern) { fillFluidPattern(*mFluid1); fillFluidPattern(*mFluid2); fillFluidPattern(*mFluidLast); }
}

bool FluidNode::handleIO(const Vec2f &p0)
{
  // io / control
  ImGuiIO &io  = ImGui::GetIO();
  float scale  = getScale();
  bool blocked = isBlocked();
  bool changed = false;
  Vec2f mp = ImGui::GetMousePos();
  Vec2f gp = screenToField(mp, &p0);

  mFieldHovered &= !blocked && !mPlacing && !mClicked;

  if(mFieldHovered && (mFieldClicked || !ImGui::IsKeyDown(GLFW_KEY_LEFT_CONTROL)) && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
      mFieldClicked = true;  mFluid1->params.mdown = true;
      Vec2f fmp = (mp - p0) / (mDisplaySize*scale);
      mFluid1->params.mp        = float2{fmp.x, fmp.y};
      mFluid1->params.mpLast    = float2{fmp.x, fmp.y};
      mFluid1->params.movedLast = false;
      mFluid1->params.lastMv    = float2{0.0f, 0.0f};
    }
  else if(ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    { mFieldClicked = false; mFluid1->params.mdown = false; mFluid2->params.mdown = false; mFluid1->params.movedLast = false; mFluid2->params.movedLast = false; }

  // dragging
  if(mFieldClicked && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
      Vec2f dmp = -Vec2f(ImGui::GetMouseDragDelta(ImGuiMouseButton_Left));
      ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
      //mFluid1->params.offset += dmp / mFluid1->vx.size / scale * 4.0f*mFluid1->params.scale;

      Vec2f fmp  = (mp - p0) / (mDisplaySize*scale);
      Vec2f fdmp = (dmp)     / (mDisplaySize*scale);

      if(mFluid1->params.mp.x != mFluid1->params.mpLast.x || mFluid1->params.mp.y != mFluid1->params.mpLast.y)
        { mFluid1->params.movedLast = true; }
      
      mFluid1->params.lastMv = mFluid1->params.mp - mFluid1->params.mpLast; // previous move vector
      mFluid1->params.mpLast = float2{fmp.x+fdmp.x, fmp.y+fdmp.y};
      mFluid1->params.mp     = float2{fmp.x, fmp.y};
      changed = true;
    }
  // else
  //   {
  //     Vec2f fmp  = (mp - p0) / (mDisplaySize*scale);
  //     mFluid.params.mpLast = float2{fmp.x-0.1f, fmp.y+};
  //     mFluid.params.mp     = float2{fmp.x, fmp.y};
  //   }

  // reset fluid with ESCAPE
  if(mFieldHovered && ImGui::IsKeyPressed(GLFW_KEY_ESCAPE) && !io.KeyCtrl)
    {
      mFluid1->params.scale  = float2{ 1.0f, 1.0f };
      mFluid1->params.offset = float2{ 0.0f, 0.0f };
      clearField(Vec4f(0.0f, 0.0f, 0.0f, 1.0f));
    }
  // reset fluid with ESCAPE
  if(mFieldHovered && ImGui::IsKeyPressed(GLFW_KEY_SPACE))
    { mPhysics = !mPhysics; }

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
  //               d
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

  mFluid2->params = mFluid1->params;
  return changed;
}

Vec2f FluidNode::fieldToScreen(const Vec2f &fp, const Vec2f *p0)
{
  float scale = getScale();
  Vec2f sp = (Vec2f((fp.x - mFluid1->params.offset.x)/mFluid1->params.scale.x,
                    (fp.y - mFluid1->params.offset.y)/mFluid1->params.scale.y)*Vec2f(mFieldSize.x, mFieldSize.y))*scale; // - Vec2f(0.5f, 0.5f))*scale;
  if(p0) { sp += *p0; }
  return sp;
}

Vec2f FluidNode::screenToField(const Vec2f &sp, const Vec2f *p0)
{
  float scale = getScale();
  Vec2f tp = (sp - (p0 ? *p0 : Vec2f())) / scale / mDisplaySize * mFluid1->vx.size; // + Vec2f(0.5, 0.5);
  return Vec2f(mFluid1->params.offset.x + mFluid1->params.scale.x * (tp.x/(float)mFluid1->vx.size.x),
               mFluid1->params.offset.y + mFluid1->params.scale.y * (tp.y/(float)mFluid1->vx.size.y));
}

void FluidNode::onUpdate()
{
  mFluid1->params.dt          = mTimeStep;
  mFluid1->params.gravity     = mGravity;
  mFluid1->params.applyChaos  = mApplyChaos;
  mFluid1->params.applyVisc   = mApplyVisc;
  mFluid1->params.chaos       = mChaos;
  mFluid1->params.viscosity   = mViscosity;
  mFluid1->params.forceRad    = mMForceRad;
  mFluid1->params.diffuseRad  = mDiffuseRad;
  mFluid1->params.projectIter = mProjectIter;

  mFluid1->params.mdown = mFieldClicked;
  ForceType ftype = FLUIDFORCE_NONE;
  if(mFPush   ) { ftype |= FLUIDFORCE_PUSH;    }
  if(mFOut    ) { ftype |= FLUIDFORCE_OUT;     } if(mFIn      ) { ftype |= FLUIDFORCE_IN;       }
  if(mFCW     ) { ftype |= FLUIDFORCE_CW;      } if(mFCCW     ) { ftype |= FLUIDFORCE_CCW;      }
  if(mFDensity) { ftype |= FLUIDFORCE_DENSITY; } if(mFPressure) { ftype |= FLUIDFORCE_PRESSURE; }
  if(mFWv)      { ftype |= FLUIDFORCE_WV; }

  mFluid1->params.forceRad = mMForceRad;
  mFluid1->params.ftype    = ftype;
  mFluid1->params.vfPush   = mPushVMult;
  mFluid1->params.vfOut    = mOutVMult;
  mFluid1->params.vfIn     = mInVMult;
  mFluid1->params.vfCw     = mCwVMult;
  mFluid1->params.vfCcw    = mCcwVMult;
  mFluid1->params.df       = mDMult;
  mFluid1->params.pf       = mPMult;
  mFluid1->params.wvf      = mWVMult;

  if(mFluid1->size != mFieldSize) { resizeField(mFieldSize); }

  bool ctrlDown  = (ImGui::IsKeyDown(GLFW_KEY_LEFT_CONTROL) || ImGui::IsKeyDown(GLFW_KEY_RIGHT_CONTROL));
  bool shiftDown = (ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT)   || ImGui::IsKeyDown(GLFW_KEY_RIGHT_SHIFT));
  bool altDown   = (ImGui::IsKeyDown(GLFW_KEY_LEFT_ALT)     || ImGui::IsKeyDown(GLFW_KEY_RIGHT_ALT));

  // step physics or set time direction with arrow keys
  bool   firstPress = !mManualStep;
  bool   repeat     = firstPress;
  double delay      = SHIFT_STEP_DELAY;
  double ctrlMult   = 1.0f; double altMult  = 1.0f;
  double keyMult    = (ctrlDown ? CTRL_SPEED_MULT : 1.0f) * (altDown ? ALT_SPEED_MULT : 1.0f);

  mManualStep = false;
  if(mFieldHovered      && ImGui::IsKeyDown(GLFW_KEY_RIGHT)) { mManualStep = true; mFluid1->params.dt =  mTimeStep; }
  else if(mFieldHovered && ImGui::IsKeyDown(GLFW_KEY_LEFT))  { mManualStep = true; mFluid1->params.dt = -mTimeStep; }
  else if(!mManualStep) { repeat = false; firstPress = false; }

  if((!mManualStep || mPhysics) && mFieldHovered) { } //mFluid1->params.dt *= keyMult; }
  else if(mManualStep)                            { delay /= keyMult; }

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

  mFluid2->params = mFluid1->params;

  bool stepping = mPhysics || mStepOnce || repeat;
  if(stepping)
    {
      mStepOnce = false;
      fluidAddForces(*mFluid1, *mFluid2, *mFluidLast);          std::swap(mFluid2, mFluid1);
      if(mDiffuseRad > 0) { fluidDiffusion(*mFluid1, *mFluid2); std::swap(mFluid2, mFluid1); }
      if(mIncompressible) { fluidProject(*mFluid1, *mFluid2);   std::swap(mFluid2, mFluid1); }
      fluidAdvection(*mFluid1, *mFluid2);                       std::swap(mFluid2, mFluid1);
      if(mIncompressible) { fluidProject(*mFluid1, *mFluid2);   std::swap(mFluid2, mFluid1); }
    }
  else
    { // add manual mouse forces, but prevent gravity
      mFluid1->params.dt = 0.0;       mFluid2->params.dt = 0.0;
      fluidAddForces(*mFluid1, *mFluid2, *mFluidLast); std::swap(mFluid2, mFluid1);
      mFluid1->params.dt = mTimeStep; mFluid2->params.dt = mTimeStep;
    }
  if(stepping) { fluidUpdateVel(*mFluid1, *mFluid2); std::swap(mFluid2, mFluid1); }
  renderFluid(*mFluid1, mFluidTex);

  //mFluid1->copyTo(*mFluidLast);
  
  outputs()[FLUIDNODE_OUTPUT_VXFIELD]->set(&mFluid1->vx);
  outputs()[FLUIDNODE_OUTPUT_VYFIELD]->set(&mFluid1->vy);
  outputs()[FLUIDNODE_OUTPUT_DFIELD ]->set(&mFluid1->d);
  outputs()[FLUIDNODE_OUTPUT_PFIELD ]->set(&mFluid1->p);
  outputs()[FLUIDNODE_OUTPUT_DIV]->set(&mFluid1->div);
  outputs()[FLUIDNODE_OUTPUT_WVFIELD]->set(&mFluid1->wv);
}

void FluidNode::onDraw()
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
            ImGui::SetNextItemWidth(100.0f*scale);
            if(ImGui::InputInt("W", &mFieldSize.x, 128, 256)) { resizeField(mFieldSize); }
            ImGui::SameLine();ImGui::SetNextItemWidth(100.0f*scale);
            if(ImGui::InputInt("H", &mFieldSize.y, 128, 256)) { resizeField(mFieldSize); }
            ImGui::Unindent();
          }
          ImGui::EndGroup();

          ImGui::BeginGroup();
          {
            ImGui::TextUnformatted("Physics");
            ImGui::Indent();
            if(ImGui::Button("Reset")) { clearField(Vec4f(0.0f, 0.0f, 0.0f, 1.0f)); }
            ImGui::SameLine(); ImGui::Checkbox("Fill With Circle", &mFillCircle);
            ImGui::SameLine(); ImGui::Checkbox("Checker", &mDensityPattern);
            ImGui::SameLine(); if(ImGui::Button("Step")) { mStepOnce = true; }
            ImGui::SameLine(); ImGui::Checkbox("Physics", &mPhysics);
            ImGui::SetNextItemWidth(inputW);
            if(ImGui::InputFloat("Time Step", &mTimeStep, 0.01f, 0.1f, "%.8f")) { mFluid1->params.dt = mTimeStep; }
            ImGui::SetNextItemWidth(inputW);
            if(ImGui::InputFloat("Chaos##mult",     &mChaos,    0.01f, 0.1f, "%.8f"))  { mFluid1->params.chaos = mChaos; }
            ImGui::SameLine(); if(ImGui::Checkbox("##chaosApply", &mApplyChaos))       { mFluid1->params.applyChaos = mApplyChaos; }
            ImGui::SetNextItemWidth(inputW);
            if(ImGui::InputFloat("Viscosity##mult",     &mViscosity,    0.01f, 0.1f, "%.8f"))  { mFluid1->params.viscosity = mViscosity; }
            ImGui::SameLine(); if(ImGui::Checkbox("##viscApply", &mApplyVisc))  { mFluid1->params.applyVisc = mApplyVisc; }
            ImGui::SetNextItemWidth(inputW);
            if(ImGui::InputInt("Diffuse Radius", &mDiffuseRad, 1, 2))  { mFluid1->params.diffuseRad = mDiffuseRad; }

            ImGui::SetNextItemWidth(inputW);
            ImGui::Checkbox("Incompressible", &mIncompressible);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(inputW);
            if(ImGui::InputInt("Iter##project", &mProjectIter, 1, 2))  { mFluid1->params.projectIter = mProjectIter; }
            ImGui::Unindent();
          }
          ImGui::EndGroup();

          ImGui::BeginGroup();
          {
            ImGui::TextUnformatted("Forces");
            ImGui::Indent();
            ImGui::SetNextItemWidth(inputW);
            if(ImGui::InputFloat("Gravity",   &mGravity, 0.01f, 0.1f, "%.8f"))  { mFluid1->params.gravity = mGravity; }
            ImGui::Unindent();
          }
          ImGui::EndGroup();
        }
        ImGui::EndGroup();

        ImGui::SameLine(); ImGui::BeginGroup();
        {
          ImGui::TextUnformatted("Mouse Force");
          ImGui::SetNextItemWidth(inputW);
          if(ImGui::InputFloat("Radius",  &mMForceRad, 0.01f, 0.1f, "%.8f")) { mFluid1->params.forceRad = mMForceRad; }
          // multipliers column
          ImGui::BeginGroup();
          {
            ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##vmultPush",&mPushVMult, 0.1f, 1.0f, "%.8f")) { mFluid1->params.vfPush = mPushVMult; }
            ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##vmultOut", &mOutVMult,  0.1f, 1.0f, "%.8f")) { mFluid1->params.vfOut  = mOutVMult;  }
            ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##vmultIn",  &mInVMult,   0.1f, 1.0f, "%.8f")) { mFluid1->params.vfIn   = mInVMult;   }
            ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##vmultCw",  &mCwVMult,   0.1f, 1.0f, "%.8f")) { mFluid1->params.vfCw   = mCwVMult;   }
            ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##vmultCcw", &mCcwVMult,  0.1f, 1.0f, "%.8f")) { mFluid1->params.vfCcw  = mCcwVMult;  }
            ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##dmult",    &mDMult,     0.1f, 1.0f, "%.8f")) { mFluid1->params.df     = mDMult;     }
            ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##pmult",    &mPMult,     0.1f, 1.0f, "%.8f")) { mFluid1->params.pf     = mPMult;     }
            ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##wvmult",   &mWVMult,    0.1f, 1.0f, "%.8f")) { mFluid1->params.wvf    = mWVMult;    }
          }

          ImGui::EndGroup();
          // enabled checkbox column
          ImGui::SameLine(); ImGui::BeginGroup();
          {
            ForceType ftype = FLUIDFORCE_PUSH; if(ImGui::Checkbox("Push",     &mFPush))     { if(mFPush)     { mFType |= ftype; } else { mFType &= ~ftype; } }
            ftype = FLUIDFORCE_OUT;            if(ImGui::Checkbox("Out",      &mFOut))      { if(mFOut)      { mFType |= ftype; } else { mFType &= ~ftype; } }
            ftype = FLUIDFORCE_IN;             if(ImGui::Checkbox("In",       &mFIn))       { if(mFIn)       { mFType |= ftype; } else { mFType &= ~ftype; } }
            ftype = FLUIDFORCE_CW;             if(ImGui::Checkbox("CW",       &mFCW))       { if(mFCW)       { mFType |= ftype; } else { mFType &= ~ftype; } }
            ftype = FLUIDFORCE_CCW;            if(ImGui::Checkbox("CCW",      &mFCCW))      { if(mFCCW)      { mFType |= ftype; } else { mFType &= ~ftype; } }
            ftype = FLUIDFORCE_DENSITY;        if(ImGui::Checkbox("Density",  &mFDensity))  { if(mFDensity)  { mFType |= ftype; } else { mFType &= ~ftype; } }
            ftype = FLUIDFORCE_PRESSURE;       if(ImGui::Checkbox("Pressure", &mFPressure)) { if(mFPressure) { mFType |= ftype; } else { mFType &= ~ftype; } }
            ftype = FLUIDFORCE_WV;             if(ImGui::Checkbox("WV/Chaos", &mFWv))       { if(mFWv)       { mFType |= ftype; } else { mFType &= ~ftype; } }
          }
          ImGui::EndGroup();
        }
        ImGui::EndGroup();

        ImGui::Checkbox("Vector Field", &mDrawVectorField);
        ImGui::SameLine(); ImGui::SetNextItemWidth(100.0f*scale);
        if(ImGui::InputInt("Samples", &mVSamples.x, 1, 2))
          {
            if(mVSamples.x > mFluid1->size.x) { mVSamples.x = mFluid1->size.x; }
            mVSamples.x = std::max(1, mVSamples.x); mVSamples.y = mVSamples.x;
          }
        ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Color");
        ImGui::SameLine();
        if(ColorSelect("Color", mVColor, mVColorLast, Vec4f(1,1,1,1), scale))
          { mActive = true; mVBColor.w = mVColor.w; }
        ImGui::SameLine();
        ImGui::TextUnformatted("Border");
        ImGui::SameLine();
        if(ColorSelect("BColor", mVBColor, mVBColorLast, Vec4f(0,0,0,1), scale))
          { mActive = true; mVColor.w = mVBColor.w; }




        ImGui::SetNextItemWidth(100.0f*scale);
        if(ImGui::InputFloat("VMult",    &mVMult,  0.01f, 0.1f, "%.4f"))      { mVMult = std::max(0.0f, mVMult); }
        ImGui::SameLine(); ImGui::Checkbox("Constant Length", &mVConst);
        ImGui::SetNextItemWidth(100.0f*scale);
        if(ImGui::InputFloat("Width",  &mVWidth,   0.1f, 0.5f, "%.3f"))     { mVWidth = std::max(0.1f, std::min(5.0f, mVWidth)); }
        ImGui::SameLine(); ImGui::SetNextItemWidth(100.0f*scale);
        if(ImGui::InputFloat("Border",  &mVBWidth, 0.1f, 0.5f, "%.3f"))    { mVBWidth = std::max(0.0f, std::min(5.0f, mVBWidth)); }

        // ImGui::Text("     MPos: %1.4f, %1.4f", mFluid1->params.mp.x,     mFluid1->params.mp.y);
        // ImGui::Text("Last MPos: %1.4f, %1.4f", mFluid1->params.mpLast.x, mFluid1->params.mpLast.y);
      }
    else if(mBodyVisible) { mSettingsOpen = false; }
    ImGui::EndGroup();

    Vec2f settingsSize = (Vec2f(ImGui::GetItemRectMax()) - ImGui::GetItemRectMin())/scale;
    if(mDisplaySize.x <= settingsSize.x || mDisplaySize.y <= 512.0f)
      { ImGui::SetCursorScreenPos(Vec2f(ImGui::GetCursorScreenPos()) + Vec2f((std::max(settingsSize.x, 512.0f)-mDisplaySize.x)/2.0f, 0.0f)*scale); }

    Vec2f p0 = ImGui::GetCursorScreenPos(); // top-left point after settings
    handleIO(p0);

    // draw cuda texture on screen
    mFluidTex.bind();

    ImGui::Image(mFluidTex.texId(), mDisplaySize*scale, Vec2f(0.0f, 0.0f), Vec2f(1.0f, 1.0f), ImColor(Vec4f(1,1,1,1)), Vec4f(0,0,0,1));
    //Vec2f tpos = Vec2f(ImGui::GetItemRectMax()) - ImGui::GetItemRectMin();
    mFluidTex.release();
    mFieldHovered = ImGui::IsItemHovered() && !blocked;

    // disable node interaction while interacting with field
    if(ImGui::IsMouseDown(ImGuiMouseButton_Left) && !mClicked && (mFieldHovered || mFieldClicked)) { mActive = true; }

    if(mDrawVectorField && mFluid1->allocated())
      {
        ImDrawList *drawList = ImGui::GetWindowDrawList();
        mFluid1->vx.pullData(); mFluid1->vy.pullData();
        Vec2f halfPix = Vec2f(0.5f,0.5f)/mFluid1->size; // normalized size of half a fluid texel

        Vec2f  vs  = mGraph->viewSize();
        Vec2f  vp  = mGraph->viewPos() - Vec2f(vs.x, vs.y)/2.0f;
        Rect2f gRect(vp, vp+vs);
        Rect2f tRect(p0, p0+(Vec2f(ImGui::GetItemRectMax())-ImGui::GetItemRectMin()));

        // only sample on-screen texels
        for(int sx = 0; sx <= mVSamples.x; sx++)
          for(int sy = 0; sy <= mVSamples.y; sy++)
            {
              Vec2f t1  = (Vec2f(sx, sy) + 0.5f) / mVSamples;  // texture coordinate [0.0, 1.0] offset by half a sample to center
              t1 *= mFluid1->size;  // scale to fluid array coordinates
              Vec2f v = mFluid1->sampleVel(t1)/mFluid1->size; // sample fluid
              if(mVConst) { v = v.normalized()/40.0f; } else { v *= 40.0f; }
              v = v*mVMult;
              t1 /= mFluid1->size; // scale back to normalized

              Vec2f t2  = t1 + v;
              Vec2f sp1 = p0 + scale*(t1 * mDisplaySize);
              Vec2f sp2 = p0 + scale*(t2 * mDisplaySize);
              // if(tRect.contains(sp1) || tRect.contains(sp2) || intersects(tRect, sp1, sp2))
              {
                drawLine(drawList, sp1, sp2, mVColor, mVWidth, mVBColor, mVBWidth,
                         0.5f/std::max(0.5f, (sp2-sp1).length()), 2.0f);
              }
            }
      }
  }
  ImGui::EndGroup();
}

void FluidNode::onResize(const Vec2f &dSize)
{
  mDisplaySize.y += dSize.y;
  mDisplaySize.x = mDisplaySize.y * ((float)mFluid1->size.x / (float)mFluid1->size.y);  // preserve fluid aspect ratio
}
