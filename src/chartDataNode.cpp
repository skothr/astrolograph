#include "chartDataNode.hpp"
using namespace astro;

#include "imgui.h"
#include "imgui_internal.h" // (for Push/Pop ItemFlag)
#include "tools.hpp"
#include "chartView.hpp"


ChartDataNode::ChartDataNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Chart Data Node"),
    mShowObjects(OBJ_COUNT + ANGLE_END-ANGLE_OFFSET, false),
    mFocusObjects(OBJ_COUNT + ANGLE_END-ANGLE_OFFSET, false)
{
  setMinSize(Vec2f(880, 0));
  for(int o = OBJ_SUN; o < OBJ_COUNT; o++) // start with objects visible
    { mShowObjects[o] = true; }
}

ChartDataNode::~ChartDataNode()
{ }

void ChartDataNode::onUpdate()
{ }

void ChartDataNode::onDraw()
{
  float scale = getScale();
  float symSize = 20*scale;
      
  bool changed = false;
  Chart *chart = inputs()[DATANODE_INPUT_CHART]->get<Chart>();

  ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding);

  // Angle Data
  ImGui::SetNextTreeNodeOpen(mAngOpen);
  if(ImGui::CollapsingHeader("Angle Data", nullptr, flags) && chart)
    {
      mAngOpen = true;
      std::string headers[] = { "", "   LONGITUDE", "   LON SPEED" };
      
      float maxW = 0.0f;
      for(int i = 0; i < 3; i++)
        { maxW = std::max(maxW, ImGui::CalcTextSize(headers[i].c_str()).x); }
      
      ImGui::Columns(3, "data-table##angles", false);
      ImGui::SetColumnWidth(0, maxW);//2.0f*symSize+30.0f*scale);
        
      // column headers
      for(int i = 0; i < 3; i++)
        {
          ImGui::TextUnformatted(headers[i].c_str()); ImGui::NextColumn();
        }
      ImGui::Separator();
        
      ImGuiIO& io = ImGui::GetIO();
      astro::Ephemeris &swe = chart->swe();
      
      for(int a = ANGLE_OFFSET; a < ANGLE_END; a++)
        {
          ObjData objData = chart->getObjectData((ObjType)a);
          double angle = objData.longitude; //chart->getObject((ObjType)a)->angle;
          std::string name = getObjName((astro::ObjType)a);
                
          ChartImage *img = getWhiteImage(name);
          Vec4f color = getObjColor(name);
          ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

          mShowObjects[OBJ_COUNT+a-ANGLE_OFFSET] = chart->getObject((ObjType)a)->visible;

          ImGui::BeginGroup();
          {
            ImGui::SetNextItemWidth(symSize+10.0f*scale);
            ImGui::Image(img->id(), ImVec2(symSize, symSize), ImVec2(0,0), ImVec2(1,1), tintCol, ImVec4(0,0,0,0));
            ImGui::SameLine(); ImGui::Text("%s", name.c_str());
          }
          ImGui::EndGroup();
          bool hover = ImGui::IsItemHovered();
          if(hover) { ImGui::SetTooltip("%s - %s", name.c_str(), angle_string(angle).c_str()); }
            
          // set focus
          bool focused = (hover && io.KeyShift); // focus on this object with SHIFT+hover
          if(mFocusObjects[a] == chart->objects()[a]->focused)
            {
              mFocusObjects[a] = focused;
              chart->setObjFocus((ObjType)a, mFocusObjects[a]);
            }
          ImGui::NextColumn();
          ImGui::TextUnformatted(angle_string(objData.longitude).c_str()); ImGui::NextColumn();
          ImGui::TextUnformatted(angle_string(objData.lonSpeed).c_str()); ImGui::NextColumn();
        }
      ImGui::Columns(1);
    }
  else if(chart && isBodyVisible())
    { mAngOpen = false; }
    
  // Object Data
  ImGui::SetNextTreeNodeOpen(mObjOpen);
  if(ImGui::CollapsingHeader("Object Data", nullptr, flags) && chart)
    {
      mObjOpen = true;
      std::string headers[] = { "", "   LATITUDE", "  LONGITUDE", " DISTANCE", "   LAT SPEED", "   LON SPEED", " DIST SPEED" };
            
      ImGui::Columns(7, "data-table##objects", false);
      ImGui::SetColumnWidth(0, 2.0f*symSize+60.0f*scale);

      // column headers
      for(int i = 0; i < 7; i++)
        { ImGui::TextUnformatted(headers[i].c_str()); ImGui::NextColumn(); }
      ImGui::Separator();
      
      ImGuiIO& io = ImGui::GetIO();
      astro::Ephemeris &swe = chart->swe();
            
      for(int o = OBJ_SUN; o < OBJ_COUNT; o++)
        {
          astro::ObjData obj = swe.getObjData((astro::ObjType)o);
          double angle = obj.longitude;//swe.getObjAngle((astro::ObjType)o);
          std::string name = getObjName((astro::ObjType)o);
          std::string nameLong = getObjNameLong((astro::ObjType)o);
                
          ChartImage *img = getWhiteImage(name);
          Vec4f color = getObjColor(name);
          ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

          mShowObjects[o] = chart->getObject((ObjType)o)->visible;

          ImGui::BeginGroup();
          {
            ImGui::SetNextItemWidth(symSize+10.0f*scale);
            ImGui::Image(img->id(), ImVec2(symSize, symSize), ImVec2(0,0), ImVec2(1,1), tintCol, ImVec4(0,0,0,0));
            ImGui::SameLine(); ImGui::Text("%s", nameLong.c_str());
          }
          ImGui::EndGroup();
          
          bool hover = ImGui::IsItemHovered();
          if(hover) { ImGui::SetTooltip("%s - %s", name.c_str(), angle_string(angle).c_str()); }
            
          // set focus
          bool focused = (hover && io.KeyShift); // focus on this object with SHIFT+hover
          if(mFocusObjects[o] == chart->objects()[o]->focused)
            {
              mFocusObjects[o] = focused;
              chart->setObjFocus((ObjType)o, mFocusObjects[o]);
            }
          
          ImGui::NextColumn();
          ImGui::TextUnformatted(angle_string(obj.latitude).c_str());       ImGui::NextColumn();
          ImGui::TextUnformatted(angle_string(obj.longitude).c_str());      ImGui::NextColumn();
          ImGui::TextUnformatted((to_string(obj.distance)+" AU").c_str());  ImGui::NextColumn();
          ImGui::TextUnformatted(angle_string(obj.latSpeed).c_str());       ImGui::NextColumn();
          ImGui::TextUnformatted(angle_string(obj.lonSpeed).c_str());       ImGui::NextColumn();
          ImGui::TextUnformatted((to_string(obj.distSpeed)+" AU").c_str()); ImGui::NextColumn();
          if(o == OBJ_PLUTO) { ImGui::Separator(); } // separator after planets
        }
      ImGui::Columns(1);
    }
  else if(chart && isBodyVisible())
    { mObjOpen = false; }
}

