#include "fieldChannels.hpp"

#include "setting.hpp"
#include "cudaField.hpp"
#include "field-operators.h"
#include <imgui.h>


ChannelCombineNode::ChannelCombineNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Field Combine Node", false)
{
  mSettings.push_back(new Setting<int>("Input Choice",   "inChoice",   (int*)(&mInputChoice), (int)mInputChoice));
  mSettings.push_back(new Setting<int>("Input Channels", "inChannels", &mInputChannels,       mInputChannels));
  for(int i = 0; i < (int)FIELDTYPE_COUNT; i++) { mTypeChoices.push_back(to_string((FieldType)i)); }
  setTitle("Combine");
}

ChannelCombineNode::~ChannelCombineNode()
{
  if(mResult) { mResult->destroy(); delete mResult; mResult = nullptr; }
  // if(mResultY) { mResultY->destroy(); delete mResultY; mResultY = nullptr; }
  // if(mResultZ) { mResultZ->destroy(); delete mResultZ; mResultZ = nullptr; }
  // if(mResultW) { mResultW->destroy(); delete mResultW; mResultW = nullptr; }
}

void ChannelCombineNode::resizeField(const Vec2f &fSize)
{
  // if(fSize.x > 0 && fSize.y > 0 && ((mInputChannels > 0 && mResultX && fSize != mResultX->size) ||
  //                                   (mInputChannels > 1 && mResultY && fSize != mResultY->size) ||
  //                                   (mInputChannels > 2 && mResultZ && fSize != mResultZ->size) ||
  //                                   (mInputChannels > 3 && mResultW && fSize != mResultW->size)))
  
  if(fSize.x > 0 && fSize.y > 0 && ((mResult && fSize != mResult->size)))
    {
      mResult->create(fSize);
      // if(mInputChannels > 1) { mResultY->create(fSize); }
      // if(mInputChannels > 2) { mResultZ->create(fSize); }
      // if(mInputChannels > 3) { mResultW->create(fSize); }
    }
}

void ChannelCombineNode::onUpdate()
{
  const auto &inFields  = inputs();
  const auto &outFields = outputs();

  if(inFields.size() != mInputChannels)
    {
      mInputChannels = numChannels((FieldType)mInputChoice);
      
      while(inputs().size() > mInputChannels) { removeInput(inputs().size()-1); }
      while(inputs().size() < mInputChannels)
        {
          std::string dimStr = "";
          switch(inputs().size())
            {
            case 0:  dimStr = "X"; break;
            case 1:  dimStr = "Y"; break;
            case 2:  dimStr = "Z"; break;
            case 3:  dimStr = "W"; break;
            default: dimStr = "?";
            }
          addInput(new Connector<CudaFieldBase>(dimStr + " Field"));
        }
    }


  bool xf = false;
  bool yf = false;
  bool zf = false;
  bool wf = false;
  
  if(mInputChannels > 0) { mInputFieldX = inFields[0]->get<CudaFieldBase>(); xf = (bool)mInputFieldX; }
  else { mInputFieldX = nullptr; xf = true; }
  if(mInputChannels > 1) { mInputFieldY = inFields[1]->get<CudaFieldBase>(); yf = (bool)mInputFieldY; }
  else { mInputFieldY = nullptr; yf = true; }
  if(mInputChannels > 2) { mInputFieldZ = inFields[2]->get<CudaFieldBase>(); zf = (bool)mInputFieldZ; }
  else { mInputFieldZ = nullptr; zf = true; }
  if(mInputChannels > 3) { mInputFieldW = inFields[3]->get<CudaFieldBase>(); wf = (bool)mInputFieldW; }
  else { mInputFieldW = nullptr; wf = true; }

  // if(mOutputChannels > 0 && !mResultFieldX)
  //   {
  //     mResultX = new CudaField<(FieldType)mOutputChannels>
  //   }
  
  if(xf && yf && zf && wf)
    {
      if(!mResult || (numChannels(mResult->type()) != mInputChannels))
        {
          if(mResult) { mResult->destroy(); delete mResult; mResult = nullptr; }
          // if(field->isTexture()) { mResult = new CudaFieldTex(); }
          // else { mResult = makeCudaField(field->type()); }
          mResult = makeCudaField((FieldType)mInputChoice);
          resizeField(mInputFieldX->size);
        }
      else if(mResult)
        { resizeField(mInputFieldX->size); }
      
      if(mResult && mResult->allocated() && mResult->size == mInputFieldX->size)
        {
          switch(mInputChannels)
            {
            case 1:  combineChannels(mInputFieldX, mResult); break;
            case 2:  combineChannels(mInputFieldX, mInputFieldY, mResult); break;
            case 3:  combineChannels(mInputFieldX, mInputFieldY, mInputFieldZ, mResult); break;
            case 4:  combineChannels(mInputFieldX, mInputFieldY, mInputFieldZ, mInputFieldW, mResult); break;
            default: break;
            }
          outputs()[FIELDCOMBINENODE_OUTPUT_FIELD]->set(mResult);
        }
      else
        {
          std::cout << "====> WARNING(ChannelCombineientNode): ";
          if(mResult && mResult->size != Vec2i(0,0))
            {
              std::cout << "  result: "   << mResult->size << " | " << mResult->dataSize << " | " << mResult->typeSize << " | " << mResult->type() << "\n";
                        // << "\n  input: "  << field->size   << " | " << field->dataSize   << " | " << field->typeSize   << " | " << field->type() << "\n";
            }
          else
            {
              std::cout<< "  result --> "   << (mResult ? mResult->size.toString() : "NULL") << "\n";
                       // << "\n  input --> "   << (field  ? field->size.toString()   : "NULL") << "\n";
            }
          
          //outputs()[FIELDCOMBINENODE_OUTPUT_FIELD]->set((CudaFieldBase*)nullptr);
        }
    }
}

void ChannelCombineNode::onDraw()
{
  float scale = getScale();
  
  Vec2i fSize = (mInputFieldX ? mInputFieldX->size : Vec2i());  
  std::stringstream ss; ss << (mInputFieldX ? mInputFieldX->type() : FIELDTYPE_INVALID);

  Vec2f p0 = ImGui::GetCursorScreenPos();  
  ImGui::Text("Input: (%d x %d)<%s>", fSize.x, fSize.y, ss.str().c_str());

  bool changed = false;
  if(ImGui::BeginCombo("Input Type", mTypeChoices[mInputChoice].c_str()))
    {
      ImGui::SetWindowFontScale(scale);
      for(int i = 0; i < mTypeChoices.size(); i++)
        {
          std::string &s = mTypeChoices[i];
          if(ImGui::Selectable(((i == mInputChoice ? "* " : "") + mTypeChoices[i]).c_str()))
            { changed = (mInputChoice != i); mInputChoice = i; }
        }
      ImGui::EndCombo();
    }
  if(changed)
    {
      mInputChannels = numChannels((FieldType)mInputChoice);
      
      while(inputs().size() > mInputChannels) { removeInput(inputs().size()-1); }
      while(inputs().size() < mInputChannels)
        {
          std::string dimStr = "";
          switch(inputs().size())
            {
            case 0:  dimStr = "X"; break;
            case 1:  dimStr = "Y"; break;
            case 2:  dimStr = "Z"; break;
            case 3:  dimStr = "W"; break;
            default: dimStr = "?";
            }
          addInput(new Connector<CudaFieldBase>(dimStr + " Field"));
        }
    }
  changed = false;
  // if(ImGui::BeginCombo(("Output Type").c_str(), mTypeChoices[mOutputChoice].c_str()))
  //   {
  //     ImGui::SetWindowFontScale(scale);
  //     for(int i = 0; i < mTypeChoices.size(); i++)
  //       {
  //         std::string &s = mTypeChoices[i];
  //         if(ImGui::Selectable(((i == mOutputChoice ? "* " : "") + mTypeChoices[i]).c_str()))
  //           { changed = (mOutputChoice != i); mOutputChoice = i; }
  //       }
  //     ImGui::EndCombo();
  //   }
  // if(changed)
  //   {
  //     mOutputChannels = numChannels((FieldType)mOutputChoice);
  //     while(outputs().size() > mOutputChannels) { removeOutput(outputs().size()-1); }
  //     while(outputs().size() < mOutputChannels)
  //       {
  //         std::string dimStr = "";
  //         switch(outputs().size())
  //           {
  //           case 0:  dimStr = "X"; break;
  //           case 1:  dimStr = "Y"; break;
  //           case 2:  dimStr = "Z"; break;
  //           case 3:  dimStr = "W"; break;
  //           default: dimStr = "?";
  //           }
  //         addOutput(new NodeConnector<CudaFieldBase>(dimStr + " Field"));
  //       }
  //   }

  //drawNabla(ImGui::GetWindowDrawList(), titlePos(), Vec2f(titleSize().y, titleSize().y)*0.9f, Vec4f(1.0f, 1.0f, 1.0f, 1.0f), 2.0f*scale);
}


void ChannelCombineNode::onLoad()
{
  if(inputs().size() != mInputChannels)
    {
      mInputChannels = numChannels((FieldType)mInputChoice);
      
      while(inputs().size() > mInputChannels) { removeInput(inputs().size()-1); }
      while(inputs().size() < mInputChannels)
        {
          std::string dimStr = "";
          switch(inputs().size())
            {
            case 0:  dimStr = "X"; break;
            case 1:  dimStr = "Y"; break;
            case 2:  dimStr = "Z"; break;
            case 3:  dimStr = "W"; break;
            default: dimStr = "?";
            }
          addInput(new Connector<CudaFieldBase>(dimStr + " Field"));
        }
    }

}














ChannelSplitNode::ChannelSplitNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Channel Split Node", false)
{
  mSettings.push_back(new Setting<int>("Output Choice",   "outChoice",   (int*)(&mOutputChoice), (int)mOutputChoice));
  mSettings.push_back(new Setting<int>("Output Channels", "outChannels", &mOutputChannels,       mOutputChannels));
  
  for(int i = 0; i < (int)FIELDTYPE_COUNT; i++) { mTypeChoices.push_back(to_string((FieldType)i)); }
  setTitle("Split");
}

ChannelSplitNode::~ChannelSplitNode()
{
  if(mResultX) { mResultX->destroy(); delete mResultX; mResultX = nullptr; }
  if(mResultY) { mResultY->destroy(); delete mResultY; mResultY = nullptr; }
  if(mResultZ) { mResultZ->destroy(); delete mResultZ; mResultZ = nullptr; }
  if(mResultW) { mResultW->destroy(); delete mResultW; mResultW = nullptr; }
}

void ChannelSplitNode::resizeField(const Vec2f &fSize)
{
  if(fSize.x > 0 && fSize.y > 0 && ((mOutputChannels > 0 && mResultX && fSize != mResultX->size) ||
                                    (mOutputChannels > 1 && mResultY && fSize != mResultY->size) ||
                                    (mOutputChannels > 2 && mResultZ && fSize != mResultZ->size) ||
                                    (mOutputChannels > 3 && mResultW && fSize != mResultW->size)))
  // if(fSize.x > 0 && fSize.y > 0 && ((mResult && fSize != mResult->size)))
    {
      if(mOutputChannels > 0) { mResultX->create(fSize); }
      if(mOutputChannels > 1) { mResultY->create(fSize); }
      if(mOutputChannels > 2) { mResultZ->create(fSize); }
      if(mOutputChannels > 3) { mResultW->create(fSize); }
    }
}

void ChannelSplitNode::onUpdate()
{
  const auto &inFields  = inputs();
  const auto &outFields = outputs();

  if(outFields.size() != mOutputChannels)
    {
      mOutputChannels = numChannels((FieldType)mOutputChoice);
      while(outputs().size() > mOutputChannels) { removeOutput(outputs().size()-1); }
      while(outputs().size() < mOutputChannels)
        {
          std::string dimStr = "";
          switch(outputs().size())
            {
            case 0:  dimStr = "X"; break;
            case 1:  dimStr = "Y"; break;
            case 2:  dimStr = "Z"; break;
            case 3:  dimStr = "W"; break;
            default: dimStr = "?";
            }
          addOutput(new Connector<CudaFieldBase>(dimStr + " Field"));
        }
    }
  
  mInputField = inFields[0]->get<CudaFieldBase>();
  if(mInputField)
    {
      if((mOutputChannels > 0 && !mResultX) ||
         (mOutputChannels > 1 && !mResultY) ||
         (mOutputChannels > 2 && !mResultZ) ||
         (mOutputChannels > 3 && !mResultW))// || (numChannels(mResult->type()) != mOutputChannels))
        {
          if(mResultX) { mResultX->destroy(); delete mResultX; mResultX = nullptr; }
          if(mResultY) { mResultY->destroy(); delete mResultY; mResultY = nullptr; }
          if(mResultZ) { mResultZ->destroy(); delete mResultZ; mResultZ = nullptr; }
          if(mResultW) { mResultW->destroy(); delete mResultW; mResultW = nullptr; }
          if(mOutputChannels > 0) { mResultX = makeCudaField(baseType((FieldType)mOutputChoice)); }
          if(mOutputChannels > 1) { mResultY = makeCudaField(baseType((FieldType)mOutputChoice)); }
          if(mOutputChannels > 2) { mResultZ = makeCudaField(baseType((FieldType)mOutputChoice)); }
          if(mOutputChannels > 3) { mResultW = makeCudaField(baseType((FieldType)mOutputChoice)); }
          resizeField(mInputField->size);
        }
      else if((mOutputChannels < 1 || mResultX) &&
              (mOutputChannels < 2 || mResultY) &&
              (mOutputChannels < 3 || mResultZ) &&
              (mOutputChannels < 4 || mResultW))
        { resizeField(mInputField->size); }
      
      if((mOutputChannels > 0 && mResultX && mResultX->allocated() && mResultX->size == mInputField->size) ||
         (mOutputChannels > 1 && mResultY && mResultY->allocated() && mResultY->size == mInputField->size) ||
         (mOutputChannels > 2 && mResultZ && mResultZ->allocated() && mResultZ->size == mInputField->size) ||
         (mOutputChannels > 3 && mResultW && mResultW->allocated() && mResultW->size == mInputField->size))
        {
          switch(mOutputChannels)
            {
            case 1:  splitChannels(mInputField, mResultX); break;
            case 2:  splitChannels(mInputField, mResultX, mResultY); break;
            case 3:  splitChannels(mInputField, mResultX, mResultY, mResultZ); break;
            case 4:  splitChannels(mInputField, mResultX, mResultY, mResultZ, mResultW); break;
            default: break;
            }
          if(mOutputChannels > 0) { outputs()[0]->set(mResultX); }
          if(mOutputChannels > 1) { outputs()[1]->set(mResultY); }
          if(mOutputChannels > 2) { outputs()[2]->set(mResultZ); }
          if(mOutputChannels > 3) { outputs()[3]->set(mResultW); }
        }
      else
        {
          std::cout << "====> WARNING(ChannelSplitNode): NULL / bad size!\n";
          if(mOutputChannels > 0) { outputs()[0]->set((CudaFieldBase*)nullptr); }
          if(mOutputChannels > 1) { outputs()[1]->set((CudaFieldBase*)nullptr); }
          if(mOutputChannels > 2) { outputs()[2]->set((CudaFieldBase*)nullptr); }
          if(mOutputChannels > 3) { outputs()[3]->set((CudaFieldBase*)nullptr); }
        }
    }
}

void ChannelSplitNode::onDraw()
{
  float scale = getScale();
  
  Vec2i fSize = (mInputField ? mInputField->size : Vec2i());  
  std::stringstream ss; ss << (mInputField ? mInputField->type() : FIELDTYPE_INVALID);

  Vec2f p0 = ImGui::GetCursorScreenPos();  
  ImGui::Text("Input: (%d x %d)<%s>", fSize.x, fSize.y, ss.str().c_str());

  bool changed = false;
  if(ImGui::BeginCombo("Output Type", mTypeChoices[mOutputChoice].c_str()))
    {
      ImGui::SetWindowFontScale(scale);
      for(int i = 0; i < mTypeChoices.size(); i++)
        {
          std::string &s = mTypeChoices[i];
          if(ImGui::Selectable(((i == mOutputChoice ? "* " : "") + mTypeChoices[i]).c_str()))
            { changed = (mOutputChoice != i); mOutputChoice = i; }
        }
      ImGui::EndCombo();
    }
  if(changed)
    {
      mOutputChannels = numChannels((FieldType)mOutputChoice);
      while(outputs().size() > mOutputChannels) { removeOutput(outputs().size()-1); }
      while(outputs().size() < mOutputChannels)
        {
          std::string dimStr = "";
          switch(outputs().size())
            {
            case 0:  dimStr = "X"; break;
            case 1:  dimStr = "Y"; break;
            case 2:  dimStr = "Z"; break;
            case 3:  dimStr = "W"; break;
            default: dimStr = "?";
            }
          addOutput(new Connector<CudaFieldBase>(dimStr + " Field"));
        }
    }
  changed = false;
}


void ChannelSplitNode::onLoad()
{

  if(outputs().size() != mOutputChannels)
    {
      mOutputChannels = numChannels((FieldType)mOutputChoice);
      while(outputs().size() > mOutputChannels) { removeOutput(outputs().size()-1); }
      while(outputs().size() < mOutputChannels)
        {
          std::string dimStr = "";
          switch(outputs().size())
            {
            case 0:  dimStr = "X"; break;
            case 1:  dimStr = "Y"; break;
            case 2:  dimStr = "Z"; break;
            case 3:  dimStr = "W"; break;
            default: dimStr = "?";
            }
          addOutput(new Connector<CudaFieldBase>(dimStr + " Field"));
        }
    }

}
