#include "viewSettings.hpp"
using namespace astro;

#include <iostream>

#include "imgui.h"
#include "glfwKeys.hpp"


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
        static Vec4f lastColor; // save previous color in case user cancels
        // choose graph background color
        ImGui::TextUnformatted("Graph Background Color");
        ImGui::SameLine();
        if(ImGui::ColorButton("##current", graphBgColor, ImGuiColorEditFlags_NoOptions|ImGuiColorEditFlags_DisplayRGB|ImGuiColorEditFlags_NoAlpha, ImVec2(20, 20)))
          {
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0,0,0,0);
            lastColor = graphBgColor;
            ImGui::OpenPopup("bgPopup");
          }
        if(ImGui::BeginPopup("bgPopup", wFlags))
          {
            busy = true;
            if(!ImGui::ColorPicker4("bgPicker", graphBgColor.data.data(), ImGuiColorEditFlags_NoSidePreview))
              {
                if(checkExitPopup(ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow|ImGuiHoveredFlags_ChildWindows)))
                  {
                    graphBgColor = lastColor;
                    ImGui::CloseCurrentPopup();
                  }
              }
            if(ImGui::Button("Select") || ImGui::IsKeyPressed(GLFW_KEY_ENTER)) // selects color
              { ImGui::CloseCurrentPopup(); }
            ImGui::SameLine();
            if(ImGui::Button("Cancel"))
              {
                graphBgColor = lastColor;
                ImGui::CloseCurrentPopup();
              }
            ImGui::EndPopup();
          }
      }
      ImGui::EndGroup();

      if(ImGui::Button("Close")) { mState = false; }

      if(!busy)
        { // check for close
          style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0,0,0,0.6);
          if(checkExitPopup(hover)) { mState = false; }
        }
      ImGui::EndPopup();
    }
  
  return mState;
}
