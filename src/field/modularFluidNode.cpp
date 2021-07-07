#include "modularFluidNode.hpp"
using namespace astro;

#include <imgui.h>

#include "setting.hpp"
#include "settingForm.hpp"
#include "cudaField.hpp"
#include "glfwKeys.hpp"
#include "complex.hpp"
#include "imtools.hpp"
#include "vector-operators.h"

#define SETTING_LABEL_W 150.0f
#define SETTING_INPUT_W 255.0f


FluidStateNode::FluidStateNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Modular Fluid Node", true)
{
  // settings
  mSettings.push_back(new Setting<bool>           ("Settings Open", "settingsOpen", &mSettingsOpen));
  SettingBase *fSizeSetting   = new Setting<Vec2i>("Field Size",    "fSize",        &mFieldSize,  mFieldSize);
  ((Setting<Vec2i>*)fSizeSetting)->setFormat(128, 256, "%d");
  mSettings.push_back(fSizeSetting);
  mSettings.push_back(new Setting<Vec2f>("Display Size",  "dispSize",     &mDisplaySize));

  // physics settings
  mSettings.push_back(new Setting<bool> ("Fill Circle",   "fillCircle",   &mFillCircle,      mFillCircle));
  mSettings.push_back(new Setting<bool> ("Physics",       "physics",      &mPhysics,         mPhysics   ));
  mSettings.push_back(new Setting<float>("Time Step",     "timeStep",     &mTimeStep,        mTimeStep  ));
  mSettings.push_back(new Setting<float>("MForce Radius", "forceRadius",  &mMForceRad,       mMForceRad ));
  mSettings.push_back(new Setting<float>("Push VMult",    "vfPush",       &mPushVMult,       mPushVMult ));
  mSettings.push_back(new Setting<float>("Out VMult",     "vfOut",        &mOutVMult,        mOutVMult  ));
  mSettings.push_back(new Setting<float>("In VMult",      "vfIn",         &mInVMult,         mInVMult   ));
  mSettings.push_back(new Setting<float>("Cw VMult",      "vfCw",         &mCwVMult,         mCwVMult   ));
  mSettings.push_back(new Setting<float>("Ccw VMult",     "vfCcw",        &mCcwVMult,        mCcwVMult  ));
  mSettings.push_back(new Setting<float>("MForce DMult",  "df",           &mDMult,           mDMult     ));
  mSettings.push_back(new Setting<float>("MForce PMult",  "pf",           &mPMult,           mPMult     ));
  mSettings.push_back(new Setting<float>("MForce WVMult", "wvf",          &mWVMult,          mWVMult    ));
  // force settings
  mSettings.push_back(new Setting<bool> ("Push",     "pushForce",     &mFPush,     mFPush    ));
  mSettings.push_back(new Setting<bool> ("Out",      "outForce",      &mFOut,      mFOut     ));
  mSettings.push_back(new Setting<bool> ("In",       "inForce",       &mFIn,       mFIn      ));
  mSettings.push_back(new Setting<bool> ("CW",       "cwForce",       &mFCW,       mFCW      ));
  mSettings.push_back(new Setting<bool> ("CCW",      "ccwForce",      &mFCCW,      mFCCW     ));
  mSettings.push_back(new Setting<bool> ("Density",  "densityForce",  &mFDensity,  mFDensity ));
  mSettings.push_back(new Setting<bool> ("Pressure", "pressureForce", &mFPressure, mFPressure));
  mSettings.push_back(new Setting<float>("Gravity",           "gravity",      &mGravity,         mGravity        ));
  mSettings.push_back(new Setting<bool> ("Apply Chaos",       "applyChaos",   &mApplyChaos,         mApplyChaos        ));
  mSettings.push_back(new Setting<bool> ("Draw Vector Field", "drawVField",   &mDrawVectorField, mDrawVectorField));
  // initialize fluid resources
  resizeField(mFieldSize);

  //mField.create(mFieldSize);
  outputs()[FLUIDSTATE_OUTPUT_FLUID  ]->set(&mFluid);
  outputs()[FLUIDSTATE_OUTPUT_VXFIELD]->set(&mFluid.vx);
  outputs()[FLUIDSTATE_OUTPUT_VYFIELD]->set(&mFluid.vy);
  outputs()[FLUIDSTATE_OUTPUT_DFIELD ]->set(&mFluid.d);
  outputs()[FLUIDSTATE_OUTPUT_PFIELD ]->set(&mFluid.p);
  outputs()[FLUIDSTATE_OUTPUT_WVFIELD]->set(&mFluid.wv);
  setTitle("Fluid State");
  setMinSize(mFieldSize);
}

FluidStateNode::~FluidStateNode()
{
  //cudaDeviceSynchronize();
  mFluid.destroy();
  mFluidTex.destroy();
}

void FluidStateNode::resizeField(const Vec2i &fSize)
{
  if(fSize.x > 0 && fSize.y > 0 && (fSize != mFluid.size || fSize != mFluidTex.size || fSize != mFieldSize))
    {
      //cudaDeviceSynchronize();
      mFieldSize = fSize;
      mFluid.destroy();    mFluid.create(mFieldSize);
      mFluidTex.destroy(); mFluidTex.create(mFieldSize);
      
      //cudaDeviceSynchronize();
      clearField(Vec4f(0, 0, 0, 1));
      //cudaDeviceSynchronize();
      
      //mDisplaySize.x = mDisplaySize.y * ((float)fSize.x / (float)fSize.y);
    }
}

void FluidStateNode::clearField(const Vec4f &color)
{
  if(mFillCircle) { fillFluidCircle(mFluid); }
  else
    {
      clearFluid(mFluid);
      if(mFluidTex.map())
        { fillTex(mFluidTex.dData, mFluidTex.size.x, mFluidTex.size.y, float4{color.x,color.y,color.z,color.w}); mFluidTex.unmap(); }
    }
  //cudaDeviceSynchronize();  
}

bool FluidStateNode::handleIO(const Vec2f &p0)
{
  // io / control
  ImGuiIO &io  = ImGui::GetIO();
  float scale  = getScale();
  bool blocked = isBlocked();
  bool changed = false;
  Vec2f mp = ImGui::GetMousePos();
  Vec2f gp = screenToField(mp, &p0);

  mFieldHovered &= !blocked;
  
  if(mFieldHovered && (mFieldClicked || !ImGui::IsKeyDown(GLFW_KEY_LEFT_CONTROL)) && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
      mFieldClicked = true;  mFluid.params.mdown = true; 
      Vec2f fmp = (mp - p0) / (mDisplaySize*scale);
      mFluid.params.mp = float2{fmp.x, fmp.y};
      mFluid.params.mpLast = mFluid.params.mp;
    }
  else if(ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    { mFieldClicked = false; mFluid.params.mdown = false; }

  // dragging
  if(mFieldClicked && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
      Vec2f dmp = -Vec2f(ImGui::GetMouseDragDelta(ImGuiMouseButton_Left));
      ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
      //mFluid.params.offset += dmp / mFluid.vx.size / scale * 4.0f*mFluid.params.scale;
      
      Vec2f fmp  = (mp - p0) / (mDisplaySize*scale);
      Vec2f fdmp = (dmp) / (mDisplaySize*scale);
      mFluid.params.mpLast = float2{fmp.x+fdmp.y, fmp.y+fdmp.y};
      mFluid.params.mp = float2{fmp.x, fmp.y};
      changed = true;
    }
  
  // reset fluid with ESCAPE
  if(mFieldHovered && ImGui::IsKeyPressed(GLFW_KEY_ESCAPE) && !io.KeyCtrl)
    {
      mFluid.params.scale  = float2{ 1.0f, 1.0f };
      mFluid.params.offset = float2{ 0.0f, 0.0f };
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
  return changed;
}


Vec2f FluidStateNode::fieldToScreen(const Vec2f &fp, const Vec2f *p0)
{
  float scale = getScale();
  Vec2f sp = (Vec2f((fp.x - mFluid.params.offset.x)/mFluid.params.scale.x,
                    (fp.y - mFluid.params.offset.y)/mFluid.params.scale.y)*Vec2f(mFieldSize.x, mFieldSize.y) - Vec2f(0.5f, 0.5f))*scale;
  if(p0) { sp += *p0; }
  return sp;
}

Vec2f FluidStateNode::screenToField(const Vec2f &sp, const Vec2f *p0)
{
  float scale = getScale();
  Vec2f tp = (sp - (p0 ? *p0 : Vec2f())) / scale + Vec2f(0.5, 0.5);
  return Vec2f(mFluid.params.offset.x + mFluid.params.scale.x * (tp.x/(float)mFluid.vx.size.x),
               mFluid.params.offset.y + mFluid.params.scale.y * (tp.y/(float)mFluid.vx.size.y));
}

void FluidStateNode::onUpdate()
{
  mFluid.params.dt       = mTimeStep;
  mFluid.params.gravity  = mGravity;
  mFluid.params.applyChaos    = mApplyChaos;
  mFluid.params.forceRad = mMForceRad;

  ForceType ftype = FLUIDFORCE_NONE;
  if(mFPush    ) { ftype |= FLUIDFORCE_PUSH;    }
  if(mFOut     ) { ftype |= FLUIDFORCE_OUT;     } if(mFIn      ) { ftype |= FLUIDFORCE_IN;       }
  if(mFCW      ) { ftype |= FLUIDFORCE_CW;      } if(mFCCW     ) { ftype |= FLUIDFORCE_CCW;      }
  if(mFDensity ) { ftype |= FLUIDFORCE_DENSITY; } if(mFPressure) { ftype |= FLUIDFORCE_PRESSURE; }
  
  mFluid.params.ftype    = ftype;
  mFluid.params.vfPush   = mPushVMult;
  mFluid.params.vfOut    = mOutVMult;
  mFluid.params.vfIn     = mInVMult;
  mFluid.params.vfCw     = mCwVMult;
  mFluid.params.vfCcw    = mCcwVMult;
  mFluid.params.df       = mDMult;
  mFluid.params.pf       = mPMult;
  mFluid.params.wvf      = mWVMult;

  if(mFluid.size != mFieldSize) { resizeField(mFieldSize); }
  
  if     (ImGui::IsKeyDown(GLFW_KEY_RIGHT)) { mStepOnce = true; mFluid.params.dt =  mTimeStep; }
  else if(ImGui::IsKeyDown(GLFW_KEY_LEFT))  { mStepOnce = true; mFluid.params.dt = -mTimeStep; }
  
  bool stepping = mPhysics || mStepOnce;
  if(stepping)
    {
      mStepOnce = false;
      fluidAdvection(mFluid, mFluid); //cudaDeviceSynchronize();
      fluidDiffusion(mFluid, mFluid); //cudaDeviceSynchronize();
      fluidAddForces(mFluid, mFluid); //cudaDeviceSynchronize();
    }
  else
    { // prevent gravity, but add manual mouse forces
      mFluid.params.dt = 0.0;
      fluidAddForces(mFluid, mFluid); //cudaDeviceSynchronize();
      mFluid.params.dt = mTimeStep;
    }
  if(stepping)
    { fluidUpdateVel(mFluid, mFluid); } //cudaDeviceSynchronize(); }
  renderFluid(mFluid, mFluidTex);     //cudaDeviceSynchronize();
}

void FluidStateNode::onDraw()
{
  bool  blocked = isBlocked();
  float scale   = getScale();
  float inputW  = 150.0f*scale;;
  
  ImGui::BeginGroup();
  {
    //if(mFieldSize != mFluid.size) { resizeField(mFieldSize); }

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
            ImGui::SameLine(); if(ImGui::Button("Step")) { mStepOnce = true; }
            ImGui::SameLine(); ImGui::Checkbox("Physics", &mPhysics);
            ImGui::SetNextItemWidth(inputW);
            if(ImGui::InputFloat("Time Step", &mTimeStep, 0.001, 0.1, "%.8f")) { mFluid.params.dt = mTimeStep; }
            ImGui::Unindent();
          }
          ImGui::EndGroup();
      
          ImGui::BeginGroup();
          {
            ImGui::TextUnformatted("Forces");
            ImGui::Indent();
            ImGui::SetNextItemWidth(inputW);
            if(ImGui::InputFloat("Gravity", &mGravity, 0.01, 0.1, "%.8f")) { mFluid.params.gravity = mGravity; }
            if(ImGui::Checkbox("Apply WV", &mApplyChaos))   { mFluid.params.applyChaos = mApplyChaos; }
            ImGui::SameLine(); ImGui::SetNextItemWidth(inputW);
            if(ImGui::InputFloat("WV Mult", &mWVMult, 0.01, 0.1, "%.8f")) { mFluid.params.wvf = mWVMult; }
            ImGui::Unindent();
          }
          ImGui::EndGroup();
        }
        ImGui::EndGroup();

        ImGui::SameLine(); ImGui::BeginGroup();
        {
          ImGui::TextUnformatted("Mouse Force");
          ImGui::SetNextItemWidth(inputW);
          if(ImGui::InputFloat("Radius",  &mMForceRad, 0.01, 0.1, "%.8f")) { mFluid.params.forceRad = mMForceRad; }
          // multipliers column
          ImGui::BeginGroup();
          {
            ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##vmultPush",&mPushVMult, 0.01, 0.1, "%.8f")) { mFluid.params.vfPush = mPushVMult; }
            ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##vmultOut", &mOutVMult,  0.01, 0.1, "%.8f")) { mFluid.params.vfOut  = mOutVMult; }
            ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##vmultIn",  &mInVMult,   0.01, 0.1, "%.8f")) { mFluid.params.vfIn   = mInVMult; }
            ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##vmultCw",  &mCwVMult,   0.01, 0.1, "%.8f")) { mFluid.params.vfCw   = mCwVMult; }
            ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##vmultCcw", &mCcwVMult,  0.01, 0.1, "%.8f")) { mFluid.params.vfCcw  = mCcwVMult; }
            ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##dmult",    &mDMult,     0.01, 0.1, "%.8f")) { mFluid.params.df     = mDMult;     }
            ImGui::SetNextItemWidth(inputW); if(ImGui::InputFloat("##pmult",    &mPMult,     0.01, 0.1, "%.8f")) { mFluid.params.pf     = mPMult;     }
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
          }
          ImGui::EndGroup();
        }
        ImGui::EndGroup();

        ImGui::Text("     MPos: %1.4f, %1.4f", mFluid.params.mp.x, mFluid.params.mp.y);
        ImGui::Text("Last MPos: %1.4f, %1.4f", mFluid.params.mpLast.x, mFluid.params.mpLast.y);

      }
    else if(mBodyVisible) { mSettingsOpen = false; }
        
    Vec2f p0 = ImGui::GetCursorScreenPos(); // top-left point after settings
    handleIO(p0);

    // draw cuda texture on screen
    mFluidTex.bind();
    ImGui::Image(mFluidTex.texId(), mDisplaySize*scale, Vec2f(0.0f, 0.0f), Vec2f(1.0f, 1.0f), ImColor(Vec4f(1,1,1,1)), Vec4f(0,0,0,1));
    mFluidTex.release();
    mFieldHovered = ImGui::IsItemHovered() && !blocked;
    
    // disable node interaction while interacting with field
    if(ImGui::IsMouseDown(ImGuiMouseButton_Left) && (mFieldHovered || mFieldClicked) && !ImGui::GetIO().KeyCtrl) { mActive = true; }

    if(mDrawVectorField)
      {
#define SAMPLE_SPACING 
        
        mFluid.vx.pullData();
        mFluid.vy.pullData();

        mFluid.sampleVel(Vec2f(mFluid.size.x, mFluid.size.y)/2.0f);
        
        // ImDrawList *drawList = ImGui::GetWindowDrawList();
        // Vec2f mp = Vec2f(ImGui::GetMousePos());
        // Vec2f gp = screenToField(mp, &p0);
        // Vec2f pLast = mp;
        // Complex<double> zLast((double)gp.x, (double)gp.y);
        // Complex<double> c((double)gp.x, (double)gp.y);
        // for(int i = 0; i < mFluid.params.maxIter; i++)
        //   {
        //     Complex<double> z = zLast*zLast + c;
        //     Vec2f p = fieldToScreen(Vec2f(z.real, z.imag), &p0);
        //     drawLine(drawList, pLast, p, Vec4f(1.0f, 0.0f, 0.0f, 1.0f), 2.0f, Vec4f(0.0f, 0.0f, 0.0f, 1.0f), 1.0f);
        //     zLast = z; pLast = p;
        //   }
      }
  }
  ImGui::EndGroup();
}

void FluidStateNode::onResize(const Vec2f &dSize)
{
  mDisplaySize.y += dSize.y;
  mDisplaySize.x = mDisplaySize.y * ((float)mFluid.size.x / (float)mFluid.size.y);  // preserve fluid aspect ratio
  // if(ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT) || ImGui::IsKeyDown(GLFW_KEY_RIGHT_SHIFT))
  //   {
  //     float iso = (mDisplaySize.x + mDisplaySize.y)/2.0f; // shift key -- isometric scaling
  //     mDisplaySize = Vec2f(iso, iso);
  //   }
}
