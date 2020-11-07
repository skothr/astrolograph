#include "chartDataNode.hpp"
using namespace astro;

#include "imgui.h"
#include "imgui_internal.h" // (for Push/Pop ItemFlag)
#include "tools.hpp"
#include "chartView.hpp"


ChartDataNode::ChartDataNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Chart Data Node")
{
  setMinSize(Vec2f(880, 0));
}

ChartDataNode::~ChartDataNode()
{ }

bool ChartDataNode::onDraw()
{
  bool changed = false;
  Chart *chart = inputs()[DATANODE_INPUT_CHART]->get<Chart>();

  ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow |
                              ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_FramePadding);
  
  ImGui::BeginGroup();
  {
    // if(!chart)
    //   { // disable tree nodes
    //     ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    //     ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    //   }

    // Angle Data
    if(ImGui::CollapsingHeader("Angle Data", nullptr, flags) && chart)
      {
        mAngOpen = true;
        int symSize = 20;
        std::string headers[] = { "", "   LONGITUDE" };

        ImGui::Columns(2, "data-table##angles", false);
        ImGui::SetColumnWidth(0, 2.0f*symSize+30.0f);

        // column headers
        for(int i = 0; i < 2; i++)
          { ImGui::Text(headers[i].c_str()); ImGui::NextColumn(); }
        ImGui::Separator();
                
        ImGuiIO& io = ImGui::GetIO();
        astro::Ephemeris &swe = chart->swe();
            
        for(int o = ANGLE_OFFSET; o < ANGLE_END; o++)
          {
            double angle = swe.getAngle((ObjType)o);
            std::string name = getObjName((astro::ObjType)o);
                
            ChartImage *img = getWhiteImage(name);
            Vec4f color = getObjColor(name);
            ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

            bool checked = chart->getObject((ObjType)o)->visible;
            ImGui::Checkbox(("##show-"+name).c_str(), &checked);
            chart->showObject((ObjType)o, checked);

            ImGui::SetNextItemWidth(symSize+10.0f);
            ImGui::SameLine(); ImGui::Image((ImTextureID)img->texId, ImVec2(symSize, symSize), ImVec2(0,0), ImVec2(1,1), tintCol, ImVec4(0,0,0,0));
            bool hover = ImGui::IsItemHovered();
            if(hover) { ImGui::SetTooltip((name + " - " + angle_string(angle)).c_str()); }
                
            if(hover && io.KeyShift) // focus on this object when SHIFT+hover
              { chart->showObject((ObjType)o, true); chart->setObjFocus((ObjType)o, hover); }
            else
              { chart->setObjFocus((ObjType)o, false); }
            ImGui::NextColumn();
            ImGui::Text(angle_string(angle).c_str()); ImGui::NextColumn();
          }
        ImGui::Columns(1);
      }
    else
      { mAngOpen = false; }
    
    // Object Data
    if(ImGui::CollapsingHeader("Object Data", nullptr, flags) && chart)
      {
        mObjOpen = true;
        int symSize = 20;
        std::string headers[] = { "", "   LATITUDE", "  LONGITUDE", " DISTANCE", "   LAT SPEED", "   LON SPEED", " DIST SPEED" };
            
        ImGui::Columns(7, "data-table##objects", false);
        ImGui::SetColumnWidth(0, 2.0f*symSize+30.0f);

        // column headers
        for(int i = 0; i < 7; i++)
          { ImGui::Text(headers[i].c_str()); ImGui::NextColumn(); }
        ImGui::Separator();
      
        ImGuiIO& io = ImGui::GetIO();
        astro::Ephemeris &swe = chart->swe();
            
        for(int o = OBJ_SUN; o < OBJ_COUNT; o++)
          {
            astro::ObjData obj = swe.getObjData((astro::ObjType)o);
            double angle = swe.getObjAngle((astro::ObjType)o);
            std::string name = getObjName((astro::ObjType)o);
                
            ChartImage *img = getWhiteImage(name);
            Vec4f color = getObjColor(name);
            ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

            bool checked = chart->getObject((ObjType)o)->visible;
            ImGui::Checkbox(("##show-"+name).c_str(), &checked);
            chart->showObject((ObjType)o, checked);

            ImGui::SetNextItemWidth(symSize+10.0f);
            ImGui::SameLine(); ImGui::Image((ImTextureID)img->texId, ImVec2(symSize, symSize), ImVec2(0,0), ImVec2(1,1), tintCol, ImVec4(0,0,0,0));
            bool hover = ImGui::IsItemHovered();
            if(hover) { ImGui::SetTooltip((name + " - " + angle_string(angle)).c_str()); }
                
            if(hover && io.KeyShift) // focus on this object when SHIFT+hover
              { chart->showObject((ObjType)o, true); chart->setObjFocus((ObjType)o, hover); }
            else
              { chart->setObjFocus((ObjType)o, false); }
            ImGui::NextColumn();
                
            ImGui::Text(angle_string(obj.latitude).c_str());       ImGui::NextColumn();
            ImGui::Text(angle_string(obj.longitude).c_str());      ImGui::NextColumn();
            ImGui::Text((to_string(obj.distance)+" AU").c_str());  ImGui::NextColumn();
            ImGui::Text(angle_string(obj.latSpeed).c_str());       ImGui::NextColumn();
            ImGui::Text(angle_string(obj.lonSpeed).c_str());       ImGui::NextColumn();
            ImGui::Text((to_string(obj.distSpeed)+" AU").c_str()); ImGui::NextColumn();
            if(o == OBJ_PLUTO) // separator after planets
              { ImGui::Separator(); }
          }
        ImGui::Columns(1);
      }
    else
      { mObjOpen = false; }
    
    // if(!chart)
    //   {
    //     ImGui::PopStyleVar();
    //     ImGui::PopItemFlag();
    //   }
  
  }
  ImGui::EndGroup();
  
  return true;
}

