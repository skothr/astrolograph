#include "fieldOperators.hpp"

#include <imgui.h>

#include "setting.hpp"
#include "settingForm.hpp"
#include "cudaField.hpp"
#include "glfwKeys.hpp"
#include "field-operators.h"
#include "cutools.hpp"
#include "cutools.cuh"

FieldMultNode::FieldMultNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Field Mult Node", false)
{
  // settings
  SettingBase *multSetting    = new Setting<float> ("Multiplier", "mult", &mMult, 1.0f);
  ((Setting<float>*)(multSetting))->setFormat(0.001, 0.1, "%.12f");
  ((Setting<float>*)(multSetting))->setMin(0.0);
  ((Setting<float>*)(multSetting))->setMax(1000000.0);
  SettingGroup *group = new SettingGroup("View Settings", "viewSettings", { multSetting }, true, false);
  mSettingForm = new SettingForm(150.0f, 150.0f);
  mSettingForm->add(group);
  mSettings.push_back(multSetting);
  setTitle("MULTIPLY");
}

FieldMultNode::~FieldMultNode()
{
  if(mResultField)
    {
      mResultField->destroy();
      delete mResultField;
      mResultField = nullptr;
    }
  if(mSettingForm) { delete mSettingForm; }
}

void FieldMultNode::resizeField(const Vec2f &fSize)
{
  if(fSize.x > 0 && fSize.y > 0 && mResultField && fSize != mResultField->size)
    { mResultField->create(fSize); }
}

void FieldMultNode::onUpdate()
{
  CudaFieldBase *field1 = inputs()[FIELDMULTNODE_INPUT_FIELD1]->get<CudaFieldBase>();
  CudaFieldBase *field2 = inputs()[FIELDMULTNODE_INPUT_FIELD2]->get<CudaFieldBase>();

  bool success = true;
  if(field1 && field2)
    {
      if(field1 && field2)
        {
          if(field1 && field2 && field1->size != field2->size)
            { std::cout << "====> WARNING: attempting to multiply different sized fields!\n";    success = false; }
          else if(field1 && field2 && field1->type() != field2->type())
            { std::cout << "====> WARNING: attempting to multiply fields of different types!\n"; success = false; }
        }

      if(success)
        {
          FieldType lastType1 = mF1Type; FieldType lastType2 = mF2Type;
          Vec2i     lastSize1 = mF1Size; Vec2i     lastSize2 = mF1Size;
          
          if(field1) { mF1Type = field1->type();    mF1Size = field1->size; }
          else       { mF1Type = FIELDTYPE_INVALID; mF1Size = Vec2i(); }
          if(field2) { mF2Type = field2->type();    mF2Size = field2->size; }
          else       { mF2Type = FIELDTYPE_INVALID; mF2Size = Vec2i(); }

          bool f1Tex = (field1 && field1->isTexture());
          bool f2Tex = (field2 && field2->isTexture());
          
          if(!mResultField ||
             (field1 && (mResultField->type() != mF1Type)) || // || (mResultField->isTexture() != f1Tex))) ||
             (field2 && (mResultField->type() != mF2Type)))   // || (mResultField->isTexture() != f2Tex))))
            {
              std::cout << "FieldMult reallocating mResultField...\n";
              if(mResultField) { mResultField->destroy(); delete mResultField; }

              std::cout << "allocating...\n";
              FieldType ft = (field1 ? mF1Type : mF2Type);
              if(field1 ? f1Tex : f2Tex) { mResultField = new CudaFieldTex(); }
              else                       { mResultField = makeCudaField(ft); }
              resizeField(field1 ? mF1Size : mF2Size);
              
              std::cout << "done allocating --> " << ((long long)mResultField) << "  |  " << mResultField->size << "  |  " << mResultField->type() << ".\n";
            }
          else if(mResultField)
            {
              Vec2i sz = (field1 ? mF1Size : mF2Size);
              int   ds = (field1 ? field1->dataSize : field2->dataSize);
              if((sz.x > 0 && sz.y > 0) && (mResultField->dataSize != ds))
                {
                  resizeField(field1 ? mF1Size : mF2Size);
                } //else { std::cout << "====> skipping resize...\n"; }
            }
          if(!mResultField) { std::cout << "Result field NULL!"; }
          
          if(mResultField && field1 && field2 &&
             (mResultField->size.x > 0 && mResultField->size.y > 0 && mResultField->size == mF1Size && mResultField->size == mF2Size))
            {
              switch(mF1Type)
                {
                case FIELDTYPE_INT:     fieldMult<int>    (field1, field2, mResultField); break;
                case FIELDTYPE_INT2:    fieldMult<int2>   (field1, field2, mResultField); break;
                case FIELDTYPE_INT3:    fieldMult<int3>   (field1, field2, mResultField); break;
                case FIELDTYPE_INT4:    fieldMult<int4>   (field1, field2, mResultField); break;
                case FIELDTYPE_FLOAT:   fieldMult<float>  (field1, field2, mResultField); break;
                case FIELDTYPE_FLOAT2:  fieldMult<float2> (field1, field2, mResultField); break;
                case FIELDTYPE_FLOAT3:  fieldMult<float3> (field1, field2, mResultField); break;
                case FIELDTYPE_FLOAT4:  fieldMult<float4> (field1, field2, mResultField); break;
                case FIELDTYPE_DOUBLE:  fieldMult<double> (field1, field2, mResultField); break;
                case FIELDTYPE_DOUBLE2: fieldMult<double2>(field1, field2, mResultField); break;
                case FIELDTYPE_DOUBLE3: fieldMult<double3>(field1, field2, mResultField); break;
                case FIELDTYPE_DOUBLE4: fieldMult<double4>(field1, field2, mResultField); break;
                }
              outputs()[FIELDMULTNODE_OUTPUT_FIELD]->set(mResultField);
            }
          else if(mResultField && (field1 || field2) &&
                  (mResultField->size.x > 0 && mResultField->size.y > 0 && mResultField->size == (field1 ? mF1Size : mF2Size)))
            {
              switch(field1 ? mF1Type : mF2Type)
                {
                case FIELDTYPE_INT:     fieldMultC<int>    (field1, mResultField, (int)mMult); break;
                case FIELDTYPE_INT2:    fieldMultC<int2>   (field1, mResultField, int2{(int)mMult, (int)mMult}); break;
                case FIELDTYPE_INT3:    fieldMultC<int3>   (field1, mResultField, int3{(int)mMult, (int)mMult, (int)mMult}); break;
                case FIELDTYPE_INT4:    fieldMultC<int4>   (field1, mResultField, int4{(int)mMult, (int)mMult, (int)mMult, (int)mMult}); break;
                case FIELDTYPE_FLOAT:   fieldMultC<float>  (field1, mResultField, (float)mMult); break;
                case FIELDTYPE_FLOAT2:  fieldMultC<float2> (field1, mResultField, float2{mMult, mMult}); break;
                case FIELDTYPE_FLOAT3:  fieldMultC<float3> (field1, mResultField, float3{mMult, mMult, mMult}); break;
                case FIELDTYPE_FLOAT4:  fieldMultC<float4> (field1, mResultField, float4{mMult, mMult, mMult, mMult}); break;
                case FIELDTYPE_DOUBLE:  fieldMultC<double> (field1, mResultField, (double)mMult); break;
                case FIELDTYPE_DOUBLE2: fieldMultC<double2>(field1, mResultField, double2{mMult, mMult}); break;
                case FIELDTYPE_DOUBLE3: fieldMultC<double3>(field1, mResultField, double3{mMult, mMult, mMult}); break;
                case FIELDTYPE_DOUBLE4: fieldMultC<double4>(field1, mResultField, double4{mMult, mMult, mMult, mMult}); break;
                }
              outputs()[FIELDMULTNODE_OUTPUT_FIELD]->set(mResultField);
            }
          else
            {
              std::cout << "====> WARNING(FieldMultNode): "; //Bad field size(s)!";
              if(mResultField && mResultField->size != Vec2i(0,0))
                {
                  std::cout << " (CField: " << (mResultField ? mResultField->size : Vec2i(-1,-1))
                            << ", input1: " << (field1 ? field1->size : Vec2i(-1,-1)) << ", input2: " << (field2 ? field2->size : Vec2i(-1,-1)) << ")\n";
                }
              else
                { std::cout<< " (mResultField --> " << (mResultField ? mResultField->size.toString() : "NULL") << ") \n"; }
              
              //if(mResultField) { mResultField->destroy(); delete mResultField; mResultField = nullptr; }
              outputs()[FIELDMULTNODE_OUTPUT_FIELD]->set((CudaFieldBase*)nullptr);
            }
        }
    }
}

void FieldMultNode::onDraw()
{
  float scale = getScale();
  ImGui::Text("Field 1 Size: (%d x %d) --> %d", mF1Size.x, mF1Size.y, mF1Type);
  ImGui::Text("Field 2 Size: (%d x %d) --> %d", mF2Size.x, mF2Size.y, mF2Type);
  mSettingForm->draw(scale, false, isBodyVisible());
}












FieldAddNode::FieldAddNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Field Add Node", false)
{
  // settings
  SettingBase *addSetting    = new Setting<float> ("Offset", "add", &mAdd, 1.0f);
  ((Setting<float>*)(addSetting))->setFormat(0.001, 0.1, "%.12f");
  ((Setting<float>*)(addSetting))->setMin(0.0);
  ((Setting<float>*)(addSetting))->setMax(1000000.0);
  SettingGroup *group = new SettingGroup("View Settings", "viewSettings", { addSetting }, true, false);
  mSettingForm = new SettingForm(150.0f, 150.0f);
  mSettingForm->add(group);
  mSettings.push_back(addSetting);
  setTitle("ADD");
}

FieldAddNode::~FieldAddNode()
{
  if(mResultField)
    {
      mResultField->destroy();
      delete mResultField;
      mResultField = nullptr;
    }
  if(mSettingForm) { delete mSettingForm; }
}

void FieldAddNode::resizeField(const Vec2f &fSize)
{
  if(fSize.x > 0 && fSize.y > 0 && mResultField && fSize != mResultField->size)
    { mResultField->create(fSize); }
}

void FieldAddNode::onUpdate()
{
  // cudaDeviceSynchronize();
  CudaFieldBase *field1 = inputs()[FIELDADDNODE_INPUT_FIELD1]->get<CudaFieldBase>();
  CudaFieldBase *field2 = inputs()[FIELDADDNODE_INPUT_FIELD2]->get<CudaFieldBase>();

  bool success = true;
  if(field1 && field2)
    {
      if(field1 && field2)
        {
          if(field1 && field2 && field1->size != field2->size)
            { std::cout << "====> WARNING: attempting to add different sized fields!\n";    success = false; }
          else if(field1 && field2 && field1->type() != field2->type())
            { std::cout << "====> WARNING: attempting to add fields of different types!\n"; success = false; }
        }

      if(success)
        {
          FieldType lastType1 = mF1Type; FieldType lastType2 = mF2Type;
          Vec2i     lastSize1 = mF1Size; Vec2i     lastSize2 = mF1Size;
          
          if(field1) { mF1Type = field1->type();    mF1Size = field1->size; }
          else       { mF1Type = FIELDTYPE_INVALID; mF1Size = Vec2i(); }
          if(field2) { mF2Type = field2->type();    mF2Size = field2->size; }
          else       { mF2Type = FIELDTYPE_INVALID; mF2Size = Vec2i(); }

          bool f1Tex = (field1 && field1->isTexture());
          bool f2Tex = (field2 && field2->isTexture());
          
          if(!mResultField ||
             (field1 && (mResultField->type() != mF1Type || (mResultField->isTexture() != f1Tex))) ||
             (field2 && (mResultField->type() != mF2Type || (mResultField->isTexture() != f2Tex))))
            {
              std::cout << "FieldAdd reallocating mResultField...\n";
              if(mResultField) { mResultField->destroy(); delete mResultField; }

              std::cout << "allocating...\n";
              FieldType ft = (field1 ? mF1Type : mF2Type);
              if(field1 ? f1Tex : f2Tex) { mResultField = new CudaFieldTex(); }
              else                                       { mResultField = makeCudaField(ft); }
              resizeField(field1 ? mF1Size : mF2Size);
              
              std::cout << "done allocating --> " << ((long long)mResultField) << "  |  " << mResultField->size << "  |  " << mResultField->type() << ".\n";
            }
          else if(mResultField)// && ((mResultField->size != (field1 ? mF1Size : (field2 ? mF2Size : Vec2i(1,1)))) ||
            // (mResultField->type() != (field1 ? mF1Type : (field2 ? mF2Type : FIELDTYPE_INVALID)))))
            {
              Vec2i sz = (field1 ? mF1Size : mF2Size);
              int   ds = (field1 ? field1->dataSize : field2->dataSize);
              if((sz.x > 0 && sz.y > 0) && (mResultField->dataSize != ds))
                {
                  resizeField(field1 ? mF1Size : mF2Size);
                } //else { std::cout << "====> skipping resize...\n"; }
            }
          if(!mResultField) { std::cout << "Result field NULL!"; }
          
          if(mResultField && field1 && field2 &&
             (mResultField->size.x > 0 && mResultField->size.y > 0 && mResultField->size == mF1Size && mResultField->size == mF2Size))
            {
              // cudaDeviceSynchronize();
              switch(mF1Type)
                {
                case FIELDTYPE_INT:     fieldAdd<int>    (field1, field2, mResultField); break;
                case FIELDTYPE_INT2:    fieldAdd<int2>   (field1, field2, mResultField); break;
                case FIELDTYPE_INT3:    fieldAdd<int3>   (field1, field2, mResultField); break;
                case FIELDTYPE_INT4:    fieldAdd<int4>   (field1, field2, mResultField); break;
                case FIELDTYPE_FLOAT:   fieldAdd<float>  (field1, field2, mResultField); break;
                case FIELDTYPE_FLOAT2:  fieldAdd<float2> (field1, field2, mResultField); break;
                case FIELDTYPE_FLOAT3:  fieldAdd<float3> (field1, field2, mResultField); break;
                case FIELDTYPE_FLOAT4:  fieldAdd<float4> (field1, field2, mResultField); break;
                case FIELDTYPE_DOUBLE:  fieldAdd<double> (field1, field2, mResultField); break;
                case FIELDTYPE_DOUBLE2: fieldAdd<double2>(field1, field2, mResultField); break;
                case FIELDTYPE_DOUBLE3: fieldAdd<double3>(field1, field2, mResultField); break;
                case FIELDTYPE_DOUBLE4: fieldAdd<double4>(field1, field2, mResultField); break;
                }
              // cudaDeviceSynchronize();
              outputs()[FIELDADDNODE_OUTPUT_FIELD]->set(mResultField);
            }
          else if(mResultField && (field1 || field2) &&
                  (mResultField->size.x > 0 && mResultField->size.y > 0 && mResultField->size == (field1 ? mF1Size : mF2Size)))
            {
              switch(field1 ? mF1Type : mF2Type)
                {
                case FIELDTYPE_INT:     fieldAddC<int>    (field1, mResultField, (int)mAdd); break;
                case FIELDTYPE_INT2:    fieldAddC<int2>   (field1, mResultField, int2{(int)mAdd, (int)mAdd}); break;
                case FIELDTYPE_INT3:    fieldAddC<int3>   (field1, mResultField, int3{(int)mAdd, (int)mAdd, (int)mAdd}); break;
                case FIELDTYPE_INT4:    fieldAddC<int4>   (field1, mResultField, int4{(int)mAdd, (int)mAdd, (int)mAdd, (int)mAdd}); break;
                case FIELDTYPE_FLOAT:   fieldAddC<float>  (field1, mResultField, (float)mAdd); break;
                case FIELDTYPE_FLOAT2:  fieldAddC<float2> (field1, mResultField, float2{mAdd, mAdd}); break;
                case FIELDTYPE_FLOAT3:  fieldAddC<float3> (field1, mResultField, float3{mAdd, mAdd, mAdd}); break;
                case FIELDTYPE_FLOAT4:  fieldAddC<float4> (field1, mResultField, float4{mAdd, mAdd, mAdd, mAdd}); break;
                case FIELDTYPE_DOUBLE:  fieldAddC<double> (field1, mResultField, (double)mAdd); break;
                case FIELDTYPE_DOUBLE2: fieldAddC<double2>(field1, mResultField, double2{mAdd, mAdd}); break;
                case FIELDTYPE_DOUBLE3: fieldAddC<double3>(field1, mResultField, double3{mAdd, mAdd, mAdd}); break;
                case FIELDTYPE_DOUBLE4: fieldAddC<double4>(field1, mResultField, double4{mAdd, mAdd, mAdd, mAdd}); break;
                }
              // cudaDeviceSynchronize();
              outputs()[FIELDADDNODE_OUTPUT_FIELD]->set(mResultField);
            }
          else
            {
              std::cout << "====> WARNING(FieldAddNode): "; //Bad field size(s)!";
              if(mResultField && mResultField->size != Vec2i(0,0))
                {
                  std::cout << " (CField: " << (mResultField ? mResultField->size : Vec2i(-1,-1))
                            << ", input1: " << (field1 ? field1->size : Vec2i(-1,-1)) << ", input2: " << (field2 ? field2->size : Vec2i(-1,-1)) << ")\n";
                }
              else
                { std::cout<< " (mResultField --> " << (mResultField ? mResultField->size.toString() : "NULL") << ") \n"; }
              
              //if(mResultField) { mResultField->destroy(); delete mResultField; mResultField = nullptr; }
              outputs()[FIELDADDNODE_OUTPUT_FIELD]->set((CudaFieldBase*)nullptr);
            }
        }
    }
}

void FieldAddNode::onDraw()
{
  float scale = getScale();
  std::stringstream ss; ss << mF1Type;
  ImGui::Text("Field 1 Size: (%d x %d) --> %s", mF1Size.x, mF1Size.y, ss.str().c_str());
  ss << mF2Type;
  ImGui::Text("Field 2 Size: (%d x %d) --> %s", mF2Size.x, mF2Size.y, ss.str().c_str());
  mSettingForm->draw(scale, false, isBodyVisible());
}




FieldNegNode::FieldNegNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Field Neg Node", false)
{
  
  setTitle("-");
}

FieldNegNode::~FieldNegNode()
{
  if(mResultField)
    {
      mResultField->destroy();
      delete mResultField;
      mResultField = nullptr;
    }
}

void FieldNegNode::resizeField(const Vec2f &fSize)
{
  std::cout << "resizing...\n";
  if(fSize.x > 0 && fSize.y > 0 && (fSize != mResultField->size))
    {
      std::cout << "creating --> " << fSize << "\n";
      mResultField->create(fSize);
    }
}

void FieldNegNode::onUpdate()
{
  CudaFieldBase *field = inputs()[FIELDNEGNODE_INPUT_FIELD]->get<CudaFieldBase>();

  if(field)
    {
      FieldType lastType = mFType;
      
      if(field) { mFType = field->type();     mFSize = field->size; }
      else      { mFType = FIELDTYPE_INVALID; mFSize = Vec2i(); }
          
      if(!mResultField || (field && (mResultField->type() != mFType || (mResultField->isTexture() != field->isTexture()))))
        {
          std::cout << "FieldNeg reallocating mResultField...\n";
          if(mResultField) { mResultField->destroy(); delete mResultField; }

          std::cout << "allocating...\n";
          if(field->isTexture()) { mResultField = new CudaFieldTex(); }
          else                   { mResultField = makeCudaField(mFType); }
          std::cout << "done allocating --> (*" << ((long long)mResultField) << ")  |  " << mResultField->size << "  |  " << mResultField->type()
                    << " | " << mResultField->dataSize << " | " << mResultField->typeSize << "\n";
          resizeField(mFSize);
          std::cout << "done resizing --> (*" << ((long long)mResultField) << ")  |  " << mResultField->size << "  |  " << mResultField->type()
                    << " | " << mResultField->dataSize << " | " << mResultField->typeSize << "\n";
        }
      else if(mResultField)// && ((mResultField->size != (field1 ? mF1Size : (field2 ? mF2Size : Vec2i(1,1)))) ||
        // (mResultField->type() != (field1 ? mF1Type : (field2 ? mF2Type : FIELDTYPE_INVALID)))))
        {
          if((field->size.x > 0 && field->size.y > 0) && (mResultField->dataSize != field->dataSize))
            { resizeField(field->size); }
        }
      if(!mResultField) { std::cout << "Result field NULL!"; return; }
      
      // if((field->size.x > 0 && field->size.y > 0) && ((mResultField->dataSize != field->dataSize)))
      //   {
      //     resizeField(field->size);
      //     std::cout << "done resizing --> (*" << ((long long)mResultField) << ")  |  " << mResultField->size << "  |  " << mResultField->type()
      //               << " | " << mResultField->dataSize << " | " << mResultField->typeSize << "\n";
      //   }
          
      if(field && mResultField && mResultField->allocated() > 0 && mResultField->size == field->size)
        {
          switch(field->type())
            {
            case FIELDTYPE_INT:     fieldNegate<int>    (field, mResultField); break;
            case FIELDTYPE_INT2:    fieldNegate<int2>   (field, mResultField); break;
            case FIELDTYPE_INT3:    fieldNegate<int3>   (field, mResultField); break;
            case FIELDTYPE_INT4:    fieldNegate<int4>   (field, mResultField); break;
            case FIELDTYPE_FLOAT:   fieldNegate<float>  (field, mResultField); break;
            case FIELDTYPE_FLOAT2:  fieldNegate<float2> (field, mResultField); break;
            case FIELDTYPE_FLOAT3:  fieldNegate<float3> (field, mResultField); break;
            case FIELDTYPE_FLOAT4:  fieldNegate<float4> (field, mResultField); break;
            case FIELDTYPE_DOUBLE:  fieldNegate<double> (field, mResultField); break;
            case FIELDTYPE_DOUBLE2: fieldNegate<double2>(field, mResultField); break;
            case FIELDTYPE_DOUBLE3: fieldNegate<double3>(field, mResultField); break;
            case FIELDTYPE_DOUBLE4: fieldNegate<double4>(field, mResultField); break;
            }
          outputs()[FIELDNEGNODE_OUTPUT_FIELD]->set(mResultField);
        }
      else
        {
          std::cout << "====> WARNING(FieldNegNode): ";
          if(mResultField->size != Vec2i(0,0))
            {
              std::cout << " (output: " << mResultField->size << " | " << mResultField->dataSize << " | " << mResultField->typeSize << " | " << mResultField->type()
                        << ", input: " << field->size << " | " << field->dataSize << " | " << field->typeSize << " | " << field->type() << ")\n";
            }
          else
            { std::cout<< " (mResultField --> " << mResultField->size.toString() << ") \n"; }
          
          //if(mResultField) { mResultField->destroy(); delete mResultField; mResultField = nullptr; }
          outputs()[FIELDNEGNODE_OUTPUT_FIELD]->set((CudaFieldBase*)nullptr);
        }
    }
}

void FieldNegNode::onDraw()
{
  float scale = getScale();

  std::stringstream ss; ss << mFType;
  ImGui::Text("Field Size: (%d x %d) --> %s", mFSize.x, mFSize.y, ss.str().c_str());
}










FieldAbsNode::FieldAbsNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Field Abs Node", false)
{
  setTitle("ABS");
}

FieldAbsNode::~FieldAbsNode()
{
  if(mResultField)
    {
      mResultField->destroy();
      delete mResultField;
      mResultField = nullptr;
    }
}

void FieldAbsNode::resizeField(const Vec2f &fSize)
{
  std::cout << "resizing...\n";
  if(fSize.x > 0 && fSize.y > 0 && mResultField && fSize != mResultField->size)
    {
      std::cout << "creating --> " << fSize << "\n";
      mResultField->create(fSize);
    }
}

void FieldAbsNode::onUpdate()
{
  CudaFieldBase *field = inputs()[FIELDABSNODE_INPUT_FIELD]->get<CudaFieldBase>();

  if(field)
    {
      FieldType lastType = mFType;
          
      if(field) { mFType = field->type();     mFSize = field->size; }
      else      { mFType = FIELDTYPE_INVALID; mFSize = Vec2i(); }
          
      if(!mResultField || (field && (mResultField->type() != mFType || (mResultField->isTexture() != field->isTexture()))))
        {
          std::cout << "FieldAbs reallocating mResultField...\n";
          if(mResultField) { mResultField->destroy(); delete mResultField; }

          std::cout << "allocating...\n";
          if(field->isTexture()) { mResultField = new CudaFieldTex(); }
          else                   { mResultField = makeCudaField(mFType); }
          std::cout << "done allocating --> (*" << ((long long)mResultField) << ")  |  " << mResultField->size << "  |  " << mResultField->type()
                    << " | " << mResultField->dataSize << " | " << mResultField->typeSize << "\n";
          resizeField(mFSize);
          std::cout << "done resizing --> (*" << ((long long)mResultField) << ")  |  " << mResultField->size << "  |  " << mResultField->type()
                    << " | " << mResultField->dataSize << " | " << mResultField->typeSize << "\n";
        }
      else if(mResultField)// && ((mResultField->size != (field1 ? mF1Size : (field2 ? mF2Size : Vec2i(1,1)))) ||
        // (mResultField->type() != (field1 ? mF1Type : (field2 ? mF2Type : FIELDTYPE_INVALID)))))
        {
          if((field->size.x > 0 && field->size.y > 0) && (mResultField->dataSize != field->dataSize)) { resizeField(field->size); }
        }
      if(!mResultField) { std::cout << "Result field NULL!"; }
          
      if(mResultField && field && mResultField->allocated() > 0 && mResultField->size == field->size)
        {
          switch(field->type())
            {
            case FIELDTYPE_INT:     fieldAbs<int>    (field, mResultField); break;
            case FIELDTYPE_INT2:    fieldAbs<int2>   (field, mResultField); break;
            case FIELDTYPE_INT3:    fieldAbs<int3>   (field, mResultField); break;
            case FIELDTYPE_INT4:    fieldAbs<int4>   (field, mResultField); break;
            case FIELDTYPE_FLOAT:   fieldAbs<float>  (field, mResultField); break;
            case FIELDTYPE_FLOAT2:  fieldAbs<float2> (field, mResultField); break;
            case FIELDTYPE_FLOAT3:  fieldAbs<float3> (field, mResultField); break;
            case FIELDTYPE_FLOAT4:  fieldAbs<float4> (field, mResultField); break;
            case FIELDTYPE_DOUBLE:  fieldAbs<double> (field, mResultField); break;
            case FIELDTYPE_DOUBLE2: fieldAbs<double2>(field, mResultField); break;
            case FIELDTYPE_DOUBLE3: fieldAbs<double3>(field, mResultField); break;
            case FIELDTYPE_DOUBLE4: fieldAbs<double4>(field, mResultField); break;
            }
          outputs()[FIELDABSNODE_OUTPUT_FIELD]->set(mResultField);
        }
      else
        {
          std::cout << "====> WARNING(FieldAbsNode): ";
          if(mResultField && mResultField->size != Vec2i(0,0))
            {
              std::cout << " (output: " << mResultField->size << " | " << mResultField->dataSize << " | " << mResultField->typeSize << " | " << mResultField->type()
                        << ", input: " << field->size << " | " << field->dataSize << " | " << field->typeSize << " | " << field->type() << ")\n";
            }
          else
            { std::cout<< " (mResultField --> " << (mResultField ? mResultField->size.toString() : "NULL") << ") \n"; }
          
          //if(mResultField) { mResultField->destroy(); delete mResultField; mResultField = nullptr; }
          outputs()[FIELDABSNODE_OUTPUT_FIELD]->set((CudaFieldBase*)nullptr);
        }
    }
}

void FieldAbsNode::onDraw()
{
  float scale = getScale();

  std::stringstream ss; ss << mFType;
  ImGui::Text("Field Size: (%d x %d) --> %s", mFSize.x, mFSize.y, ss.str().c_str());
}




FieldLogNode::FieldLogNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Field Log Node", false)
{
  setTitle("LOG(F)");
}

FieldLogNode::~FieldLogNode()
{
  if(mResultField)
    {
      mResultField->destroy();
      delete mResultField;
      mResultField = nullptr;
    }
}

void FieldLogNode::resizeField(const Vec2f &fSize)
{
  std::cout << "resizing...\n";
  if(fSize.x > 0 && fSize.y > 0 && mResultField && fSize != mResultField->size)
    {
      std::cout << "creating --> " << fSize << "\n";
      mResultField->create(fSize);
    }
}

void FieldLogNode::onUpdate()
{
  CudaFieldBase *field = inputs()[FIELDLOGNODE_INPUT_FIELD]->get<CudaFieldBase>();

  if(field)
    {
      FieldType lastType = mFType;
          
      if(field) { mFType = field->type();     mFSize = field->size; }
      else      { mFType = FIELDTYPE_INVALID; mFSize = Vec2i(); }
          
      if(!mResultField || (field && (mResultField->type() != mFType || (mResultField->isTexture() != field->isTexture()))))
        {
          std::cout << "FieldLog reallocating mResultField...\n";
          if(mResultField) { mResultField->destroy(); delete mResultField; }

          std::cout << "allocating...\n";
          if(field->isTexture()) { mResultField = new CudaFieldTex(); }
          else                   { mResultField = makeCudaField(mFType); }
          std::cout << "done allocating --> (*" << ((long long)mResultField) << ")  |  " << mResultField->size << "  |  " << mResultField->type()
                    << " | " << mResultField->dataSize << " | " << mResultField->typeSize << "\n";
          resizeField(mFSize);
          std::cout << "done resizing --> (*" << ((long long)mResultField) << ")  |  " << mResultField->size << "  |  " << mResultField->type()
                    << " | " << mResultField->dataSize << " | " << mResultField->typeSize << "\n";
        }
      else if(mResultField)
        { if((field->size.x > 0 && field->size.y > 0) && (mResultField->dataSize != field->dataSize)) { resizeField(field->size); } }
      if(!mResultField) { std::cout << "Result field NULL!"; }
          
      if(mResultField && field && mResultField->allocated() > 0 && mResultField->size == field->size)
        {
          switch(field->type())
            {
            case FIELDTYPE_INT:     fieldLog<int>    (field, mResultField); break;
            case FIELDTYPE_INT2:    fieldLog<int2>   (field, mResultField); break;
            case FIELDTYPE_INT3:    fieldLog<int3>   (field, mResultField); break;
            case FIELDTYPE_INT4:    fieldLog<int4>   (field, mResultField); break;
            case FIELDTYPE_FLOAT:   fieldLog<float>  (field, mResultField); break;
            case FIELDTYPE_FLOAT2:  fieldLog<float2> (field, mResultField); break;
            case FIELDTYPE_FLOAT3:  fieldLog<float3> (field, mResultField); break;
            case FIELDTYPE_FLOAT4:  fieldLog<float4> (field, mResultField); break;
            case FIELDTYPE_DOUBLE:  fieldLog<double> (field, mResultField); break;
            case FIELDTYPE_DOUBLE2: fieldLog<double2>(field, mResultField); break;
            case FIELDTYPE_DOUBLE3: fieldLog<double3>(field, mResultField); break;
            case FIELDTYPE_DOUBLE4: fieldLog<double4>(field, mResultField); break;
            }
          outputs()[FIELDLOGNODE_OUTPUT_FIELD]->set(mResultField);
        }
      else
        {
          std::cout << "====> WARNING(FieldLogNode): ";
          if(mResultField && mResultField->size != Vec2i(0,0))
            {
              std::cout << " (output: " << mResultField->size << " | " << mResultField->dataSize << " | " << mResultField->typeSize << " | " << mResultField->type()
                        << ", input: " << field->size << " | " << field->dataSize << " | " << field->typeSize << " | " << field->type() << ")\n";
            }
          else
            { std::cout<< " (mResultField --> " << (mResultField ? mResultField->size.toString() : "NULL") << ") \n"; }
          
          //if(mResultField) { mResultField->destroy(); delete mResultField; mResultField = nullptr; }
          outputs()[FIELDLOGNODE_OUTPUT_FIELD]->set((CudaFieldBase*)nullptr);
        }
    }
}
void FieldLogNode::onDraw()
{
  float scale = getScale();
  std::stringstream ss; ss << mFType;
  ImGui::Text("Field Size: (%d x %d) --> %s", mFSize.x, mFSize.y, ss.str().c_str());
}





FieldExpNode::FieldExpNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Field Exp Node", false)
{
  setTitle("EXP(F)");
}

FieldExpNode::~FieldExpNode()
{
  if(mResultField)
    {
      mResultField->destroy();
      delete mResultField;
      mResultField = nullptr;
    }
}

void FieldExpNode::resizeField(const Vec2f &fSize)
{
  std::cout << "resizing...\n";
  if(fSize.x > 0 && fSize.y > 0 && mResultField && fSize != mResultField->size)
    {
      std::cout << "creating --> " << fSize << "\n";
      mResultField->create(fSize);
    }
}

void FieldExpNode::onUpdate()
{
  CudaFieldBase *field = inputs()[FIELDEXPNODE_INPUT_FIELD]->get<CudaFieldBase>();

  if(field)
    {
      FieldType lastType = mFType;
          
      if(field) { mFType = field->type();     mFSize = field->size; }
      else      { mFType = FIELDTYPE_INVALID; mFSize = Vec2i(); }
          
      if(!mResultField || (field && (mResultField->type() != mFType || (mResultField->isTexture() != field->isTexture()))))
        {
          std::cout << "FieldExp reallocating mResultField...\n";
          if(mResultField) { mResultField->destroy(); delete mResultField; }

          std::cout << "allocating...\n";
          if(field->isTexture()) { mResultField = new CudaFieldTex(); }
          else                   { mResultField = makeCudaField(mFType); }
          std::cout << "done allocating --> (*" << ((long long)mResultField) << ")  |  " << mResultField->size << "  |  " << mResultField->type()
                    << " | " << mResultField->dataSize << " | " << mResultField->typeSize << "\n";
          resizeField(mFSize);
          std::cout << "done resizing --> (*" << ((long long)mResultField) << ")  |  " << mResultField->size << "  |  " << mResultField->type()
                    << " | " << mResultField->dataSize << " | " << mResultField->typeSize << "\n";
        }
      else if(mResultField)
        { if((field->size.x > 0 && field->size.y > 0) && (mResultField->dataSize != field->dataSize)) { resizeField(field->size); } }
      if(!mResultField) { std::cout << "Result field NULL!"; }
          
      if(mResultField && field && mResultField->allocated() > 0 && mResultField->size == field->size)
        {
          switch(field->type())
            {
            case FIELDTYPE_INT:     fieldExp<int>    (field, mResultField); break;
            case FIELDTYPE_INT2:    fieldExp<int2>   (field, mResultField); break;
            case FIELDTYPE_INT3:    fieldExp<int3>   (field, mResultField); break;
            case FIELDTYPE_INT4:    fieldExp<int4>   (field, mResultField); break;
            case FIELDTYPE_FLOAT:   fieldExp<float>  (field, mResultField); break;
            case FIELDTYPE_FLOAT2:  fieldExp<float2> (field, mResultField); break;
            case FIELDTYPE_FLOAT3:  fieldExp<float3> (field, mResultField); break;
            case FIELDTYPE_FLOAT4:  fieldExp<float4> (field, mResultField); break;
            case FIELDTYPE_DOUBLE:  fieldExp<double> (field, mResultField); break;
            case FIELDTYPE_DOUBLE2: fieldExp<double2>(field, mResultField); break;
            case FIELDTYPE_DOUBLE3: fieldExp<double3>(field, mResultField); break;
            case FIELDTYPE_DOUBLE4: fieldExp<double4>(field, mResultField); break;
            }
          outputs()[FIELDEXPNODE_OUTPUT_FIELD]->set(mResultField);
        }
      else
        {
          std::cout << "====> WARNING(FieldExpNode): ";
          if(mResultField && mResultField->size != Vec2i(0,0))
            {
              std::cout << " (output: " << mResultField->size << " | " << mResultField->dataSize << " | " << mResultField->typeSize << " | " << mResultField->type()
                        << ", input: " << field->size << " | " << field->dataSize << " | " << field->typeSize << " | " << field->type() << ")\n";
            }
          else
            { std::cout<< " (mResultField --> " << (mResultField ? mResultField->size.toString() : "NULL") << ") \n"; }
          
          //if(mResultField) { mResultField->destroy(); delete mResultField; mResultField = nullptr; }
          outputs()[FIELDEXPNODE_OUTPUT_FIELD]->set((CudaFieldBase*)nullptr);
        }
    }
}
void FieldExpNode::onDraw()
{
  float scale = getScale();
  std::stringstream ss; ss << mFType;
  ImGui::Text("Field Size: (%d x %d) --> %s", mFSize.x, mFSize.y, ss.str().c_str());
}
















FieldMaxNode::FieldMaxNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Field Max Node", false)
{
  setTitle("F/max(F)");
}

FieldMaxNode::~FieldMaxNode()
{
  mMaxField.destroy();
  if(mResultField)
    {
      mResultField->destroy();
      delete mResultField;
      mResultField = nullptr;
    }
}

void FieldMaxNode::resizeField(const Vec2f &fSize)
{
  std::cout << "resizing...\n";
  if(fSize.x > 0 && fSize.y > 0 && (fSize != mResultField->size || fSize != mMaxField.size))
    {
      std::cout << "creating --> " << fSize << "\n";
      mResultField->create(fSize);
      mMaxField.create(fSize);
    }
}

void FieldMaxNode::onUpdate()
{
  CudaFieldBase *field = inputs()[FIELDMAXNODE_INPUT_FIELD]->get<CudaFieldBase>();

  if(field)
    {
      FieldType lastType = mFType;
      
      if(field) { mFType = field->type();     mFSize = field->size; }
      else      { mFType = FIELDTYPE_INVALID; mFSize = Vec2i(); }
          
      if(!mResultField || (field && (mResultField->type() != mFType || (mResultField->isTexture() != field->isTexture()))))
        {
          std::cout << "FieldMax reallocating mResultField...\n";
          if(mResultField) { mResultField->destroy(); delete mResultField; }

          std::cout << "allocating...\n";
          if(field->isTexture()) { mResultField = new CudaFieldTex(); }
          else                   { mResultField = makeCudaField(mFType); }
          std::cout << "done allocating --> (*" << ((long long)mResultField) << ")  |  " << mResultField->size << "  |  " << mResultField->type()
                    << " | " << mResultField->dataSize << " | " << mResultField->typeSize << "\n";
          resizeField(mFSize);
          std::cout << "done resizing --> (*" << ((long long)mResultField) << ")  |  " << mResultField->size << "  |  " << mResultField->type()
                    << " | " << mResultField->dataSize << " | " << mResultField->typeSize << "\n";
        }
      else if(mResultField)// && ((mResultField->size != (field1 ? mF1Size : (field2 ? mF2Size : Vec2i(1,1)))) ||
        // (mResultField->type() != (field1 ? mF1Type : (field2 ? mF2Type : FIELDTYPE_INVALID)))))
        {
          if((field->size.x > 0 && field->size.y > 0) && (mResultField->dataSize != field->dataSize))
            { resizeField(field->size); }
        }
      if(!mResultField) { std::cout << "Result field NULL!"; return; }
      
      // if((field->size.x > 0 && field->size.y > 0) && (mMaxField.dataSize != field->dataSize ||
      //                                                 (mResultField->dataSize != field->dataSize)))
      //   {
      //     resizeField(field->size);
      //     std::cout << "done resizing --> (*" << ((long long)mResultField) << ")  |  " << mResultField->size << "  |  " << mResultField->type()
      //               << " | " << mResultField->dataSize << " | " << mResultField->typeSize << "\n";
      //   }
          
      if(field && mMaxField.allocated() > 0 && mMaxField.size == field->size && mResultField && mResultField->allocated() > 0 && mResultField->size == field->size)
        {
          switch(field->type())
            {
            case FIELDTYPE_INT:     mMaxValue = fieldMax<int>    (field, mResultField, &mMaxField); break;
            case FIELDTYPE_INT2:    mMaxValue = fieldMax<int2>   (field, mResultField, &mMaxField); break;
            case FIELDTYPE_INT3:    mMaxValue = fieldMax<int3>   (field, mResultField, &mMaxField); break;
            case FIELDTYPE_INT4:    mMaxValue = fieldMax<int4>   (field, mResultField, &mMaxField); break;
            case FIELDTYPE_FLOAT:   mMaxValue = fieldMax<float>  (field, mResultField, &mMaxField); break;
            case FIELDTYPE_FLOAT2:  mMaxValue = fieldMax<float2> (field, mResultField, &mMaxField); break;
            case FIELDTYPE_FLOAT3:  mMaxValue = fieldMax<float3> (field, mResultField, &mMaxField); break;
            case FIELDTYPE_FLOAT4:  mMaxValue = fieldMax<float4> (field, mResultField, &mMaxField); break;
            case FIELDTYPE_DOUBLE:  mMaxValue = fieldMax<double> (field, mResultField, &mMaxField); break;
            case FIELDTYPE_DOUBLE2: mMaxValue = fieldMax<double2>(field, mResultField, &mMaxField); break;
            case FIELDTYPE_DOUBLE3: mMaxValue = fieldMax<double3>(field, mResultField, &mMaxField); break;
            case FIELDTYPE_DOUBLE4: mMaxValue = fieldMax<double4>(field, mResultField, &mMaxField); break;
            }
          outputs()[FIELDMAXNODE_OUTPUT_FIELD]->set(mResultField);
        }
      else
        {
          std::cout << "====> WARNING(FieldMaxNode): ";
          if(mResultField->size != Vec2i(0,0))
            {
              std::cout << " (output: " << mResultField->size << " | " << mResultField->dataSize << " | " << mResultField->typeSize << " | " << mResultField->type()
                        << ", input: " << field->size << " | " << field->dataSize << " | " << field->typeSize << " | " << field->type() << ")\n";
            }
          else
            { std::cout<< " (mResultField --> " << mResultField->size.toString() << ") \n"; }
          
          //if(mResultField) { mResultField->destroy(); delete mResultField; mResultField = nullptr; }
          outputs()[FIELDMAXNODE_OUTPUT_FIELD]->set((CudaFieldBase*)nullptr);
        }
    }
}

void FieldMaxNode::onDraw()
{
  float scale = getScale();

  std::stringstream ss; ss << mFType;
  ImGui::Text("Field Size: (%d x %d) --> %s", mFSize.x, mFSize.y, ss.str().c_str());

  // if(mMaxField.allocated())
  //   {
  //     mMaxField.pullData();
  //     if(mMaxField.hData)
  //       {
  //         int blockSz = mMaxField.size.x*mMaxField.size.y/(float)(16*16*2);
  //         float mult = 0.0f;
  //         for(int i = 0; i < blockSz; i++)
  //           { mult = std::max(mult, mMaxField.hData[i]); }
  //         ss << " --> " << mult;
  //       }
  ImGui::Text("%.6f", mMaxValue);
  // }
}









inline void drawHat(ImDrawList *drawList, Vec2f p, Vec2f s, const Vec4f &color, float w)
{
  if(!drawList) { return; }

  // s.x = 2.0f*s.y/tan(M_PI/3.0) * 0.9f; // scale width

  float dimW   = w/(2*sqrt(2.0));
  float tw     = w*(sqrt(2.0));
  float innerW = s.x - 2.0f*dimW;

  // left side of hat
  Vec2f l0 = p  + Vec2f(-innerW/2.0f, 0.0f);
  Vec2f l1 = l0 + Vec2f(-dimW, -dimW);
  // right side of hat
  Vec2f r0 = p  + Vec2f(innerW/2.0f, 0.0f);
  Vec2f r1 = r0 + Vec2f(dimW, -dimW);
  // top of hat
  Vec2f t0 = p + Vec2f(0.0f, -(s.y - tw));
  Vec2f t1 = t0 + Vec2f(0.0f, -tw);

  // drawList->AddTriangleFilled(l1 - Vec2f(-dimW, -dimW)/2.0f, l0, t0, ImColor(color));
  drawList->AddTriangleFilled(t0, t1, l1, ImColor(color));
  // drawList->AddTriangleFilled(r0, r1 - Vec2f(dimW, -dimW)/2.0f, t0, ImColor(color));
  drawList->AddTriangleFilled(t1, t0, r1, ImColor(color));
  
  
}









FieldNormNode::FieldNormNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Field Norm Node", false)
{
  setTitle("F/avg(F)"); // will be 'F-hat' (hat drawn in onDraw())
}

FieldNormNode::~FieldNormNode()
{
  mNormField.destroy();
  if(mResultField)
    {
      mResultField->destroy();
      delete mResultField;
      mResultField = nullptr;
    }
}

void FieldNormNode::resizeField(const Vec2f &fSize)
{
  std::cout << "resizing...\n";
  if(fSize.x > 0 && fSize.y > 0 && (fSize != mResultField->size || fSize != mNormField.size))
    {
      std::cout << "creating --> " << fSize << "\n";
      mResultField->create(fSize);
      mNormField.create(fSize);
    }
}

void FieldNormNode::onUpdate()
{
  CudaFieldBase *field = inputs()[FIELDNORMNODE_INPUT_FIELD]->get<CudaFieldBase>();

  if(field)
    {
      FieldType lastType = mFType;
      
      if(field) { mFType = field->type();     mFSize = field->size; }
      else      { mFType = FIELDTYPE_INVALID; mFSize = Vec2i(); }
          
      if(!mResultField || (field && (mResultField->type() != mFType || (mResultField->isTexture() != field->isTexture()))))
        {
          std::cout << "FieldNorm reallocating mResultField...\n";
          if(mResultField) { mResultField->destroy(); delete mResultField; }

          std::cout << "allocating...\n";
          if(field->isTexture()) { mResultField = new CudaFieldTex(); }
          else                   { mResultField = makeCudaField(mFType); }
          std::cout << "done allocating --> (*" << ((long long)mResultField) << ")  |  " << mResultField->size << "  |  " << mResultField->type()
                    << " | " << mResultField->dataSize << " | " << mResultField->typeSize << "\n";
          resizeField(mFSize);
          std::cout << "done resizing --> (*" << ((long long)mResultField) << ")  |  " << mResultField->size << "  |  " << mResultField->type()
                    << " | " << mResultField->dataSize << " | " << mResultField->typeSize << "\n";
        }
      else if(mResultField)// && ((mResultField->size != (field1 ? mF1Size : (field2 ? mF2Size : Vec2i(1,1)))) ||
        // (mResultField->type() != (field1 ? mF1Type : (field2 ? mF2Type : FIELDTYPE_INVALID)))))
        {
          if((field->size.x > 0 && field->size.y > 0) && (mResultField->dataSize != field->dataSize))
            { resizeField(field->size); }
        }
      if(!mResultField) { std::cout << "Result field NULL!"; return; }
      
      // if((field->size.x > 0 && field->size.y > 0) && (mNormField.dataSize != field->dataSize ||
      //                                                 (mResultField->dataSize != field->dataSize)))
      //   {
      //     resizeField(field->size);
      //     std::cout << "done resizing --> (*" << ((long long)mResultField) << ")  |  " << mResultField->size << "  |  " << mResultField->type()
      //               << " | " << mResultField->dataSize << " | " << mResultField->typeSize << "\n";
      //   }
          
      if(field && mNormField.allocated() > 0 && mNormField.size == field->size && mResultField && mResultField->allocated() > 0 && mResultField->size == field->size)
        {
          switch(field->type())
            {
            case FIELDTYPE_INT:     mAvgLength = fieldNorm<int>    (field, mResultField, &mNormField); break;
            case FIELDTYPE_INT2:    mAvgLength = fieldNorm<int2>   (field, mResultField, &mNormField); break;
            case FIELDTYPE_INT3:    mAvgLength = fieldNorm<int3>   (field, mResultField, &mNormField); break;
            case FIELDTYPE_INT4:    mAvgLength = fieldNorm<int4>   (field, mResultField, &mNormField); break;
            case FIELDTYPE_FLOAT:   mAvgLength = fieldNorm<float>  (field, mResultField, &mNormField); break;
            case FIELDTYPE_FLOAT2:  mAvgLength = fieldNorm<float2> (field, mResultField, &mNormField); break;
            case FIELDTYPE_FLOAT3:  mAvgLength = fieldNorm<float3> (field, mResultField, &mNormField); break;
            case FIELDTYPE_FLOAT4:  mAvgLength = fieldNorm<float4> (field, mResultField, &mNormField); break;
            case FIELDTYPE_DOUBLE:  mAvgLength = fieldNorm<double> (field, mResultField, &mNormField); break;
            case FIELDTYPE_DOUBLE2: mAvgLength = fieldNorm<double2>(field, mResultField, &mNormField); break;
            case FIELDTYPE_DOUBLE3: mAvgLength = fieldNorm<double3>(field, mResultField, &mNormField); break;
            case FIELDTYPE_DOUBLE4: mAvgLength = fieldNorm<double4>(field, mResultField, &mNormField); break;
            }
          outputs()[FIELDNORMNODE_OUTPUT_FIELD]->set(mResultField);
        }
      else
        {
          std::cout << "====> WARNING(FieldNormNode): ";
          if(mResultField->size != Vec2i(0,0))
            {
              std::cout << " (output: " << mResultField->size << " | " << mResultField->dataSize << " | " << mResultField->typeSize << " | " << mResultField->type()
                        << ", input: " << field->size << " | " << field->dataSize << " | " << field->typeSize << " | " << field->type() << ")\n";
            }
          else
            { std::cout<< " (mResultField --> " << mResultField->size.toString() << ") \n"; }
          
          //if(mResultField) { mResultField->destroy(); delete mResultField; mResultField = nullptr; }
          outputs()[FIELDNORMNODE_OUTPUT_FIELD]->set((CudaFieldBase*)nullptr);
        }
    }
}

void FieldNormNode::onDraw()
{
  float scale = getScale();
  Vec2f p0 = ImGui::GetCursorScreenPos();
  
  std::stringstream ss; ss << mFType;
  ImGui::Text("Field Size: (%d x %d) <%s>", mFSize.x, mFSize.y, ss.str().c_str());
  ImGui::Text("Avg Length: %.6f", mAvgLength);

  // drawHat(drawList(), // ???
  //         titlePos() + Vec2f(titleSize().x/2.0f, 1.0f*scale), Vec2f(titleSize().y*0.5f, titleSize().y*0.3),
  //         Vec4f(1.0f, 1.0f, 1.0f, 1.0f), 2.0f*scale);
}
