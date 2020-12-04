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
  style.Colors[ImGuiCol_ModalWindowDimBg] = Vec4f(0.0f, 0.0f, 0.0f, 0.6f);

  mForm.add(new SettingGroup("Graph", "graph",
                             {   new Setting<Vec4f>("Background Color", "gBgCol",   &graphBgColor),
                                 new Setting<Vec4f>("Line Color",       "gLnCol",   &graphLineColor),
                                 new Setting<Vec4f>("Axes Color",       "gAxCol",   &graphAxesColor),
                                 new Setting<bool> ("Draw Lines",       "gDrawLn",  &drawGraphLines),
                                 new Setting<bool> ("Draw Axes",        "gDrawAx",  &drawGraphAxes),
                                 new Setting<Vec2f>("Line Spacing",     "gLnSpace", &graphLineSpacing),
                                 new Setting<float>("Line Width",       "gLnWidth", &graphLineWidth) }));
  mForm.add(new SettingGroup("Nodes", "node",
                             {   new Setting<Vec4f>("Background Color", "nBgCol",   &nodeBgColor) }));
}
ViewSettings::~ViewSettings()
{ }

void ViewSettings::toggleWindow()
{
  std::cout << "TOGGLING VIEW SETTINGS!\n";
  mState = !mState;
}

bool ViewSettings::checkExitPopup(bool hover)
{ return (ImGui::IsKeyPressed(GLFW_KEY_ESCAPE) || (!hover && ImGui::IsMouseClicked(ImGuiMouseButton_Left))); }

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

bool ViewSettings::draw(const Vec2f &frameSize)
{
  ImGuiWindowFlags wFlags = (ImGuiWindowFlags_NoMove           |
                             ImGuiWindowFlags_NoTitleBar       |
                             ImGuiWindowFlags_NoResize );
  
  if(mState) { ImGui::OpenPopup("viewSettings"); }
  
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, mWindowSize);
  ImGui::SetNextWindowPos((frameSize - mWindowSize)/2.0f);
  if(ImGui::BeginPopupModal("viewSettings", &mState, wFlags))
    {
      ImGui::PopStyleVar();
      ImGuiIO &io = ImGui::GetIO();
      ImGuiStyle& style = ImGui::GetStyle();
      bool hover = ImGui::IsWindowHovered();
      
      bool busy = false; // whether view should check for close (if true, another popup is open)

      // // center title
      ImGui::SameLine((ImGui::GetWindowContentRegionWidth() - ImGui::CalcTextSize("ViewSettings").x)/2.0f);
      ImGui::Text("View Settings");
      
      ImGui::BeginChild("", mWindowSize - mWindowPadding, true);
      {
        hover |= ImGui::IsWindowHovered();
        busy  |= mForm.draw(1.0f, busy);
      }
      ImGui::EndChild();
      
      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();
      if(ImGui::Button("Close")) { mState = false; }

      if(!busy)
        { // check for close
          style.Colors[ImGuiCol_ModalWindowDimBg] = Vec4f(0,0,0,0.6);
          if(checkExitPopup(hover)) { mState = false; }
        }
      ImGui::EndPopup();
    }
  else
    { ImGui::PopStyleVar(); } // minSize
  ImGui::PopStyleVar();       // rounding
  
  return mState;
}
