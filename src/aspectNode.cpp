#include "aspectNode.hpp"
using namespace astro;

#include "imgui.h"
#include "tools.hpp"
#include "chartView.hpp"

AspectNode::AspectNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Aspect Node")
{
  setMinSize(Vec2f(384, 0));
  for(int a = 0; a < ASPECT_COUNT; a++)
    {
      mAspVisible.push_back(true); // start with all aspects visible
      mAspOrbs.push_back(getAspect((AspectType)a)->orb); // default orbs
    }
}

bool AspectNode::onDraw()
{
  float scale = getScale();
  Vec2f symSize = Vec2f(20, 20)*scale;
  
  bool changed = false;
  Chart *chart = inputs()[ASPECTNODE_INPUT_CHART]->get<Chart>();
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_Framed;
  
  // aspect list
  ImGui::SetNextItemWidth(384);
  ImGui::SetNextTreeNodeOpen(mListOpen);
  if(ImGui::CollapsingHeader("aspectList", nullptr, flags) && chart)
    {
      mListOpen = true;
      // update aspect visiblity and count visible aspects
      long visibleCount = 0;
      for(int i = 0; i < chart->aspects().size(); i++)
        {
          mAspVisible[i] = chart->getAspectVisible(chart->aspects()[i].type);
          if(mAspVisible[i] && chart->aspects()[i].obj1->visible && chart->aspects()[i].obj2->visible)
            { visibleCount++; }
        }
        
      ImGui::Text("Total count: %d (visible: %d)", (int)chart->aspects().size(), (int)visibleCount);
      ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

      ImGui::BeginChild("##listChild", Vec2f(384,512)*scale);
      {
        ImGui::SetWindowFontScale(scale);
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        for(int i = 0; i < chart->aspects().size(); i++)
          {
            const Aspect &asp = chart->aspects()[i];
            // skip north/south node opposition, and non-visible aspects
            if((asp.obj1->type == OBJ_NORTHNODE && asp.obj2->type == OBJ_SOUTHNODE) ||
               (asp.obj2->type == OBJ_NORTHNODE && asp.obj1->type == OBJ_SOUTHNODE) ||
               !mAspVisible[i] || !asp.visible || !asp.obj1->visible || !asp.obj2->visible) { continue; }
              
            std::string aName = getAspectName(asp.type);
            std::string o1Name = getObjName(asp.obj1->type);
            std::string o2Name = getObjName(asp.obj2->type);
            Vec4f aColor = getAspect(asp.type)->color;
            Vec4f scaledColor = aColor;
            scaledColor.w *= asp.strength;//*asp.strength; // weight surrounding hexagon color by aspect strength
            Vec4f o1Color = getObjColor(o1Name);
            Vec4f o2Color = getObjColor(o2Name);

            ImGui::Spacing();
            ImGui::Text("%s", angle_string(asp.orb, true).c_str());
            {
              Vec2f centerOffset = Vec2f(164-14,-14)*scale;
              ImGui::SameLine(); ImGui::Image(getWhiteImage(o1Name)->id(), symSize, Vec2f(0,0), Vec2f(1,1), o1Color, Vec4f(0,0,0,0));
              ImGui::SameLine(); ImGui::Image(getImage(aName)->id(),       symSize, Vec2f(0,0), Vec2f(1,1), aColor,  Vec4f(0,0,0,0));
              draw_list->AddCircle(Vec2f(ImGui::GetCursorScreenPos())+centerOffset, symSize.x*0.7f, ImColor(scaledColor), 6, 1);
              ImGui::SameLine(); ImGui::Image(getWhiteImage(o2Name)->id(), symSize, Vec2f(0,0), Vec2f(1,1), o2Color, Vec4f(0,0,0,0));
            }
            ImGui::SameLine(); ImGui::TextUnformatted(aName.c_str());
            ImGui::Spacing();
          }
      }
      ImGui::EndChild();
    }
  else if(chart && isBodyVisible())
    { mListOpen = false; }
    
  // aspect settings/orbs
  ImGui::SetNextTreeNodeOpen(mOrbsOpen);
  if(ImGui::CollapsingHeader("orbs", nullptr, flags) && chart)
    {
      mOrbsOpen = true;
      if(ImGui::BeginTable("##aspectCols", 4)) // COLUMNS --> aspect enabledCheck(0), symbol(1), name(2), orb(3)
        {
          ImGui::TableSetupColumn("ENABLE", ImGuiTableColumnFlags_WidthAlwaysAutoResize);
          ImGui::TableSetupColumn("SYMBOL", ImGuiTableColumnFlags_WidthAlwaysAutoResize);
          ImGui::TableSetupColumn("NAME",   ImGuiTableColumnFlags_WidthAlwaysAutoResize);
          ImGui::TableSetupColumn("ORB",    ImGuiTableColumnFlags_WidthAlwaysAutoResize);
          for(int i = 0; i < ASPECT_COUNT; i++)
            {
              std::string name = getAspectName((AspectType)i);
              float orbVal = chart->getAspectOrb((AspectType)i);
              float orbStep = 0.1f;
              float orbFastStep = 1.0f;
          
              ImGui::TableNextRow();
              ImGui::TableSetColumnIndex(0);

              bool enabled = chart->getAspectVisible((AspectType)i);
              if(ImGui::Checkbox(("##enableCheck-"+name).c_str(), &enabled))
                { chart->setAspectVisible((AspectType)i, enabled); }
                
              ImGui::TableSetColumnIndex(1);
              Vec4f color = getAspect((AspectType)i)->color;
              ChartImage *img = getImage(name);
              ImGui::Image(reinterpret_cast<ImTextureID*>(img->texId), symSize, Vec2f(0,0), Vec2f(1,1), color, Vec4f(0,0,0,0));
                
              ImGui::TableSetColumnIndex(2);
              ImGui::TextUnformatted(name.c_str());
          
              ImGui::TableSetColumnIndex(3);
              ImGui::SetNextItemWidth(120*scale);
              if(ImGui::InputFloat(("##aspOrbInput-"+name).c_str(), &orbVal, orbStep, orbFastStep, "%.2f°"))
                {
                  std::cout << "ASPECT --> Setting orb: " << i << " : " << orbVal << "\n";
                  chart->setAspectOrb((AspectType)i, orbVal);
                  changed = true;
                }
            }
          ImGui::EndTable();
        }
        
      if(changed) { chart->update(); }
    }
  else if(chart && isBodyVisible())
    { mOrbsOpen = false; }
  
  return true;
}

