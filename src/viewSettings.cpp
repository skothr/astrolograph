#include "viewSettings.hpp"
using namespace astro;

#include <iostream>

#include "imgui.h"
#include "glfwKeys.hpp"


ViewSettings::ViewSettings()
{
  ImFontConfig config;
  config.OversampleH = 4;
  config.OversampleV = 4;
  mainFont = ImGui::GetIO().Fonts->AddFontFromFileTTF(FONT_PATH, MAIN_FONT_HEIGHT, &config);
  titleFont = ImGui::GetIO().Fonts->AddFontFromFileTTF(FONT_PATH, TITLE_FONT_HEIGHT, &config);

  // set modal dim overlay color
  ImGuiStyle& style = ImGui::GetStyle();
  style.Colors[ImGuiCol_ModalWindowDimBg] = Vec4f(0,0,0,0.6);
}
ViewSettings::~ViewSettings()
{ }

void ViewSettings::openWindow()
{
  mState = true;
}

void ViewSettings::closeWindow()
{
  mState = false;
}

void ViewSettings::toggleWindow()
{
  std::cout << "TOGGLING VIEW SETTINGS!\n";
  mState = !mState;
}

bool ViewSettings::checkExitPopup(bool hover)
{
  if(ImGui::IsKeyPressed(GLFW_KEY_ESCAPE) || (!hover && ImGui::IsMouseClicked(ImGuiMouseButton_Left)))
    { return true; }
  else
    { return false; }
}

bool ViewSettings::colorSetting(const std::string &name, const std::string &id, Vec4f *color, bool busy)
{
  ImGuiWindowFlags wFlags = (ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoMove           |
                             ImGuiWindowFlags_NoTitleBar       |
                             ImGuiWindowFlags_NoResize );
  ImGuiStyle& style = ImGui::GetStyle();
  static Vec4f lastColor; // save previous color in case user cancels
  std::string popupName = id + "popup";
  std::string pickerName = id + "picker";
  
  // choose graph background color
  ImGui::TextUnformatted(name.c_str());
  ImGui::SameLine(mLabelColWidth);
  if(ImGui::ColorButton((std::string("##")+id).c_str(), *color,
                        ImGuiColorEditFlags_NoOptions|ImGuiColorEditFlags_DisplayRGB|ImGuiColorEditFlags_NoAlpha, ImVec2(20, 20)) && !busy)
    {
      style.Colors[ImGuiCol_ModalWindowDimBg] = Vec4f(0,0,0,0);
      lastColor = *color;
      ImGui::OpenPopup(popupName.c_str());
    }
  if(ImGui::BeginPopup(popupName.c_str(), wFlags))
    {
      busy = true;
      if(!ImGui::ColorPicker4(pickerName.c_str(), color->data.data(), ImGuiColorEditFlags_NoSidePreview))
        {
          if(checkExitPopup(ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow|ImGuiHoveredFlags_ChildWindows)))
            {
              *color = lastColor;
              ImGui::CloseCurrentPopup();
            }
        }
      if(ImGui::Button("Select") || ImGui::IsKeyPressed(GLFW_KEY_ENTER)) // selects color
        { ImGui::CloseCurrentPopup(); }
      ImGui::SameLine();
      if(ImGui::Button("Cancel"))
        {
          *color = lastColor;
          ImGui::CloseCurrentPopup();
        }
      ImGui::EndPopup();
    }
  return busy;
}

bool ViewSettings::draw()
{
  ImGuiWindowFlags wFlags = (ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoMove           |
                             ImGuiWindowFlags_NoTitleBar       |
                             ImGuiWindowFlags_NoResize );
  
  if(mState)
    { ImGui::OpenPopup("viewSettings"); }
  
  if(ImGui::BeginPopupModal("viewSettings", &mState, wFlags))
    {
      ImGui::SetWindowSize(Vec2f(512,512));
      ImGuiIO &io = ImGui::GetIO();
      ImGuiStyle& style = ImGui::GetStyle();
      bool hover = ImGui::IsWindowHovered();

      // center title
      ImGui::SameLine((ImGui::GetWindowContentRegionWidth() - ImGui::CalcTextSize("ViewSettings").x)/2.0f);
      ImGui::Text("View Settings");
      
      bool busy = false; // whether view should check for close (if true, another popup is open)

      ImGui::BeginGroup();
      {
        ImGui::TextUnformatted("Node Graph");

        ImGui::TextUnformatted("Draw Graph Lines");
        ImGui::SameLine(mLabelColWidth);
        ImGui::Checkbox("##glDrawLn", &drawGraphLines);
        ImGui::TextUnformatted("Draw Axes");
        ImGui::SameLine(mLabelColWidth);
        ImGui::Checkbox("##glDrawAx", &drawGraphAxes);
        
        ImGui::TextUnformatted("Graph Line Spacing");
        ImGui::SameLine(mLabelColWidth);
        ImGui::BeginGroup();
        {
          ImGui::TextUnformatted("X:");
          ImGui::SameLine();
          ImGui::SetNextItemWidth(100);
          ImGui::InputFloat("##glSpacingX", &graphLineSpacing.x);
          ImGui::SameLine();
          ImGui::TextUnformatted("Y:");
          ImGui::SameLine();
          ImGui::SetNextItemWidth(100);
          ImGui::InputFloat("##glSpacingY", &graphLineSpacing.y);
        }
        ImGui::EndGroup();
        
        busy |= colorSetting("Graph Background Color", "ngBgCol", &graphBgColor,   busy);
        busy |= colorSetting("Graph Line Color",       "ngLnCol", &graphLineColor, busy);
        busy |= colorSetting("Graph Axes Color",       "ngAxCol", &graphAxesColor, busy);
        ImGui::Separator();
        ImGui::TextUnformatted("Nodes");
        busy |= colorSetting("Node Background Color",  "nBgCol",  &nodeBgColor,    busy);
      }
      ImGui::EndGroup();

      if(ImGui::Button("Close")) { mState = false; }

      if(!busy)
        { // check for close
          style.Colors[ImGuiCol_ModalWindowDimBg] = Vec4f(0,0,0,0.6);
          if(checkExitPopup(hover)) { mState = false; }
        }
      ImGui::EndPopup();
    }
  
  return mState;
}
