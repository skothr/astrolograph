#include "fftNode.hpp"
using namespace astro;
// using namespace quantum;

#include "cutools.hpp"
#include "setting.hpp"
#include "settingForm.hpp"
#include "cudaField.hpp"
#include "glfwKeys.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <cstring>


FFTNode::FFTNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "FFT Node")
{
  // settings
  mSettings.push_back(new Setting<bool> ("Settings Open", "settingsOpen", &mSettingsOpen));
  mSettings.push_back(new Setting<Vec2f>("Display Size",  "dispSize",     &mDisplaySize));
  
  //SettingBase *debugSetting     = new Setting<bool>  ("Debug",       "debug",      &mDebug,      false);
  SettingBase *inverseSetting   = new Setting<bool>  ("Inverse FFT", "inverseFft", &mInverseFft, false);
  SettingBase *preShiftSetting  = new Setting<bool>  ("Pre Shift",   "preShift",   &mPreShift,   false);
  SettingBase *preScaleSetting  = new Setting<bool>  ("Pre Scale",   "preScale",   &mPreScale,   false);
  SettingBase *postShiftSetting = new Setting<bool>  ("Post Shift",  "postShift",  &mPostShift,  true);
  SettingBase *postScaleSetting = new Setting<bool>  ("Post Scale",  "postScale",  &mPostScale,  true);
  SettingBase *multSetting      = new Setting<float> ("Multiplier",  "mult",       &mMult,       1.0f);
  ((Setting<float>*)(multSetting))->setFormat(0.001, 0.1, "%.12f");
  SettingBase *typeSetting      = new ComboSetting   ("Render Type", "renderType", (int*)&mRenderType, RENDER_TYPE_NAMES, (int)RENDER_MAGNITUDE);

  mSettings.push_back(typeSetting);
  mSettings.push_back(inverseSetting);
  mSettings.push_back(preShiftSetting);
  mSettings.push_back(preScaleSetting);
  mSettings.push_back(postShiftSetting);
  mSettings.push_back(postScaleSetting);
  mSettings.push_back(multSetting);
  
  mField.create(mInputSize);
  
  mFftField.create(mInputSize);
  cufftPlan2d(&mFftPlan, mInputSize.x, mInputSize.y, CUFFT_C2C);
  
  outputs()[FFTNODE_OUTPUT_FIELD]->set(&mField);
  setTitle("FFT");
  // setMinSize(Vec2f(222.0f, 42.0f));
}

FFTNode::~FFTNode()
{
  mField.destroy(); mFftField.destroy(); cufftDestroy(mFftPlan);
}

void FFTNode::resizeField(const Vec2f &fSize)
{
  if(fSize.x > 0 && fSize.y > 0 && (mInputSize != fSize || mField.size != fSize))
    {
      mInputSize = fSize;
      mField.create(mInputSize); mFftField.create(mInputSize);
      cufftDestroy(mFftPlan); cufftPlan2d(&mFftPlan, mInputSize.x, mInputSize.y, CUFFT_C2C);
      //mDisplaySize.x = mDisplaySize.y * ((float)fSize.x / (float)fSize.y);
    }
}

void FFTNode::onUpdate()
{
  CudaFieldBase *field = inputs()[FFTNODE_INPUT_FIELD]->get<CudaFieldBase>();
  if(field)
    {
      resizeField(field->size);
      if(field->size.x > 0 && field->size.y > 0 && mField.size == field->size && mInputSize == field->size)
        {
          if(field->isTexture())
            {
              CudaFieldTex *f = reinterpret_cast<CudaFieldTex*>(field);
              if(f) { loadTexMagnitude2(*f, mFftField); }
              else  { std::cout << "====> WARNING(FFTNode::onUpdate()): Failed to reinterpret field as CudaFieldTex!\n"; }
            }
          else if(field->type() == FIELDTYPE_FLOAT)
            {
              CudaField<float> *f = reinterpret_cast<CudaField<float>*>(field);
              if(f) { loadFieldMagnitude(*f, mFftField); }
              else  { std::cout << "====> WARNING(FFTNode::onUpdate()): Failed to reinterpret field as CudaFieldTex!\n"; }
            }
          else if(field->type() == FIELDTYPE_FLOAT2)
            {
              CudaField<float2> *f = reinterpret_cast<CudaField<float2>*>(field);
              if(f) { cudaMemcpy(mFftField.dData, f->dData, field->dataSize*field->typeSize, cudaMemcpyDeviceToDevice); }
              else  { std::cout << "====> WARNING(FFTNode::onUpdate()): Failed to reinterpret field as CudaField<float2>!\n"; }
            }
          
          CudaField<float2> *curr = &mFftField;
          CudaField<float2> *next = &mField;
          if(mPreShift || mPreScale) { fftShift(*curr, *next, mPreShift, mPreScale); std::swap(curr, next); }
          
          cufftExecC2C(mFftPlan, (cufftComplex*)curr->dData, (cufftComplex*)next->dData, (mInverseFft ? CUFFT_INVERSE : CUFFT_FORWARD));
          std::swap(curr, next);
          
          if(mPostShift || mPostScale) { fftShift(*curr, *next, mPostShift, mPostScale); std::swap(curr, next); }

          outputs()[FFTNODE_OUTPUT_FIELD]->set(curr);
          
          // switch(mRenderType)
          //   {
          //   case RENDER_REAL:      renderFft_real     (*curr, mFieldTex); break;
          //   case RENDER_IMAGINARY: renderFft_imag     (*curr, mFieldTex); break;
          //   case RENDER_MAGNITUDE: renderFft_magnitude(*curr, mFieldTex); break;
          //   case RENDER_PHASE:     renderFft_phase    (*curr, mFieldTex); break;
          //   case RENDER_COMBINED:  renderFft_combined (*curr, mFieldTex); break;
          //   }
        }
      else
        {
          std::cout << "====> WARNING: Bad FFT/field size(s)! (CField: " << mField.size << ", input: " << field->size << ")\n";
          mField.destroy();
          mFftField.destroy();
          mInputSize = Vec2i(0,0);
        }
    }
}

void FFTNode::onDraw()
{
  float scale = getScale();
  ImGui::BeginGroup();
  {
    char fileName[256];
    strcpy(fileName, mSaveFileName.c_str());
    
    ImGui::Text("Size: (%d x %d)", mInputSize.x, mInputSize.y);
    
    // ImGui::SameLine();
    // ImGui::SetNextItemWidth(222.0f*scale);
    // if(ImGui::InputText("File Name", fileName, 256)) { mSaveFileName = fileName; }
    // ImGui::SameLine();
    // if(ImGui::Button("Save")) { writeTexture(mSaveFileName, &mFieldTex); }
    //mSettingForm->draw(scale, false, isBodyVisible());

    ImGui::Checkbox("Debug", &mDebug);

    
    // inline bool ComboSetting::onDraw(float scale, bool busy, bool &changed, bool visible)
    
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, mPlacing); // disable interaction while placing node
    if(ImGui::BeginCombo("Render Type", RENDER_TYPE_NAMES[mRenderType].c_str()))
      {
        ImGui::SetWindowFontScale(scale);
        for(int i = 0; i < RENDER_TYPE_NAMES.size(); i++)
          {
            if(ImGui::Selectable(((i == (int)mRenderType ? "* " : "") + RENDER_TYPE_NAMES[i]).c_str()))
              { mRenderType = (RenderType)i; }
          }
        ImGui::EndCombo();
      }
    ImGui::PopItemFlag();

    ImGui::Checkbox("Inverse",     &mInverseFft);
    ImGui::Checkbox("Pre-Shift",   &mPreShift); ImGui::SameLine(); ImGui::Checkbox("Post-Shift", &mPostShift);
    ImGui::Checkbox("Pre-Scale",   &mPreScale); ImGui::SameLine(); ImGui::Checkbox("Post-Scale", &mPostScale);
    ImGui::SetNextItemWidth(120*scale); ImGui::InputFloat("Multiplier", &mMult, 0.01, 0.1, "%.4f");
        
    // // draw image of FFT
    // if(mDebug)
    //   {
        
    //   }
    // else
    //   {
    //     mFieldTex.bind();
    //     ImGui::Image(mFieldTex.texId(), mDisplaySize*scale, Vec2f(0.0f, 0.0f), Vec2f(1.0f, 1.0f), ImColor(Vec4f(1,1,1,1)), Vec4f(0,0,0,1));
    //     mFieldTex.release();
    //   }
  }
  ImGui::EndGroup();
}

void FFTNode::onResize(const Vec2f &dSize)
{
  // mDisplaySize.y += dSize.y;
  // mDisplaySize.x = mDisplaySize.y * ((float)mInputSize.x / (float)mInputSize.y);  // preserve fluid aspect ratio
}
