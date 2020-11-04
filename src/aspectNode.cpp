#include "aspectNode.hpp"
using namespace astro;

#include "imgui.h"
#include "tools.hpp"
#include "chartView.hpp"


#define ASPECTNODE_INPUT_CHART 0

static std::vector<ConnectorBase*> CONNECTOR_INPUTS()
{ return {new Connector<Chart>("Chart")}; }
static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
{ return {}; }

AspectNode::AspectNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Aspect Node")
{
  setMinSize(Vec2f(384, 0));
}

bool AspectNode::onDraw()
{
  bool changed = false;
  Chart *chart = inputs()[ASPECTNODE_INPUT_CHART]->get<Chart>();
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding;
  
  ImGui::BeginGroup();
  {
    // aspect list
    if(ImGui::CollapsingHeader("aspectList", nullptr, flags) && chart)
      {
        // count visible aspects
        int visibleCount = 0;
        for(int i = 0; i < chart->aspects().size(); i++)
          {
            if(chart->getAspectVisible(chart->aspects()[i].type) &&
              chart->aspects()[i].obj1->visible && chart->aspects()[i].obj2->visible)
              { visibleCount++; }
          }
        
        ImGui::Text("Total count: %d (visible: %d)", chart->aspects().size(), visibleCount);
        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

        ImGui::BeginChild("##listChild", Vec2f(384,512));
        {
          ImDrawList *draw_list = ImGui::GetWindowDrawList();
          for(int i = 0; i < chart->aspects().size(); i++)
            {
              const Aspect &asp = chart->aspects()[i];
              // skip north/south node opposition, and non-visible aspects
              if((asp.obj1->type == OBJ_NORTHNODE && asp.obj2->type == OBJ_SOUTHNODE) ||
                 (asp.obj2->type == OBJ_NORTHNODE && asp.obj1->type == OBJ_SOUTHNODE) ||
                 !chart->getAspectVisible(asp.type) || !asp.visible || !asp.obj1->visible || !asp.obj2->visible) { continue; }
              
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
                ImGui::SameLine(); ImGui::Image((ImTextureID)getWhiteImage(o1Name)->texId, Vec2f(20,20), Vec2f(0,0), Vec2f(1,1), o1Color, Vec4f(0,0,0,0));
                ImGui::SameLine(); ImGui::Image((ImTextureID)getImage(aName)->texId,       Vec2f(20,20), Vec2f(0,0), Vec2f(1,1), aColor,  Vec4f(0,0,0,0));
                draw_list->AddCircle(Vec2f(ImGui::GetCursorScreenPos())+Vec2f(164-14,-14), 20.0f*0.7f, ImColor(scaledColor), 6, 1);
                ImGui::SameLine(); ImGui::Image((ImTextureID)getWhiteImage(o2Name)->texId, Vec2f(20,20), Vec2f(0,0), Vec2f(1,1), o2Color, Vec4f(0,0,0,0));
              }
              ImGui::SameLine(); ImGui::TextUnformatted(aName.c_str());
              ImGui::Spacing();
            }
        }
        ImGui::EndChild();
      }

    
    // aspect settings/orbs
    if(ImGui::CollapsingHeader("orbs", nullptr, flags) && chart)
      {
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
                ImGui::Image((ImTextureID)img->texId, Vec2f(20, 20), Vec2f(0,0), Vec2f(1,1), color, Vec4f(0,0,0,0));
                
                ImGui::TableSetColumnIndex(2);
                ImGui::Text(name.c_str());
          
                ImGui::TableSetColumnIndex(3);
                ImGui::SetNextItemWidth(120);
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
  }  
  // ImGui::EndChild();
  ImGui::EndGroup();
  
  return true;
}

