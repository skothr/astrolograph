#include "aspectNode.hpp"
using namespace astro;

#include "imgui.h"
#include "imgui_internal.h" // for scaling + window scrolling
#include "setting.hpp"
#include "tools.hpp"
#include "chartView.hpp"

#include "viewSettings.hpp"

AspectNode::AspectNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Aspect Node"), mParams(new ChartParams())
{
  setMinSize(Vec2f(384, 0));
  for(int a = 0; a < ASPECT_COUNT; a++)
    {
      mAspVisible.push_back(true); // start with all aspects visible
      AspectInfo *info = getAspectInfo(a);
      mAspOrbs.push_back(info->orb); // default orbs
    }

  mSettings.push_back(new Setting<bool> ("List Open", "listOpen",   &mListOpen));
  mSettings.push_back(new Setting<bool> ("Orbs Open", "orbsOpen",   &mOrbsOpen));
  mSettings.push_back(makeSettingGroup<BoolStruct> ("Visible Aspects", "aspVisible", &mAspVisible));
  mSettings.push_back(makeSettingGroup<double>     ("Aspect Orbs",     "aspOrbs",    &mAspOrbs));
}

void AspectNode::onUpdate()
{
  // mLastScale = getScale();
}

void AspectNode::onDraw()
{
  float scale = getScale();
  Vec2f symSize = Vec2f(20, 20)*scale;
  float borderW = 1.0f;
  Vec2f childSize = Vec2f(384, 512);
  
  bool changed = false;
  Chart *chart = inputs()[ASPECTNODE_INPUT_CHART]->get<Chart>();
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_Framed;

  if(chart && (chart->hasChanged() || mAspects.size() == 0 || *mParams != *chart->getParams()))
    {
      *mParams = *chart->getParams();
      mAspects = chart->calcAspects(*mParams, true);
    }
  else
    { mAspects.clear(); }
  
  // aspect list
  if(chart)
    {
      mListOpen = true;
      ChartParams *params = chart->getParams();
      
      // update aspect visiblity and count visible aspects
      int visibleCount = 0;
      for(int i = 0; i < mAspects.size(); i++)
        {
          ChartAspect &asp = mAspects[i];
          if(asp.type < 0 || asp.type >= ASPECT_COUNT) { continue; }
          if(!(asp.obj1 && asp.obj2 && asp.obj1->type > 0 && asp.obj1->type < OBJ_END && asp.obj2->type > 0 && asp.obj2->type < OBJ_END)) { continue; }
          // mAspVisible[asp.type] = chart->getAspectVisible(asp.type);
          if(mAspVisible[asp.type] && params->aspVisible[asp.type] &&
             params->objVisible[asp.obj1->type] && params->objVisible[asp.obj2->type])
            { visibleCount++; }
        }
      
      // sort aspects by orb (reverse?)
      std::sort(mAspects.begin(), mAspects.end(),
                [](const ChartAspect &a, const ChartAspect &b) -> bool
                {
                  if(std::abs(a.orb - b.orb) < 0.001)
                    { // differentiate by aspect type, then object types
                      if(a.type < b.type)      { return true;  }
                      else if(a.obj1 < b.obj1) { return true;  }
                  else if(a.obj2 < b.obj2)     { return true;  }
                  else                         { return false; }
                }
              else // return smaller orb
                { return (a.orb < b.orb); }
            } ); // sort by orb (ascending)

      ImGui::Text("Total count: %d (visible: %d)", (int)mAspects.size(), visibleCount);
      ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 20*scale);
      ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 2.0f*scale);
      ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, borderW*scale);
      ImGui::PushStyleColor(ImGuiCol_Border, Vec4f(0,0,0,1));
      ImGui::GetStyle().ScrollbarScaling = scale;
      ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos().x+borderW/2.0f*scale, ImGui::GetCursorPos().y));
      ImGui::BeginChild("##listChild", childSize*scale, true, 0);
      ImGui::GetStyle().ScrollbarScaling = 1.0f;
      ImGui::SetNextWindowScroll(Vec2f(0, mListScroll*mLastScale/scale));
      ImGui::PopStyleColor();
      ImGui::PopStyleVar(3);
      {
        ImGui::SetWindowFontScale(scale);
        ImGuiWindow *window = ImGui::GetCurrentWindow();
        mListScroll = window->Scroll.y;
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        for(int i = 0; i < mAspects.size(); i++)
          {
            ChartAspect asp = mAspects[i];
            if(asp.type < 0 || asp.type >= ASPECT_COUNT) { continue; }
            if(!(asp.obj1 && asp.obj2 && asp.obj1->type > 0 && asp.obj1->type < OBJ_END && asp.obj2->type > 0 && asp.obj2->type < OBJ_END)) { continue; }

            // std::cout << asp.type << " / " << asp.obj1->type << " / " << asp.obj2->type << "\n";
            
            // skip north/south node opposition, and non-visible aspects
            if(!(asp.obj1 && asp.obj2) || (asp.obj1->type == OBJ_NORTHNODE && asp.obj2->type == OBJ_SOUTHNODE) ||
               asp.obj1->type < 0 || asp.obj1->type >= OBJ_END || asp.obj2->type < 0 || asp.obj2->type >= OBJ_END ||
               (asp.obj2->type == OBJ_NORTHNODE && asp.obj1->type == OBJ_SOUTHNODE) ||
               asp.type < 0 || asp.type >= mAspVisible.size() || asp.type >= params->aspVisible.size() ||
               !mAspVisible[asp.type] || !params->aspVisible[asp.type] || !asp.visible ||
               !params->objVisible[asp.obj1->type] || !params->objVisible[asp.obj2->type]) { continue; }
            
            std::string aName = getAspectName(asp.type);
            std::string o1Name = getObjName(asp.obj1->type);
            std::string o2Name = getObjName(asp.obj2->type);
            Vec4f aColor = getAspectInfo(asp.type)->color;
            Vec4f scaledColor = aColor;
            scaledColor.w *= asp.strength;//*asp.strength; // weight surrounding hexagon color by aspect strength
            Vec4f o1Color = getObjColor(o1Name);
            Vec4f o2Color = getObjColor(o2Name);

            ImGui::Spacing();
            ImGui::Text("%s", angle_string(asp.orb, true).c_str());
            {
              Vec2f centerOffset = Vec2f(164*scale, 0)-symSize*0.7f;
              ImGui::SameLine(); ImGui::Image(getWhiteImage(o1Name)->id(), symSize, Vec2f(0,0), Vec2f(1,1), o1Color, Vec4f(0,0,0,0));
              ImGui::SameLine(); Vec2f cpos = ImGui::GetCursorScreenPos();
              ImGui::Image(getImage(aName)->id(),       symSize, Vec2f(0,0), Vec2f(1,1), aColor,  Vec4f(0,0,0,0));
              draw_list->AddCircle(Vec2f(cpos)+symSize/2.0f, symSize.x*0.7f, ImColor(scaledColor), 6, 1);
              ImGui::SameLine(); ImGui::Image(getWhiteImage(o2Name)->id(), symSize, Vec2f(0,0), Vec2f(1,1), o2Color, Vec4f(0,0,0,0));
            }
            ImGui::SameLine(); ImGui::TextUnformatted(aName.c_str());
            ImGui::Spacing();
          }
      }
      ImGui::EndChild();
    }
  mLastScale = scale;
  ImGui::SetCursorPos(Vec2f((ImGui::GetCursorPos().x+childSize.x + borderW)*scale, ImGui::GetCursorPos().y));
}

