#include "fieldViewNode.hpp"

#include <imgui.h>

#include "setting.hpp"
#include "settingForm.hpp"
#include "cudaField.hpp"
#include "glfwKeys.hpp"
#include "imtools.hpp"


FieldViewNode::FieldViewNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Field View Node", true)
{
  mSettings.push_back(new Setting<bool> ("Settings Open", "settingsOpen", &mSettingsOpen));
  mSettings.push_back(new Setting<Vec2f>("Display Size",  "dispSize",     &mDisplaySize));
  
  // settings
  SettingBase *multSetting    = new Setting<float> ("Constant Multiplier", "texmult", &mTexMult, mTexMult);
  ((Setting<float>*)(multSetting))->setFormat(0.001, 0.1, "%.12f");
  ((Setting<float>*)(multSetting))->setMin(0.0);
  ((Setting<float>*)(multSetting))->setMax(1000000.0);

  SettingGroup *group = new SettingGroup("View Settings", "viewSettings", { multSetting }, true, false);
  mSettingForm = new SettingForm(180.0f, 150.0f);
  mSettingForm->add(group);
  mSettings.push_back(multSetting);
 
  // mTex.create(mTexSize);
  // outputs()[FIELDVIEWNODE_OUTPUT_TEX]->set(&mTex);
  // setTitle("Field View");
  // setMinSize(Vec2f(512.0f, 512.0f));
}

FieldViewNode::~FieldViewNode()
{
  mTex.destroy();
  if(mSettingForm) { delete mSettingForm; }
}

void FieldViewNode::resizeField(const Vec2f &fSize)
{
  if(fSize.x > 0 && fSize.y > 0 && (mTex.size != fSize))
    {
      //mTexSize = fSize;
      // mTex.destroy(); 
      mTex.create(fSize);
      //mDisplaySize.x = mDisplaySize.y * ((float)fSize.x / (float)fSize.y);
    }
}

Vec2f FieldViewNode::fieldToScreen(const Vec2f &fp, const Vec2f *p0)
{
  float scale = getScale();
  Vec2f vOffset(0,0); Vec2f vScale(1,1);
  Vec2f sp = ((Vec2f((fp.x - vOffset.x)/(vScale.x),
                     (fp.y - vOffset.y)/(vScale.y))*mDisplaySize)*scale +
              (p0 ? *p0 : Vec2f()));
  return sp;
}

Vec2f FieldViewNode::screenToField(const Vec2f &sp, const Vec2f *p0)
{
  float scale = getScale();
  Vec2f vOffset(0,0); Vec2f vScale(1,1);
  Vec2f tp = (sp - (p0 ? *p0 : Vec2f())) / scale;
  return Vec2f(vOffset.x + vScale.x * (tp.x/mDisplaySize.x),
               vOffset.y + vScale.y * (tp.y/mDisplaySize.y));
}

void FieldViewNode::onUpdate()
{
  //std::cout << "FIELDVIEWNODE UPDATE!\n";
  CudaFieldBase *field = inputs()[FIELDVIEWNODE_INPUT_FIELD]->get<CudaFieldBase>();
  mInputField = field;
  if(field && field->size.x > 0 && field->size.y > 0)
    {
      resizeField(field->size);
      if(field->size.x > 0 && field->size.y > 0 && mTex.size == field->size)
        {
          mTex.mult = mTexMult;
          switch(field->type())
            {
            case FIELDTYPE_INT:     renderTex<int>    (field, &mTex); break;
            case FIELDTYPE_INT2:    renderTex<int2>   (field, &mTex); break;
            case FIELDTYPE_INT3:    renderTex<int3>   (field, &mTex); break;
            case FIELDTYPE_INT4:    renderTex<int4>   (field, &mTex); break;
            case FIELDTYPE_FLOAT:   renderTex<float>  (field, &mTex); break;
            case FIELDTYPE_FLOAT2:  renderTex<float2> (field, &mTex); break;
            case FIELDTYPE_FLOAT3:  renderTex<float3> (field, &mTex); break;
            case FIELDTYPE_FLOAT4:  renderTex<float4> (field, &mTex); break;
            case FIELDTYPE_DOUBLE:  renderTex<double> (field, &mTex); break;
            case FIELDTYPE_DOUBLE2: renderTex<double2>(field, &mTex); break;
            case FIELDTYPE_DOUBLE3: renderTex<double3>(field, &mTex); break;
            case FIELDTYPE_DOUBLE4: renderTex<double4>(field, &mTex); break;
            }
          outputs()[FIELDVIEWNODE_OUTPUT_TEX]->set(&mTex);
        }
      else
        {
          std::cout << "====> WARNING(FieldViewNode): Bad field size(s)! (CField: " << mTex.size << ", input: " << field->size << ")\n";
          outputs()[FIELDVIEWNODE_OUTPUT_TEX]->set((CudaFieldBase*)nullptr);
        }
    }
}

void FieldViewNode::onDraw()
{
  float scale = getScale();
  Vec2f mp = ImGui::GetMousePos();
  
  mInputField = inputs()[FIELDVIEWNODE_INPUT_FIELD]->get<CudaFieldBase>();
  
  ImGui::BeginGroup();
  {
    ImGui::Text("Size: (%d x %d)", mTex.size.x, mTex.size.y);
    mSettingForm->draw(scale, false, isBodyVisible());
    Vec2f p0 = ImGui::GetCursorScreenPos();

    // draw texture
    ImGui::BeginGroup();
    if(mTex.size.x > 0 && mTex.size.y > 0)
      {
        mTex.bind();
        ImGui::Image(mTex.texId(), mDisplaySize*scale, Vec2f(0.0f, 0.0f), Vec2f(1.0f, 1.0f), ImColor(Vec4f(1,1,1,1)), Vec4f(0,0,0,1));
        mTex.release();
      }
    else
      {
        ImGui::GetWindowDrawList()->AddRectFilled(p0, p0+mDisplaySize*scale, ImColor(Vec4f(0.0f, 0.0f, 0.0f, 1.0f)));
        ImGui::InvisibleButton("##texDummy", mDisplaySize*scale, ImGuiButtonFlags_Disabled);
      }
    ImGui::EndGroup();
      
    Vec2f gp = screenToField(mp, &p0);
    bool  hovered = ImGui::IsItemHovered();
    std::string contextName = "##fieldViewContext";
    if(BeginContext(contextName, CONTEXT_WINDOW_RCLICK, hovered))
      {
        // mActive = true;
        //mContextForm->draw(1.0f, false);
        EndContext(true);
      }
    else
      {
        EndContext(false);
        if(mInputField && hovered && (ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT) || ImGui::IsKeyDown(GLFW_KEY_RIGHT_SHIFT)))
          { // draw tooltip
            BeginTooltip();
            {
              Vec2f tp = gp * mInputField->size;
              if(mInputField->allocated() && tp.x >= 0.0 && tp.x < mTex.size.x && tp.y >= 0.0 && tp.y < mTex.size.y)
                {
                  mInputField->pullData();
                  int i = ((int)tp.y)*mInputField->size.x + (int)tp.x;
                  ImGui::Text("Position:    < %s%.8f, %s%.8f >", (gp.x < 0.0 ? "-" : " "), abs(gp.x), (gp.y < 0.0 ? "-" : " "), abs(gp.y));
                  ImGui::Text("Index:       < %d, %d >", (int)tp.x, (int)tp.y);

                  std::string pval = "Value: " + mInputField->hDataStr(Vec2i(tp));
                  ImGui::TextUnformatted(pval.c_str());
                }
              else { ImGui::Text("< N/A >"); }
            }
            EndTooltip();
          }
      }
  }
  ImGui::EndGroup();
}

void FieldViewNode::onResize(const Vec2f &dSize)
{
  mDisplaySize.y += dSize.y;
  if(mTex.size.x > 0 && mTex.size.y > 0)  // preserve fluid aspect ratio
    { mDisplaySize.x = mDisplaySize.y * ((float)mTex.size.x / (float)mTex.size.y); }
  else
    { mDisplaySize.x = mDisplaySize.y; }
  mDisplaySize.x = std::max(mDisplaySize.x, 10.0f*getScale());
  mDisplaySize.y = std::max(mDisplaySize.y, 10.0f*getScale());
}









FieldChannelViewNode::FieldChannelViewNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Field View Node", true)
{
  // mSettings.push_back(new Setting<bool> ("Settings Open", "settingsOpen", &mSettingsOpen));
  mSettings.push_back(new Setting<Vec2f>("Display Size",  "dispSize",     &mDisplaySize));
  
  // settings
  // SettingBase *multSetting = new Setting<float> ("Constant Multiplier", "texmult", &mTexMult, mTexMult);
  // ((Setting<float>*)(multSetting))->setFormat(0.001, 0.1, "%.12f");
  // ((Setting<float>*)(multSetting))->setMin(0.0);
  // ((Setting<float>*)(multSetting))->setMax(1000000.0);
  // mSettings.push_back(multSetting);


  SettingBase *rRangeSetting = new Setting<Vec2f> ("R Range", "rRange", &mRRange, mRRange);
  mSettings.push_back(rRangeSetting);
  SettingBase *gRangeSetting = new Setting<Vec2f> ("G Range", "gRange", &mGRange, mGRange);
  mSettings.push_back(gRangeSetting);
  SettingBase *bRangeSetting = new Setting<Vec2f> ("B Range", "bRange", &mBRange, mBRange);
  mSettings.push_back(bRangeSetting);
  SettingBase *aRangeSetting = new Setting<Vec2f> ("A Range", "aRange", &mARange, mARange);
  mSettings.push_back(aRangeSetting);

  

  // SettingGroup *group = new SettingGroup("View Settings", "viewSettings", { multSetting }, true, false);
  // mSettingForm = new SettingForm(180.0f, 150.0f);
  // mSettingForm->add(group);
 
  // mTex.create(mTexSize);
  // outputs()[FIELDCHANNELVIEWNODE_OUTPUT_TEX]->set(&mTex);
  //setTitle("Field View");
  //setMinSize(Vec2f(512.0f, 512.0f));
}

FieldChannelViewNode::~FieldChannelViewNode()
{
  mTex.destroy();
}

void FieldChannelViewNode::resizeField(const Vec2f &fSize)
{
  if(fSize.x > 0 && fSize.y > 0 && (mTex.size != fSize))
    {
      mTex.create(fSize);
      mTex.map();
      fillTex(mTex.dData, mTex.size.x, mTex.size.y, float4{0.0f, 0.0f, 0.0f, 1.0f});
      mTex.unmap();
      //mDisplaySize.x = mDisplaySize.y * ((float)fSize.x / (float)fSize.y);
    }
}

Vec2f FieldChannelViewNode::fieldToScreen(const Vec2f &fp, const Vec2f *p0)
{
  float scale = getScale();
  Vec2f vOffset(0,0); Vec2f vScale(1,1);
  Vec2f sp = ((Vec2f((fp.x - vOffset.x)/(vScale.x),
                     (fp.y - vOffset.y)/(vScale.y))*mDisplaySize)*scale +
              (p0 ? *p0 : Vec2f()));
  return sp;
}

Vec2f FieldChannelViewNode::screenToField(const Vec2f &sp, const Vec2f *p0)
{
  float scale = getScale();
  Vec2f vOffset(0,0); Vec2f vScale(1,1);
  Vec2f tp = (sp - (p0 ? *p0 : Vec2f())) / scale;
  return Vec2f(vOffset.x + vScale.x * (tp.x/mDisplaySize.x),
               vOffset.y + vScale.y * (tp.y/mDisplaySize.y));
}

void FieldChannelViewNode::onUpdate()
{
  mRField = inputs()[FIELDCHANNELVIEWNODE_INPUT_RFIELD]->get<CudaFieldBase>();
  mGField = inputs()[FIELDCHANNELVIEWNODE_INPUT_GFIELD]->get<CudaFieldBase>();
  mBField = inputs()[FIELDCHANNELVIEWNODE_INPUT_BFIELD]->get<CudaFieldBase>();
  mAField = inputs()[FIELDCHANNELVIEWNODE_INPUT_AFIELD]->get<CudaFieldBase>();
  if((mRField && mRField->size.x > 0 && mRField->size.y > 0) ||
     (mGField && mGField->size.x > 0 && mGField->size.y > 0) ||
     (mBField && mBField->size.x > 0 && mBField->size.y > 0) ||
     (mAField && mAField->size.x > 0 && mAField->size.y > 0))
    {
      mTex.mult = 1.0f;
      
      bool success = true;
      if((mRField && mGField && mRField->size != mGField->size) ||
         (mGField && mBField && mGField->size != mBField->size) ||
         (mBField && mAField && mBField->size != mAField->size)) { std::cout << "WARNING: different field sizes!\n"; success = false; }

      std::vector<CudaFieldBase*> fields = { mRField, mGField, mBField, mAField };

      if(mRField) { resizeField(mRField->size); }
      if(mGField) { resizeField(mGField->size); }
      if(mBField) { resizeField(mBField->size); }
      if(mAField) { resizeField(mAField->size); }

      ColorChannel coveredChannels = CHANNEL_NONE;
      for(int i = 0; i < fields.size(); i++)
        {
          ColorChannel channels = (ColorChannel)(1 << i);
          if(!fields[i])
            {
              if(channels & CHANNEL_A) { fillTexChannel(&mTex, channels, 1.0); } // fill alpha channel with 1 if invalid input
              else                     { fillTexChannel(&mTex, channels, 0.0); } // fill other channels with 0 if invalid input
              continue;
            }
          
          // else if((int)channels & (int)coveredChannels)
          CudaFieldBase *f = fields[i];
          
          if(f && f->allocated() && f->size == mTex.size)
            {
              if((int)coveredChannels & (int)channels) { channels = CHANNEL_NONE; }

              if(channels != CHANNEL_NONE) // otherwise already covered (field connected to multiple channels)
                {
                  for(int j = i+1; j < fields.size(); j++)
                    { if(f == fields[j]) { channels = (ColorChannel)((int)channels | (1 << j)); } }
                  if(channels == CHANNEL_NONE) { continue; } // otherwise already covered (field connected to multiple channels)
                  
                  switch(f->type())
                    {
                    case FIELDTYPE_INT:     renderTexChannel<int>    (f, &mTex, channels, mRRange, mGRange, mBRange, mARange); break;
                    case FIELDTYPE_INT2:    renderTexChannel<int2>   (f, &mTex, channels, mRRange, mGRange, mBRange, mARange); break;
                    case FIELDTYPE_INT3:    renderTexChannel<int3>   (f, &mTex, channels, mRRange, mGRange, mBRange, mARange); break;
                    case FIELDTYPE_INT4:    renderTexChannel<int4>   (f, &mTex, channels, mRRange, mGRange, mBRange, mARange); break;
                    case FIELDTYPE_FLOAT:   renderTexChannel<float>  (f, &mTex, channels, mRRange, mGRange, mBRange, mARange); break;
                    case FIELDTYPE_FLOAT2:  renderTexChannel<float2> (f, &mTex, channels, mRRange, mGRange, mBRange, mARange); break;
                    case FIELDTYPE_FLOAT3:  renderTexChannel<float3> (f, &mTex, channels, mRRange, mGRange, mBRange, mARange); break;
                    case FIELDTYPE_FLOAT4:  renderTexChannel<float4> (f, &mTex, channels, mRRange, mGRange, mBRange, mARange); break;
                    case FIELDTYPE_DOUBLE:  renderTexChannel<double> (f, &mTex, channels, mRRange, mGRange, mBRange, mARange); break;
                    case FIELDTYPE_DOUBLE2: renderTexChannel<double2>(f, &mTex, channels, mRRange, mGRange, mBRange, mARange); break;
                    case FIELDTYPE_DOUBLE3: renderTexChannel<double3>(f, &mTex, channels, mRRange, mGRange, mBRange, mARange); break;
                    case FIELDTYPE_DOUBLE4: renderTexChannel<double4>(f, &mTex, channels, mRRange, mGRange, mBRange, mARange); break;
                    }
                  cudaDeviceSynchronize(); getLastCudaError("renderTexChannel!");
                  coveredChannels = (ColorChannel)((int)coveredChannels | (int)channels);
                }
            }
          else if(f)
            { std::cout << "====> WARNING(FieldChannelViewNode): Bad field size(s)! (value: " << mTex.size << ", input: " << f->size << ")\n"; }
        }
    }
  else if(mTex.allocated())
    {
      fillTexChannel(&mTex, (ColorChannel)((int)CHANNEL_R | (int)CHANNEL_G | (int)CHANNEL_B), 0.0);
      fillTexChannel(&mTex, CHANNEL_A,                                                        1.0);
    }

  outputs()[FIELDCHANNELVIEWNODE_OUTPUT_TEX]->set(mTex.allocated() ? &mTex : (CudaFieldBase*)nullptr);
}

void FieldChannelViewNode::onDraw()
{
  float scale = getScale();
  Vec2f mp = ImGui::GetMousePos();

  float inputW = 130.0*scale;

  ImGui::BeginGroup();
  {
    // settings collapsing header
    ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth);
    ImGui::SetNextTreeNodeOpen(mSettingsOpen);
    if(ImGui::CollapsingHeader("Settings", nullptr, flags))
      {
        mSettingsOpen = true;
        ImGui::BeginGroup();
        {
          ImGui::TextUnformatted("R");
          ImGui::SameLine(); ImGui::TextUnformatted("MIN");
          ImGui::SameLine(); ImGui::SetNextItemWidth(inputW);
          if(ImGui::InputFloat("##rmin", &mRRange.x, 0.01f, 0.1f, "%.6f")) { if(mRRange.x > mRRange.y) { mRRange.x = mRRange.y; } }
          ImGui::SameLine(); ImGui::TextUnformatted("MAX");
          ImGui::SameLine(); ImGui::SetNextItemWidth(inputW);
          if(ImGui::InputFloat("##rmax", &mRRange.y, 0.01f, 0.1f, "%.6f")) { if(mRRange.y < mRRange.x) { mRRange.y = mRRange.x; } }
          ImGui::SameLine(); ImGui::Text("%+.6f | %.6f", (mRRange.x+mRRange.y)/2.0f, (mRRange.y - mRRange.x));
          ImGui::TextUnformatted("G");
          ImGui::SameLine(); ImGui::TextUnformatted("MIN");
          ImGui::SameLine(); ImGui::SetNextItemWidth(inputW);
          if(ImGui::InputFloat("##gmin", &mGRange.x, 0.01f, 0.1f, "%.6f")) { if(mGRange.x > mGRange.y) { mGRange.x = mGRange.y; } }
          ImGui::SameLine(); ImGui::TextUnformatted("MAX");
          ImGui::SameLine(); ImGui::SetNextItemWidth(inputW);
          if(ImGui::InputFloat("##gmax", &mGRange.y, 0.01f, 0.1f, "%.6f")) { if(mGRange.y < mGRange.x) { mGRange.y = mGRange.x; } }
          ImGui::SameLine(); ImGui::Text("%+.6f | %.6f", (mGRange.x+mGRange.y)/2.0f, (mGRange.y - mGRange.x));
          ImGui::TextUnformatted("B");
          ImGui::SameLine(); ImGui::TextUnformatted("MIN");
          ImGui::SameLine(); ImGui::SetNextItemWidth(inputW);
          if(ImGui::InputFloat("##bmin", &mBRange.x, 0.01f, 0.1f, "%.6f")) { if(mBRange.x > mBRange.y) { mBRange.x = mBRange.y; } }
          ImGui::SameLine(); ImGui::TextUnformatted("MAX");
          ImGui::SameLine(); ImGui::SetNextItemWidth(inputW);
          if(ImGui::InputFloat("##bmax", &mBRange.y, 0.01f, 0.1f, "%.6f")) { if(mBRange.y < mBRange.x) { mBRange.y = mBRange.x; } }
          ImGui::SameLine(); ImGui::Text("%+.6f | %.6f", (mBRange.x+mBRange.y)/2.0f, (mBRange.y - mBRange.x));
          ImGui::TextUnformatted("A");
          ImGui::SameLine(); ImGui::TextUnformatted("MIN");
          ImGui::SameLine(); ImGui::SetNextItemWidth(inputW);
          if(ImGui::InputFloat("##amin", &mARange.x, 0.01f, 0.1f, "%.6f")) { if(mARange.x > mARange.y) { mARange.x = mARange.y; } }
          ImGui::SameLine(); ImGui::TextUnformatted("MAX");
          ImGui::SameLine(); ImGui::SetNextItemWidth(inputW);
          if(ImGui::InputFloat("##amax", &mARange.y, 0.01f, 0.1f, "%.6f")) { if(mARange.y < mARange.x) { mARange.y = mARange.x; } }
          ImGui::SameLine(); ImGui::Text("%+.6f | %.6f", (mARange.x+mARange.y)/2.0f, (mARange.y - mARange.x));
        }
        ImGui::EndGroup();
        // ssize = Vec2f(ImGui::GetItemRectMax()) - ImGui::GetItemRectMin();
      }
    else { mSettingsOpen = false; }
    Vec2f ssize = Vec2f(ImGui::GetItemRectMax()) - ImGui::GetItemRectMin();

    // center display in node
    if(ssize.x >= mDisplaySize.x*scale) { ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f((ssize.x - mDisplaySize.x*scale)/2.0f, 0.0f)); }

    Vec2f p0 = ImGui::GetCursorScreenPos();
  
    // draw texture
    if(mTex.size.x > 0 && mTex.size.y > 0)
      {
        mTex.bind();
        ImGui::Image(mTex.texId(), mDisplaySize*scale, Vec2f(0.0f, 0.0f), Vec2f(1.0f, 1.0f), ImColor(Vec4f(1,1,1,1)), Vec4f(0,0,0,1));
        mTex.release();
      }
    else
      {
        ImGui::GetWindowDrawList()->AddRectFilled(p0, p0+mDisplaySize*scale, ImColor(Vec4f(0.0f, 0.0f, 0.0f, 1.0f)));
        ImGui::InvisibleButton("##texDummy", mDisplaySize*scale, ImGuiButtonFlags_Disabled);
      }
      
    // Vec2f gp = screenToField(mp, &p0);
    // bool  hovered = ImGui::IsItemHovered();
    // std::string contextName = "##fieldViewContext";
    // if(BeginContext(contextName, CONTEXT_WINDOW_RCLICK, hovered))
    //   {
    //     mActive = true;
    //     //mContextForm->draw(1.0f, false);
    //     EndContext(true);
    //   }
    // else
    //   {
    //     EndContext(false);
    //     if(mInputField && hovered && (ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT) || ImGui::IsKeyDown(GLFW_KEY_RIGHT_SHIFT)))
    //       { // draw tooltip
    //         BeginTooltip();
    //         {
    //           Vec2f tp = gp * mInputField->size;
    //           if(mInputField->allocated() && tp.x >= 0.0 && tp.x < mTex.size.x && tp.y >= 0.0 && tp.y < mTex.size.y)
    //             {
    //               mInputField->pullData();
    //               int i = ((int)tp.y)*mInputField->size.x + (int)tp.x;
    //               ImGui::Text("Position:    < %s%.8f, %s%.8f >", (gp.x < 0.0 ? "-" : " "), abs(gp.x), (gp.y < 0.0 ? "-" : " "), abs(gp.y));
    //               ImGui::Text("Index:       < %d, %d >", (int)tp.x, (int)tp.y);

    //               std::string pval = "Value: " + mInputField->hDataStr(Vec2i(tp));
    //               ImGui::TextUnformatted(pval.c_str());
    //             }
    //           else { ImGui::Text("< N/A >"); }
    //         }
    //         EndTooltip();
    //       }
    //   }
  }
  ImGui::EndGroup();
}

void FieldChannelViewNode::onResize(const Vec2f &dSize)
{
  mDisplaySize.y += dSize.y;
  if(mTex.size.x > 0 && mTex.size.y > 0)  // preserve fluid aspect ratio
    { mDisplaySize.x = mDisplaySize.y * ((float)mTex.size.x / (float)mTex.size.y); }
  else
    { mDisplaySize.x = mDisplaySize.y; }
  mDisplaySize.x = std::max(mDisplaySize.x, 10.0f*getScale());
  mDisplaySize.y = std::max(mDisplaySize.y, 10.0f*getScale());
}
