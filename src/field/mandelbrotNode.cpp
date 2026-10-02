#include "mandelbrotNode.hpp"
// using namespace quantum;

#include <imgui.h>
#include <chrono>

#include "fileDialog.hpp"
#include "setting.hpp"
#include "settingForm.hpp"
#include "cudaField.hpp"
#include "glfwKeys.hpp"
#include "complex.hpp"
#include "imtools.hpp"
#include "vector-operators.h"

#define SETTING_LABEL_W 150.0f
#define SETTING_INPUT_W 280.0f

MandelbrotNode::MandelbrotNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Mandelbrot Node", true), mFileDialog(new FileDialog())
{
  // settings
  mSettings.push_back(new Setting<bool>          ("Settings Open",  "settingsOpen",  &mSettingsOpen, mSettingsOpen));
  mSettings.push_back(new Setting<Vec2f>         ("Display Size",   "dispSize",      &mDisplaySize,  mDisplaySize));
  SettingBase *fSizeSetting  = new Setting<Vec2i>("Field Size",     "fSize",         &mFieldSize,    Vec2i(512, 512), [&]() { resizeField(mFieldSize); });
  ((Setting<Vec2i>*)fSizeSetting)->setFormat(128, 256, "%d");
  ((Setting<Vec2i>*)fSizeSetting)->setMin(Vec2f(1, 1));
  ((Setting<Vec2i>*)fSizeSetting)->setMax(Vec2f(8192, 8192));
  SettingBase *scaleSetting  = new Setting<Vec2f>  ("Scale",          "fieldScale",    &mField.params.scale,   Vec2f(2.0f, 2.0f));
  ((Setting<Vec2f>*)scaleSetting)->setFormat(0.01f, 0.1f, "%.8f");
  SettingBase *offsetSetting = new Setting<Vec2f>  ("Offset",         "fieldOffset",   &mField.params.offset,  Vec2f(0.0f, 0.0f));
  ((Setting<Vec2f>*)offsetSetting)->setFormat(0.01f, 0.1f, "%.8f");
  SettingBase *cutoffSetting = new Setting<float>  ("Cutoff",         "mandelCutoff",  &mField.params.cutoff,  2.0f);
  ((Setting<float>*)cutoffSetting)->setFormat(0.01f, 0.1f, "%.2f");
  ((Setting<Vec2i>*)cutoffSetting)->setMin(0.0f);
  SettingBase *iterSetting   = new Setting<int>    ("Max Iterations", "mandelMaxIter", &mField.params.maxIter, 64);
  ((Setting<int>*)iterSetting)->setFormat(8, 16, "%d");
  ((Setting<int>*)iterSetting)->setMin(0);
  SettingBase *hollowSetting = new Setting<bool> ("Hollow",       "mandelHollow", &mHollow,   true, [&]() { mField.params.hollow = mHollow; });
  SettingBase *pathSetting   = new Setting<bool> ("Draw Path",    "drawPath",     &mDrawPath, false);
  SettingBase *pRadSetting   = new Setting<int>  ("Path Radius",  "pathRad",      &mPathRad);
  ((Setting<int>*)pRadSetting)->setMin(0);
  SettingBase *pSpaceSetting = new Setting<float>("Path Spacing", "pathSpacing",  &mPathSpacing);
  ((Setting<float>*)pSpaceSetting)->setFormat(0.01f, 0.1f, "%.4f");

  SettingGroup *group = new SettingGroup("Field Settings", "settings",
                                         { fSizeSetting, scaleSetting, offsetSetting, iterSetting, cutoffSetting, pathSetting, pRadSetting, pSpaceSetting, hollowSetting },
                                         true, false);
  SettingGroup *contextGroup = new SettingGroup("Context Settings", "contextSettings",
                                                { fSizeSetting, scaleSetting, offsetSetting, iterSetting, cutoffSetting, pathSetting, pRadSetting, pSpaceSetting, hollowSetting },
                                                false, false);
  mSettingForm = new SettingForm(SETTING_LABEL_W, SETTING_INPUT_W);
  mSettingForm->add(group);
  mContextForm = new SettingForm(SETTING_LABEL_W, SETTING_INPUT_W);
  mContextForm->add(contextGroup);
  
  mSettings.push_back(fSizeSetting);
  mSettings.push_back(scaleSetting);
  mSettings.push_back(offsetSetting);
  mSettings.push_back(cutoffSetting);
  mSettings.push_back(iterSetting);
  mSettings.push_back(hollowSetting);
  mSettings.push_back(pathSetting);
  mSettings.push_back(pRadSetting);
  mSettings.push_back(pSpaceSetting);
  mSettings.push_back(group);
  mSettings.push_back(contextGroup);

  mField.create(mFieldSize);
  mFieldTex.create(mFieldSize);
  outputs()[MANDELBROTNODE_OUTPUT_FIELD]->set(&mField);
  outputs()[MANDELBROTNODE_OUTPUT_TEX]->set(&mFieldTex);
  setTitle("Mandelbrot Set");
  setMinSize(Vec2f(512.0f, 512.0f));
}

MandelbrotNode::~MandelbrotNode()
{
  mField.destroy(); mFieldTex.destroy();
  if(mSettingForm) { delete mSettingForm; }
  if(mFileDialog)  { delete mFileDialog; }
}

void MandelbrotNode::resizeField(const Vec2f &fSize)
{
  if(fSize.x != 0 && fSize.y != 0 && (mFieldSize != fSize || mField.size != fSize || mFieldTex.size != fSize))
    {
      mField.create(fSize);
      mFieldTex.create(fSize);
      //mDisplaySize.x = mDisplaySize.y * ((float)fSize.x / (float)fSize.y);
    }
}

bool MandelbrotNode::handleIO(const Vec2f &p0)
{
  // io / control
  ImGuiIO &io  = ImGui::GetIO();
  float scale  = getScale();
  bool blocked = isBlocked();
  bool changed = false;
  Vec2f mp = ImGui::GetMousePos();
  Vec2f gp = screenToField(mp, &p0);

  mFieldHovered &= !blocked;
  
  if(mFieldHovered && (mFieldClicked || !ImGui::IsKeyDown(GLFW_KEY_LEFT_CONTROL)) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    { mFieldClicked = true; }
  else if(ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    { mFieldClicked = false; }
  
  if(mFieldClicked && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
      Vec2f dmp = -Vec2f(ImGui::GetMouseDragDelta(ImGuiMouseButton_Left));
      ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
      mField.params.offset += dmp / mDisplaySize / scale * 4.0f*mField.params.scale;
      changed = true;
    }

  // reset offset/scale with ESCAPE  
  if(mFieldHovered && !io.KeyCtrl && std::abs(io.MouseWheel) > 0.0f)
    {
      float vel = (io.KeyAlt ? 1.36 : 1.055); // scroll velocity
      mField.params.scale = mField.params.scale * (io.MouseWheel > 0.0f ? 1.0/vel : vel);
      Vec2f gp2 = screenToField(mp, &p0);
      mField.params.offset = mField.params.offset + (float2{gp.x, gp.y} - float2{gp2.x, gp2.y});
      changed = true;
    }
  
  // reset offset/scale with ESCAPE
  if(mFieldHovered && ImGui::IsKeyPressed(GLFW_KEY_ESCAPE) && !io.KeyCtrl)
    { mField.params.scale = float2{ 1.0f, 1.0f }; mField.params.offset = float2{ 0.0f, 0.0f }; }

  std::string contextName = "##plotContext";
  if(BeginContext(contextName, CONTEXT_WINDOW_RCLICK, mFieldHovered))
    {
      mActive = true;
      mContextForm->draw(1.0f, false);
      EndContext(true);
    }
  else
    {
      EndContext(false);
      if(mFieldHovered)
        {
          // draw tooltip
          BeginTooltip();
          {
            Vec2f tp = (mp - p0) / scale + Vec2f(0.5, 0.5); // position in texture
            if(tp.x >= 0.0 && tp.x < mDisplaySize.x && tp.y >= 0.0 && tp.y < mDisplaySize.y)
              {
                int i = ((int)tp.y)*mDisplaySize.x + (int)tp.x;
                Vec2f lp(mField.hData[i].x, mField.hData[i].y);
                ImGui::Text("Position:    < %s%.8f + %s%.8f i >", (gp.x < 0.0 ? "-" : " "), abs(gp.x), (gp.y < 0.0 ? "-" : " "), abs(gp.y));
                ImGui::Text("Final Value: < %s%.8f + %s%.8f i >", (lp.x < 0.0 ? "-" : " "), abs(lp.x), (lp.y < 0.0 ? "-" : " "), abs(lp.y));
              }
            else
              { ImGui::Text("< N/A >"); }
          }
          EndTooltip();
        }
    }
  return changed;
}



Vec2f MandelbrotNode::fieldToScreen(const Vec2f &fp, const Vec2f *p0)
{
  float scale = getScale();
  Vec2f sp = (Vec2f((fp.x - mField.params.offset.x)/(4.0f*mField.params.scale.x) + 0.5f,
                    (fp.y - mField.params.offset.y)/(4.0f*mField.params.scale.y) + 0.5f)*mDisplaySize - Vec2f(0.5f, 0.5f))*scale;
  if(p0) { sp += *p0; }
  return sp;
}

Vec2f MandelbrotNode::screenToField(const Vec2f &sp, const Vec2f *p0)
{
  float scale = getScale();
  Vec2f tp = (sp - (p0 ? *p0 : Vec2f())) / scale + Vec2f(0.5, 0.5);
  return Vec2f(mField.params.offset.x + 4.0f*mField.params.scale.x * (tp.x/mDisplaySize.x - 0.5f),
               mField.params.offset.y + 4.0f*mField.params.scale.y * (tp.y/mDisplaySize.y - 0.5f));
}

void MandelbrotNode::onUpdate()
{
  calcMandelbrot(mField, mFieldTex);
  // renderMandelbrot(mField, mFieldTex);
  //mField.pullData();
}

void MandelbrotNode::onDraw()
{
  float scale = getScale();
  bool blocked = isBlocked();
  ImGui::BeginGroup();
  {
    if(mFieldSize != mField.size) { mField.create(mFieldSize); }
    mFieldHovered &= !blocked;
    
    // char fileName[256];
    // strcpy(fileName, mSaveFileName.c_str());
    // ImGui::SetNextItemWidth(222.0f*scale);
    // if(ImGui::InputText("File Name", fileName, 256)) { mSaveFileName = fileName; }
    // ImGui::SameLine();
    // if(ImGui::Button("Save")) { writeTexture(mSaveFileName, &mFieldTex); }

    mFileDialog->setGraph(mGraph);
    std::string savePath = mFileDialog->drawButton("Save##img", mFileDialog, "Save Image", DIALOG_SAVE, ".", {"*.png", "*.raw", "*.jpg", "*.bmp"});
    if(!savePath.empty())
      {
        mSavePath = savePath; mSaving = true;
        mFieldTex.pullData();
        mSaveThread = std::thread([&]()
                                  {
                                    auto t0 = std::chrono::high_resolution_clock::now();
                                    writeTexture(mSavePath, &mFieldTex);
                                    auto t1 = std::chrono::high_resolution_clock::now();
                                    mSaveTime = (std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count() / 1000000000.0);
                                  });
      }
    else if(mSaveThread.joinable())
      { mSaveThread.join(); mSaving = false; }

    if(!mSavePath.empty())
      {
        ImGui::SameLine();
        if(mSaving) { ImGui::Text("Writing image to %s...",  mSavePath.c_str()); }
        else        { ImGui::Text("Wrote image to %s (%.2f seconds)", mSavePath.c_str(), mSaveTime); }
      }
    
    mSettingForm->draw(scale, false, isBodyVisible());

    Vec2f p0 = ImGui::GetCursorScreenPos(); // top-left point after settings
    handleIO(p0);

    // draw cuda texture on screen
    mFieldTex.bind();
    ImGui::Image(mFieldTex.texId(), mDisplaySize*scale, Vec2f(0.0f, 0.0f), Vec2f(1.0f, 1.0f), ImColor(Vec4f(1,1,1,1)), Vec4f(0,0,0,1));
    mFieldTex.release();
    mFieldHovered = ImGui::IsItemHovered() && !blocked;
    
    // disable node interaction while interacting with field
    if(ImGui::IsMouseDown(ImGuiMouseButton_Left) && mFieldHovered && !ImGui::GetIO().KeyCtrl && !ImGui::GetIO().KeyCtrl) { mActive = true; }

    if(mDrawPath)
      {
        // mField.pullData();
        ImDrawList *drawList = ImGui::GetWindowDrawList();

        Vec2f mp = Vec2f(ImGui::GetMousePos());
        Vec2f gp = screenToField(mp, &p0);

        ImGui::PushClipRect(p0, p0+mDisplaySize*scale, true);

        for(int i = -mPathRad; i <= mPathRad; i++)
          for(int j = -mPathRad; j <= mPathRad; j++)
            {
              if((i*i + j*j) <= mPathRad*mPathRad)
                {
                  Vec2f ip = gp + Vec2f(i, j)*mPathSpacing;
                  Vec2f pLast = fieldToScreen(ip, &p0);
                  Complex<double> zLast((double)ip.x, (double)ip.y);
                  Complex<double> c((double)ip.x, (double)ip.y);
                  for(int i = 0; i < mField.params.maxIter; i++)
                    {
                      Complex<double> z = zLast*zLast + c;
                      Vec2f p = fieldToScreen(Vec2f(z.real, z.imag), &p0);
                      drawLine(drawList, pLast, p, Vec4f(1.0f, 0.0f, 0.0f, 1.0f), 2.0f, Vec4f(0.0f, 0.0f, 0.0f, 1.0f), 1.0f);
                      zLast = z; pLast = p;
                    }
                }
            }
        ImGui::PopClipRect();
      }
  }
  ImGui::EndGroup();
}

void MandelbrotNode::onResize(const Vec2f &dSize)
{
  mDisplaySize.y += dSize.y;
  mDisplaySize.x = mDisplaySize.y * ((float)mFieldSize.x / (float)mFieldSize.y);  // preserve fluid aspect ratio
}
