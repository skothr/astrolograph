#include "chartParamWidget.hpp"
using namespace astro;

#include "chart.hpp"
#include "chartView.hpp"
#include "glfwKeys.hpp"
#include "imgui.h"

ChartParamWidget::ChartParamWidget(ChartParams *params)
  : mParams(params)
{

}

void ChartParamWidget::draw(float scale, bool blocked, bool visible)
{
  if(!mParams) { return; }

  ImGuiIO& io = ImGui::GetIO();
  float symSize = 20.0f*scale;

  // ChartParams mFrameParams = mLocalParams;
  // ChartParams params = *mParams;
  
  // options
  ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth);
  ImGui::SetNextTreeNodeOpen(mSettingsOpen);
  if(ImGui::CollapsingHeader("Settings", nullptr, flags))
    {
      if(!blocked) { mSettingsOpen = true; }
      ImGui::Indent();
      
      float columnW = 180.0f*scale;
      
      { // size of chart
        ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(0.0f, 2.0f));
        ImGui::TextUnformatted("Chart Size ");
        ImGui::SameLine(columnW);
        ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) - Vec2f(0.0f, 2.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0.0f, 0.0f));
        // ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, ImGui::GetStyle().GrabMinSize*scale);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, Vec2f(2.0f, 2.0f)*scale);
        ImGui::SetNextItemWidth(330*scale);
        ImGui::SliderFloat("##chartWidth", &mParams->chartWidth, CHART_SIZE_MIN, CHART_SIZE_MAX, "%.0f");
        ImGui::PopStyleVar(2);
      }

      // align ascendant
      ImGui::TextUnformatted("Align Ascendant");
      ImGui::SameLine(columnW);
      bool align = mParams->alignAsc;
      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
      if(ImGui::Checkbox("##align", &align))
        { mParams->alignAsc = align; }
      
      // show houses 
      ImGui::TextUnformatted("Show Houses    ");
      ImGui::SameLine(columnW);
      bool show = mParams->showHouses;
      if(ImGui::Checkbox("##show", &show))
        { mParams->showHouses = show; }
      ImGui::PopStyleVar();
      
      // OBJECT VISIBILITY //
      // display settings (toggle object/angle visibility)
      ImGui::SetNextTreeNodeOpen(mObjDisplayOpen);
      if(ImGui::CollapsingHeader("Object Display", nullptr, flags))
        {
          mObjDisplayOpen = true;

          ImGui::Indent();
          // ImGui::PushFont(viewSettings->titleFont);
          // ImGui::TextUnformatted("Visibility");
          // ImGui::PopFont();
          ImGui::Separator();
          // ImGui::Indent();
          
          ImGui::BeginTable("##angVis", 4, ImGuiTableFlags_SizingPolicyStretchX | ImGuiTableFlags_NoClip);
          ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
          {
            ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0);
            // angles
            // ImGui::BeginGroup();
            int numPerColumn = 4;
            bool grouping = false;
            for(int a = ANGLE_OFFSET; a < ANGLE_END; a++)
              {
                if((a-ANGLE_OFFSET) % numPerColumn == 0)
                  {
                    if(a > ANGLE_OFFSET) { ImGui::Spacing(); }
                    ImGui::BeginGroup();
                    grouping = true;
                  }
                int i = OBJ_COUNT+a-ANGLE_OFFSET; // obj index
                std::string name = getObjName(a);
                std::string longName = getObjNameLong(a);
                ChartImage *img = getWhiteImage(name);
                Vec4f color = getObjColor(name);
                ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

                ImGui::BeginGroup();
                {
                  bool checked = mParams->objVisible[i];
                  // if(a > ANGLE_OFFSET) { ImGui::SameLine(columnW*(a-ANGLE_OFFSET)); }
                  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
                  if(ImGui::Checkbox(("##show-"+name).c_str(), &checked))
                    {
                      mParams->objVisible[i] = checked;
                    }
                  ImGui::PopStyleVar();
                  
                  ImGui::SetNextItemWidth(symSize+10.0f*scale);
                  ImGui::SameLine(); ImGui::Image(img->id(), ImVec2(symSize, symSize), ImVec2(0,0), ImVec2(1,1), tintCol, ImVec4(0,0,0,0));
                  ImGui::SameLine(); ImGui::Text("%s", longName.c_str());
                }
                ImGui::EndGroup();
                
                bool hover = ImGui::IsItemHovered();
                if(mChart)
                  {
                    if(hover)
                      { // angle tooltip
                        ChartObject *obj = mChart->getObject(i);
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  Vec2f(ImGui::GetStyle().FramePadding)/scale);
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   Vec2f(ImGui::GetStyle().ItemSpacing)/scale);
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(ImGui::GetStyle().WindowPadding)/scale);
                        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetStyle().IndentSpacing/scale);
                        ImGui::SetTooltip("%s - %s %s", name.c_str(), getSignName(mChart->getSign(obj->angle)).c_str(),
                                          angle_string(fmod(obj->angle, 30.0), false).c_str());
                        ImGui::PopStyleVar(4);
                      }
                    // set focus
                    bool focused = (hover && io.KeyShift); // focus on this object with SHIFT+hover
                    if(focused || !mParams->objFocused[i] || (mParams->objFocused[i] && mObjDisplayFocused[i]))
                      {
                        mParams->objFocused[i] = focused;
                        mObjDisplayFocused[i] = focused;
                      }
                  }
                
                if((a-ANGLE_OFFSET) % numPerColumn == (numPerColumn-1))
                  {
                    ImGui::EndGroup();
                    ImGui::TableNextColumn();
                    grouping = false;
                  }
                //ImGui::TableNextColumn();
              }
            if(grouping)
              {
                // ImGui::Spacing();
                ImGui::EndGroup();
                grouping = false;
              }

            ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0);
            ImGui::Separator();
            
            // objects
            numPerColumn = 5;
            for(int i = 0; i < OBJ_COUNT; i++)
              {
                if(i % numPerColumn == 0)
                  {
                    if(i > 0) { ImGui::Spacing(); }
                    ImGui::BeginGroup();
                    grouping = true;
                  }
              
                std::string name = getObjName(i);
                std::string longName = getObjNameLong(i);
                ChartImage *img = getWhiteImage(name);
                Vec4f color = getObjColor(name);
                ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

                ImGui::BeginGroup();
                {
                  bool checked = mParams->objVisible[i];
                  
                  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
                  if(ImGui::Checkbox(("##show-"+name).c_str(), &checked))
                    {
                      mParams->objVisible[i] = checked;
                    }
                  ImGui::PopStyleVar();
              
                  ImGui::SetNextItemWidth(symSize+10.0f*scale);
                  ImGui::SameLine(); ImGui::Image(img->id(), ImVec2(symSize, symSize), ImVec2(0,0), ImVec2(1,1), tintCol, ImVec4(0,0,0,0));
                  ImGui::SameLine(); ImGui::Text("%s", longName.c_str());
                }
                ImGui::EndGroup();
                
                bool hover = !blocked && ImGui::IsItemHovered();
                if(mChart)
                  {
                    if(hover)
                      { // object tooltip
                        ChartObject *obj = mChart->getObject(i);
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  Vec2f(ImGui::GetStyle().FramePadding)/scale);
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   Vec2f(ImGui::GetStyle().ItemSpacing)/scale);
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(ImGui::GetStyle().WindowPadding)/scale);
                        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetStyle().IndentSpacing/scale);
                        ImGui::SetTooltip("%s - %s %s", name.c_str(), getSignName(mChart->getSign(obj->angle)).c_str(),
                                          angle_string(fmod(obj->angle, 30.0), false).c_str());
                        ImGui::PopStyleVar(4);
                      }
                    // set focus
                    bool focused = (hover && io.KeyShift); // focus on this object with SHIFT+hover
                    if(focused || !mParams->objFocused[i] || (mParams->objFocused[i] && mObjDisplayFocused[i]))
                      {
                        mParams->objFocused[i] = focused;
                        mObjDisplayFocused[i] = focused;
                      }
                  }

                if(i % numPerColumn == (numPerColumn-1))
                  {
                    ImGui::EndGroup();
                    ImGui::TableNextColumn();
                    grouping = false;
                  }
              }
            if(grouping)
              {
                ImGui::EndGroup();
                grouping = false;
              }
          }
          ImGui::Separator();
          ImGui::EndTable();
          // ImGui::Unindent();
          ImGui::Unindent();
        }
      else if(visible) { mObjDisplayOpen = false; }

      // OBJECT ORBS //
      ImGui::SetNextTreeNodeOpen(mObjOrbsOpen);
      if(ImGui::CollapsingHeader("Object Orbs", nullptr, flags))
        {
          mObjOrbsOpen = true;
          ImGui::Indent();
          // ImGui::PushFont(viewSettings->titleFont);
          // ImGui::TextUnformatted("Orbs");
          // ImGui::PopFont();
          ImGui::Separator();
          // ImGui::Indent();
          
          ImGui::BeginTable("##orbs", 4, ImGuiTableFlags_SizingPolicyStretchX | ImGuiTableFlags_NoClip);
          ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
          {
            ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0);
          
            // objects
            int numPerColumn = 5;
            bool grouping = false;
            for(int i = OBJ_SUN; i < OBJ_COUNT; i++)
              {
                if(i % numPerColumn == 0)
                  { 
                    ImGui::BeginGroup();
                    grouping = true;
                  }
                
                std::string name = getObjName(i);
                std::string longName = getObjNameLong(i);
                ChartImage *img = getWhiteImage(name);
                Vec4f color = getObjColor(name);
                ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

                ImGui::BeginGroup();
                {
                  // ImGui::SetNextItemWidth(50*scale);
                  // ImGui::InputDouble(("##setorb"+std::to_string(i)).c_str(), &mParams->orbs.objOrbs[i], 0.0, 0.0, "%.2f", ImGuiInputTextFlags_None);
                  ImGui::SetNextItemWidth(symSize+10.0f*scale);
                  ImGui::SameLine(); ImGui::Image(img->id(), ImVec2(symSize, symSize), ImVec2(0,0), ImVec2(1,1), tintCol, ImVec4(0,0,0,0));
                  ImGui::SameLine(); ImGui::Text("%s", longName.c_str());
                }
                ImGui::EndGroup();
                
                bool hover = !blocked && ImGui::IsItemHovered();
                if(mChart)
                  {
                    if(hover)
                      { // object tooltip
                        ChartObject *obj = mChart->getObject(i);
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  Vec2f(ImGui::GetStyle().FramePadding)/scale);
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   Vec2f(ImGui::GetStyle().ItemSpacing)/scale);
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(ImGui::GetStyle().WindowPadding)/scale);
                        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetStyle().IndentSpacing/scale);
                        ImGui::SetTooltip("%s - %s %s", name.c_str(), getSignName(mChart->getSign(obj->angle)).c_str(),
                                          angle_string(fmod(obj->angle, 30.0), false).c_str());
                        ImGui::PopStyleVar(4);
                      }
                    // set focus
                    bool focused = (hover && io.KeyShift); // focus on this object with SHIFT+hover
                    if(focused || !mParams->objFocused[i] || (mParams->objFocused[i] && mObjOrbsFocused[i]))
                      {
                        mParams->objFocused[i] = focused;
                        mObjOrbsFocused[i] = focused;
                      }
                  }
                if(i % numPerColumn == (numPerColumn-1))
                  {
                    ImGui::EndGroup();
                    ImGui::TableNextColumn();
                    grouping = false;
                  }
              }
            if(grouping)
              {
                ImGui::EndGroup();
                grouping = false;
              }
            ImGui::Separator();
          }
          ImGui::EndTable();
          ImGui::Unindent();
        }
      else if(visible) { mObjOrbsOpen = false; }


      
      std::vector<ChartAspect> aspects;
      std::array<int, ASPECT_COUNT> counts = {0};
      if(mChart)
        {
          aspects = mChart->calcAspects(*mParams);
          for(auto asp : aspects) { counts[asp.type]++; }
        }

      
      // ASPECT VISIBILITY //
      ImGui::SetNextTreeNodeOpen(mAspDisplayOpen);
      if(ImGui::CollapsingHeader("Aspect Display", nullptr, flags))
        {
          mAspDisplayOpen = true;

          ImGui::Indent();
          // ImGui::PushFont(viewSettings->titleFont);
          // ImGui::TextUnformatted("Visibility");
          // ImGui::PopFont();
          ImGui::Separator();
          // ImGui::Indent();
          
          // angles
          int numPerColumn = 4;
          bool grouping = false;
          for(int a = 0; a < ASPECT_COUNT; a++)
            {
              int i = a; // aspect type index
              std::string name = getAspectName(a);
              std::string longName = getAspectNameLong(a);
              ChartImage *img = getImage(name);
              Vec4f color = getAspectInfo(a)->color;
              ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

              ImGui::BeginGroup();
              {
                bool checked = mParams->aspVisible[i];
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
                if(ImGui::Checkbox(("##show-"+name).c_str(), &checked))
                  {
                    mParams->aspVisible[i] = checked;
                    // if(mChart) { mChart->setAspectVisible(a, checked); }
                  }
                ImGui::PopStyleVar();
                
                ImGui::SetNextItemWidth(symSize+10.0f*scale);
                ImGui::SameLine(); ImGui::Image(img->id(), ImVec2(symSize, symSize), ImVec2(0,0), ImVec2(1,1), tintCol, ImVec4(0,0,0,0));
                ImGui::SameLine(); ImGui::Text("%s", longName.c_str());
              }
              ImGui::EndGroup();
                
              bool hover = ImGui::IsItemHovered();
              if(mChart)
                {
                  if(hover)
                    { // aspect tooltip
                      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  Vec2f(ImGui::GetStyle().FramePadding)/scale);
                      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   Vec2f(ImGui::GetStyle().ItemSpacing)/scale);
                      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(ImGui::GetStyle().WindowPadding)/scale);
                      ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetStyle().IndentSpacing/scale);
                      ImGui::SetTooltip("%s (%d)", getAspectName(a).c_str(), counts[a]);
                      ImGui::PopStyleVar(4);
                    }   
                  // set focus
                  bool focused = (hover && io.KeyShift); // focus on this aspect type with SHIFT+hover
                  if(focused || !mParams->aspFocused[a] || (mParams->aspFocused[a] && mAspDisplayFocused[a]))
                    {
                      mParams->aspFocused[a] = focused;
                      mAspDisplayFocused[a] = focused;
                    }
                  
                }
            }
          ImGui::Separator();
          ImGui::Unindent();
        }
      else if(visible) { mAspDisplayOpen = false; }

      
      // ASPECT ORBS //
      ImGui::SetNextTreeNodeOpen(mAspOrbsOpen);
      if(ImGui::CollapsingHeader("Aspect Orbs", nullptr, flags))
        {
          mAspOrbsOpen = true;
          ImGui::Indent();
          // ImGui::PushFont(viewSettings->titleFont);
          // ImGui::TextUnformatted("Orbs");
          // ImGui::PopFont();
          ImGui::Separator();
          // ImGui::Indent();
          
            // objects
            int numPerColumn = 5;
            for(int i = 0; i < ASPECT_COUNT; i++)
              {              
                std::string name = getAspectName(i);
                std::string longName = getAspectNameLong(i);
                ChartImage *img = getImage(name);
                Vec4f color = getAspectInfo(i)->color;
                ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

                ImGui::BeginGroup();
                {
                  // ImGui::SetNextItemWidth(50*scale);
                  // ImGui::InputDouble(("##setorb"+std::to_string(i)).c_str(), &mParams->aspOrbs[i], 0.0, 0.0, "%.2f", ImGuiInputTextFlags_None);
                  ImGui::SetNextItemWidth(symSize+10.0f*scale);
                  ImGui::SameLine(); ImGui::Image(img->id(), ImVec2(symSize, symSize), ImVec2(0,0), ImVec2(1,1), tintCol, ImVec4(0,0,0,0));
                  ImGui::SameLine(); ImGui::Text("%s", longName.c_str());
                }
                ImGui::EndGroup();
                
                bool hover = !blocked && ImGui::IsItemHovered();
                if(mChart)
                  {
                    if(hover)
                      { // aspect tooltip
                      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  Vec2f(ImGui::GetStyle().FramePadding)/scale);
                      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   Vec2f(ImGui::GetStyle().ItemSpacing)/scale);
                      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(ImGui::GetStyle().WindowPadding)/scale);
                      ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetStyle().IndentSpacing/scale);
                      ImGui::SetTooltip("%s (%d)", name.c_str(), counts[i]);
                      ImGui::PopStyleVar(4);
                      }
                    
                    // set focus
                    bool focused = (hover && io.KeyShift); // focus on this object with SHIFT+hover
                    if(focused || !mParams->aspFocused[i] || (mParams->aspFocused[i] && mAspOrbsFocused[i]))
                    {
                      mParams->aspFocused[i] = focused;
                      mAspOrbsFocused[i] = focused;
                    }
                  }
              }
            ImGui::Separator();
          ImGui::Unindent();
        }
      else if(visible) { mAspOrbsOpen = false; }
      ImGui::Unindent();
    } 
  else if(visible) { mSettingsOpen = false; }
}
