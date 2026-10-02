#include "fieldDelOperators.hpp"

#include <imgui.h>

#include "setting.hpp"
#include "settingForm.hpp"
#include "cudaField.hpp"
#include "glfwKeys.hpp"
#include "field-operators.h"



inline void drawNabla(ImDrawList *drawList, Vec2f p, Vec2f s, const Vec4f &color, float w)
{
  if(!drawList) { return; }

  s.x = 2.0f*s.y/tan(M_PI/3.0) * 0.9f; // scale width
  
  // outer points
  Vec2f o0 = p;
  Vec2f o1 = p + Vec2f(s.x, 0.0f);
  Vec2f o2 = p + Vec2f(s.x/2.0f, s.y);

  // diagonal offset
  float xOffset = (float)w / tan(M_PI/6.0);
  float yOffset = (float)w / sin(M_PI/6.0);
  
  // inner points
  Vec2f i0 = o0 + Vec2f( xOffset, w);
  Vec2f i1 = o1 + Vec2f(-xOffset, w);
  Vec2f i2 = o2 + Vec2f(0.0f, -yOffset);

  // slightly offset inner points
  Vec2f shift = Vec2f(xOffset, 0.0f)*0.2f;
  i0 += shift; i1 += shift; i2 += shift;
  
  drawList->AddTriangleFilled(o0, o1, i1, ImColor(color));
  drawList->AddTriangleFilled(i1, o0, i0, ImColor(color));
  
  drawList->AddTriangleFilled(o1, o2, i2, ImColor(color));
  drawList->AddTriangleFilled(i2, o1, i1, ImColor(color));
  
  drawList->AddTriangleFilled(o2, o0, i0, ImColor(color));
  drawList->AddTriangleFilled(i0, o2, i2, ImColor(color));
}




//////////////////////////////////////
// ∇ OPERATORS
//////////////////////////////////////

FieldGradNode::FieldGradNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Field Gradient Node", false)
{
  setTitle("  F");
}

FieldGradNode::~FieldGradNode()
{
  if(mResultX) { mResultX->destroy(); delete mResultX; mResultX = nullptr; }
  if(mResultY) { mResultY->destroy(); delete mResultY; mResultY = nullptr; }
}

void FieldGradNode::resizeField(const Vec2f &fSize)
{
  if(fSize.x > 0 && fSize.y > 0 && mResultX && mResultY && (fSize != mResultX->size || fSize != mResultY->size))
    {
      mResultX->create(fSize);
      mResultY->create(fSize);
    }
}

void FieldGradNode::onUpdate()
{
  CudaFieldBase *field = inputs()[FIELDGRADIENTNODE_INPUT_FIELD]->get<CudaFieldBase>();
  mInputField = field;
  
  if(field)
    {
      if(!mResultX || !mResultY || (field && (mResultX->type() != field->type() || (mResultX->isTexture() != field->isTexture()) ||
                                              (mResultY->type() != field->type() || (mResultY->isTexture() != field->isTexture())))))
        {
          if(mResultX) { mResultX->destroy(); delete mResultX; mResultX = nullptr; }
          if(mResultY) { mResultY->destroy(); delete mResultY; mResultY = nullptr; }
          if(field->isTexture()) { mResultX = new CudaFieldTex(); mResultY = new CudaFieldTex(); }
          else { mResultX = makeCudaField(field->type()); mResultY = makeCudaField(field->type()); }
          resizeField(field->size);
        }
      else if(mResultX && mResultY)
        { resizeField(field->size); }
      
      if(mResultX && mResultY && field && mResultX->allocated() && mResultX->size == field->size && mResultY->allocated() && mResultY->size == field->size)
        {
          switch(field->type())
            {
            case FIELDTYPE_INT:     fieldGradient<int>    (field, mResultX, mResultY); break;
            case FIELDTYPE_INT2:    fieldGradient<int2>   (field, mResultX, mResultY); break;
            case FIELDTYPE_INT3:    fieldGradient<int3>   (field, mResultX, mResultY); break;
            case FIELDTYPE_INT4:    fieldGradient<int4>   (field, mResultX, mResultY); break;
            case FIELDTYPE_FLOAT:   fieldGradient<float>  (field, mResultX, mResultY); break;
            case FIELDTYPE_FLOAT2:  fieldGradient<float2> (field, mResultX, mResultY); break;
            case FIELDTYPE_FLOAT3:  fieldGradient<float3> (field, mResultX, mResultY); break;
            case FIELDTYPE_FLOAT4:  fieldGradient<float4> (field, mResultX, mResultY); break;
            case FIELDTYPE_DOUBLE:  fieldGradient<double> (field, mResultX, mResultY); break;
            case FIELDTYPE_DOUBLE2: fieldGradient<double2>(field, mResultX, mResultY); break;
            case FIELDTYPE_DOUBLE3: fieldGradient<double3>(field, mResultX, mResultY); break;
            case FIELDTYPE_DOUBLE4: fieldGradient<double4>(field, mResultX, mResultY); break;
            }
          outputs()[FIELDGRADIENTNODE_OUTPUT_FIELDX]->set(mResultX);
          outputs()[FIELDGRADIENTNODE_OUTPUT_FIELDY]->set(mResultY);
        }
      else
        {
          std::cout << "====> WARNING(FieldGradientNode): ";
          if(mResultX && mResultX->size != Vec2i(0,0) && mResultY && mResultY->size != Vec2i(0,0))
            {
              std::cout << "  resultX: "   << mResultX->size << " | " << mResultX->dataSize << " | " << mResultX->typeSize << " | " << mResultX->type()
                        << "\n  resultY: " << mResultY->size << " | " << mResultY->dataSize << " | " << mResultY->typeSize << " | " << mResultY->type()
                        << "\n  input: "   << field->size    << " | " << field->dataSize    << " | " << field->typeSize    << " | " << field->type() << "\n";
            }
          else
            { std::cout<< "  resultX --> "   << (mResultX ? mResultX->size.toString() : "NULL")
                       << "\n  resultY --> " << (mResultY ? mResultY->size.toString() : "NULL")
                       << "\n  input --> "   << (field    ? field->size.toString()    : "NULL") << "\n"; }
          
          outputs()[FIELDGRADIENTNODE_OUTPUT_FIELDX]->set((CudaFieldBase*)nullptr);
          outputs()[FIELDGRADIENTNODE_OUTPUT_FIELDY]->set((CudaFieldBase*)nullptr);
        }
    }
}

void FieldGradNode::onDraw()
{
  float scale = getScale();
  
  Vec2i fSize = (mInputField ? mInputField->size : Vec2i());  
  std::stringstream ss; ss << (mInputField ? mInputField->type() : FIELDTYPE_INVALID);

  Vec2f p0 = ImGui::GetCursorScreenPos();  
  ImGui::Text("Input: (%d x %d)<%s>", fSize.x, fSize.y, ss.str().c_str());

  drawNabla(ImGui::GetWindowDrawList(), titlePos(), Vec2f(titleSize().y, titleSize().y)*0.9f, Vec4f(1.0f, 1.0f, 1.0f, 1.0f), 2.0f*scale);
}








FieldDivNode::FieldDivNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Field Divergence Node", false)
{
  setTitle("    F");
}

FieldDivNode::~FieldDivNode()
{
  if(mResult) { mResult->destroy(); delete mResult; mResult = nullptr; }
}

void FieldDivNode::resizeField(const Vec2f &fSize)
{
  if(fSize.x > 0 && fSize.y > 0 && mResult && fSize != mResult->size)
    {
      mResult->create(fSize);
    }
}

void FieldDivNode::onUpdate()
{
  CudaFieldBase *field = inputs()[FIELDGRADIENTNODE_INPUT_FIELD]->get<CudaFieldBase>();
  mInputField = field;
  
  if(field)
    {
      if(!mResult || (field && (mResult->type() != field->type() || (mResult->isTexture() != field->isTexture()))))
        {
          if(mResult) { mResult->destroy(); delete mResult; mResult = nullptr; }
          // if(field->isTexture()) { mResult = new CudaFieldTex(); }
          { mResult = makeCudaField(baseType(field->type())); }
          resizeField(field->size);
        }
      else if(mResult)
        { resizeField(field->size); }
      
      if(mResult && field && mResult->allocated() && mResult->size == field->size)
        {
          switch(field->type())
            {
            case FIELDTYPE_INT:     fieldDivergence<int>    (field, mResult); break;
            // case FIELDTYPE_INT2:    fieldDivergence<int2>   (field, (CudaFieldBase*)&mResult); break;
            // case FIELDTYPE_INT3:    fieldDivergence<int3>   (field, (CudaFieldBase*)&mResult); break;
            // case FIELDTYPE_INT4:    fieldDivergence<int4>   (field, (CudaFieldBase*)&mResult); break;
            case FIELDTYPE_FLOAT:   fieldDivergence<float>  (field, mResult); break;
            case FIELDTYPE_FLOAT2:  fieldDivergence<float2> (field, mResult); break;
            // case FIELDTYPE_FLOAT3:  fieldDivergence<float3> (field, (CudaFieldBase*)&mResult); break;
            // case FIELDTYPE_FLOAT4:  fieldDivergence<float4> (field, (CudaFieldBase*)&mResult); break;
            case FIELDTYPE_DOUBLE:  fieldDivergence<double> (field, mResult); break;
            // case FIELDTYPE_DOUBLE2: fieldDivergence<double2>(field, (CudaFieldBase*)&mResult); break;
            // case FIELDTYPE_DOUBLE3: fieldDivergence<double3>(field, (CudaFieldBase*)&mResult); break;
            // case FIELDTYPE_DOUBLE4: fieldDivergence<double4>(field, (CudaFieldBase*)&mResult); break;
            }
          outputs()[FIELDDIVNODE_OUTPUT_FIELD]->set(mResult);
        }
      else
        {
          std::cout << "====> WARNING(FieldDivNode): ";
          if(mResult && mResult->size != Vec2i(0,0))
            {
              std::cout << "  result: "   << mResult->size << " | " << mResult->dataSize << " | " << mResult->typeSize << " | " << mResult->type()
                        << "\n  input: "  << field->size   << " | " << field->dataSize   << " | " << field->typeSize   << " | " << field->type() << "\n";
            }
          else
            {
              std::cout<< "  result --> "   << (mResult ? mResult->size.toString() : "NULL")
                       << "\n  input --> "   << (field  ? field->size.toString()   : "NULL") << "\n";
            }
          
          outputs()[FIELDDIVNODE_OUTPUT_FIELD]->set((CudaFieldBase*)nullptr);
        }
    }
  else
    {
      outputs()[FIELDDIVNODE_OUTPUT_FIELD]->set((CudaFieldBase*)nullptr);
    }
}

void FieldDivNode::onDraw()
{
  float scale = getScale();
  
  Vec2i fSize = (mInputField ? mInputField->size : Vec2i());  
  std::stringstream ss; ss << (mInputField ? mInputField->type() : FIELDTYPE_INVALID);

  Vec2f p0 = ImGui::GetCursorScreenPos();  
  ImGui::Text("Input: (%d x %d)<%s>", fSize.x, fSize.y, ss.str().c_str());

  ImDrawList *drawList = ImGui::GetWindowDrawList();
  drawNabla(drawList, titlePos(), Vec2f(titleSize().y, titleSize().y)*0.9f, Vec4f(1.0f, 1.0f, 1.0f, 1.0f), 2.0f*scale);

  drawList->AddCircleFilled(titlePos()+Vec2f(titleSize().x*0.55f, titleSize().y/2.0f), 4.0f*scale, ImColor(Vec4f(1.0f, 1.0f, 1.0f, 1.0f)), 32);
}







FieldCurlNode::FieldCurlNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Field Curl Node", false)
{
  setTitle("    F");
}

FieldCurlNode::~FieldCurlNode()
{
  if(mResult) { mResult->destroy(); delete mResult; mResult = nullptr; }
  //if(mResultY) { mResultY->destroy(); delete mResultY; mResultY = nullptr; }
}

void FieldCurlNode::resizeField(const Vec2f &fSize)
{
  if(fSize.x > 0 && fSize.y > 0 && mResult && fSize != mResult->size)
    {
      mResult->create(fSize);
      //mResultY->create(fSize);
    }
}

void FieldCurlNode::onUpdate()
{
  CudaFieldBase *field = inputs()[FIELDCURLNODE_INPUT_FIELD]->get<CudaFieldBase>();
  mInputField = field;
  
  if(field)
    {
      if(!mResult || (field && (mResult->type() != field->type() || (mResult->isTexture() != field->isTexture()))))
        {
          if(mResult) { mResult->destroy(); delete mResult; mResult = nullptr; }
          //if(mResultY) { mResultY->destroy(); delete mResultY; mResultY = nullptr; }
          //if(field->isTexture()) { mResult = new CudaFieldTex(); } //mResultY = new CudaFieldTex(); }
           { mResult = makeCudaField(FIELDTYPE_FLOAT); } //mResultY = makeCudaField(field->type()); }
          resizeField(field->size);
        }
      else if(mResult)// && mResultY)
        { resizeField(field->size); }
      
      if(mResult && field && mResult->allocated() && mResult->size == field->size) // && mResultY->allocated() && mResultY->size == field->size)
        {
          switch(field->type())
            {
              //case FIELDTYPE_INT:     fieldCurl<int>    (field, mResult); break;
            case FIELDTYPE_INT2:    fieldCurl<int2>   (field, mResult); break;
            // case FIELDTYPE_INT3:    fieldCurl<int3>   (field, mResult); break;
            // case FIELDTYPE_INT4:    fieldCurl<int4>   (field, mResult); break;
              //case FIELDTYPE_FLOAT:   fieldCurl<float>  (field, mResult); break;
            case FIELDTYPE_FLOAT2:  fieldCurl<float2> (field, mResult); break;
            // case FIELDTYPE_FLOAT3:  fieldCurl<float3> (field, mResult); break;
            // case FIELDTYPE_FLOAT4:  fieldCurl<float4> (field, mResult); break;
              //case FIELDTYPE_DOUBLE:  fieldCurl<double> (field, mResult); break;
            case FIELDTYPE_DOUBLE2: fieldCurl<double2>(field, mResult); break;
            // case FIELDTYPE_DOUBLE3: fieldCurl<double3>(field, mResult); break;
            // case FIELDTYPE_DOUBLE4: fieldCurl<double4>(field, mResult); break;
            }
          outputs()[FIELDCURLNODE_OUTPUT_FIELD]->set(mResult);
          //outputs()[FIELDCURLNODE_OUTPUT_FIELDY]->set(mResultY);
        }
      else
        {
          std::cout << "====> WARNING(FieldCurlNode): ";
          if(mResult && mResult->size != Vec2i(0,0)) // && mResultY && mResultY->size != Vec2i(0,0))
            {
              std::cout << "  resultX: "   << mResult->size << " | " << mResult->dataSize << " | " << mResult->typeSize << " | " << mResult->type()
                //<< "\n  resultY: " << mResultY->size << " | " << mResultY->dataSize << " | " << mResultY->typeSize << " | " << mResultY->type()
                        << "\n  input: "   << field->size    << " | " << field->dataSize    << " | " << field->typeSize    << " | " << field->type() << "\n";
            }
          else
            { std::cout<< "  resultX --> "   << (mResult ? mResult->size.toString() : "NULL")
                //<< "\n  resultY --> " << (mResultY ? mResultY->size.toString() : "NULL")
                       << "\n  input --> "   << (field    ? field->size.toString()    : "NULL") << "\n"; }
          
          outputs()[FIELDCURLNODE_OUTPUT_FIELD]->set((CudaFieldBase*)nullptr);
          //outputs()[FIELDCURLNODE_OUTPUT_FIELDY]->set((CudaFieldBase*)nullptr);
        }
    }
}

void FieldCurlNode::onDraw()
{
  float scale = getScale();
  
  Vec2i fSize = (mInputField ? mInputField->size : Vec2i());  
  std::stringstream ss; ss << (mInputField ? mInputField->type() : FIELDTYPE_INVALID);

  Vec2f p0 = ImGui::GetCursorScreenPos();  
  ImGui::Text("Input: (%d x %d)<%s>", fSize.x, fSize.y, ss.str().c_str());

  ImDrawList *drawList = ImGui::GetWindowDrawList();
  drawNabla(drawList, titlePos(), Vec2f(titleSize().y, titleSize().y)*0.9f, Vec4f(1.0f, 1.0f, 1.0f, 1.0f), 2.0f*scale);
  Vec2f xCenter = titlePos()+Vec2f(titleSize().x*0.55f, titleSize().y*0.5f);
  Vec2f xSize   = Vec2f(titleSize().y/3.0f, titleSize().y/3.0f);
  drawList->AddLine(xCenter - xSize/2.0f, xCenter + xSize/2.0f, ImColor(Vec4f(1.0f, 1.0f, 1.0f, 1.0f)), 2.0f*scale);
  drawList->AddLine(xCenter - Vec2f(xSize.x, -xSize.y)/2.0f, xCenter + Vec2f(xSize.x, -xSize.y)/2.0f, ImColor(Vec4f(1.0f, 1.0f, 1.0f, 1.0f)), 2.0f*scale);
}


