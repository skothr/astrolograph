#include "chartOrbWidget.hpp"
using namespace astro;

#include <imgui.h>

ChartOrbWidget::ChartOrbWidget()
{
  for(int i = 0; i < OBJ_END; i++)      { mConstObjOrbs.push_back(false); }
  for(int j = 0; j < ASPECT_COUNT; j++) { mConstAspOrbs.push_back(false); }
}

ChartOrbWidget::~ChartOrbWidget()
{
  
}

void ChartOrbWidget::draw(float scale)
{
  ImGui::BeginGroup();
  {
    ImGuiInputTextFlags iFlags = (//ImGuiInputTextFlags_EnterReturnsTrue |
                                  ImGuiInputTextFlags_CharsNoBlank |
                                  ImGuiInputTextFlags_CharsDecimal
                                  );
    float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
    float columnW = 42.0f*scale + 2*spacing;
    Vec2f symSize = Vec2f(20.0f, 20.0f)*scale;
    float padding = (columnW - symSize.x) / 2.0f;
    float column1W = symSize.x*2 + spacing;
    
    // constant checkboxes
    ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(column1W+spacing+padding, 0));
    for(int j = 0; j < ASPECT_COUNT; j++)
      {
        if(ImGui::Checkbox(("##asp"+std::to_string(j)).c_str(), &mConstAspOrbs[j].data))
          {
            if(&mConstAspOrbs[j])
              {
                for(int i = 0; i < OBJ_END; i++)
                  { mOrbs.objOrbs[i][j] = mOrbs.objOrbs[0][j]; }
              }
          }
        if(j < ASPECT_COUNT-1) { ImGui::SameLine(column1W + spacing + columnW*(j+1) + padding); }
      }
    
    // aspect labels (top horizontal header)
    ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(column1W+spacing+padding, 0));
    for(int j = 0; j < ASPECT_COUNT; j++)
      {
        std::string aspName  = getAspectName(j);
        Vec4f       aspColor = getAspectInfo(j)->color;
        // draw aspect symbol
        ChartImage *img = getImage(aspName);
        ImGui::Image(img->id(), symSize, Vec2f(0,0), Vec2f(1,1), aspColor, Vec4f(0,0,0,0));
        if(j < ASPECT_COUNT-1) { ImGui::SameLine(column1W + spacing + columnW*(j+1) + padding); }
      }
    
    // object rows
    for(int i = 0; i < OBJ_END; i++)
      {
        std::string objName = getObjName(i);
        if(ImGui::Checkbox(("##"+objName).c_str(), &mConstObjOrbs[i].data))
          {
            if(&mConstObjOrbs[i])
              {
                for(int j = 0; j < ASPECT_COUNT; j++)
                  { mOrbs.objOrbs[i][j] = mOrbs.objOrbs[i][0]; }
              }
          }
        ChartImage *img = getWhiteImage(objName);
        ImGui::SameLine();
        ImGui::Image(img->id(), symSize, Vec2f(0,0), Vec2f(1,1), getObjColor(i), Vec4f(0,0,0,0));
        
        for(int j = 0; j < ASPECT_COUNT; j++)
          {
            ImGui::SameLine(column1W + spacing + columnW*j + spacing);
            ImGui::SetNextItemWidth(columnW - 2*spacing);
            if(ImGui::InputFloat(("##obj"+std::to_string(i)+std::to_string(j)).c_str(), &mOrbs.objOrbs[i][j], 0.0f, 0.0f, "%.2f", iFlags))
              {
                if(mConstObjOrbs[i])
                  {
                    for(int j2 = 0; j2 < ASPECT_COUNT; j2++)
                      { mOrbs.objOrbs[i][j2] = mOrbs.objOrbs[i][j]; }
                  }
                if(mConstAspOrbs[j])
                  {
                    for(int i2 = 0; i2 < OBJ_END; i2++)
                      { mOrbs.objOrbs[i2][j] = mOrbs.objOrbs[i][j]; }
                  }
              }
          }
      }
  }
  ImGui::EndGroup();
}
