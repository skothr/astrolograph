#include "viewSettings.hpp"
using namespace astro;

#include <iostream>

#include "imgui.h"
#include "glfwKeys.hpp"
#include "settingForm.hpp"

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

  mForm = new SettingForm();
  mForm->add(new SettingGroup("Graph", "graph",
                              { new Setting<Vec4f>("Background Color", "gBgCol",   &graphBgColor,     DEFAULT_GRAPH_BG_COLOR),
                                new Setting<Vec4f>("Line Color",       "gLnCol",   &graphLineColor,   DEFAULT_GRAPH_LINE_COLOR),
                                new Setting<Vec4f>("Axes Color",       "gAxCol",   &graphAxesColor,   DEFAULT_GRAPH_AXES_COLOR),
                                new Setting<bool> ("Draw Lines",       "gDrawLn",  &drawGraphLines,   DEFAULT_GRAPH_DRAW_LINES),
                                new Setting<bool> ("Draw Axes",        "gDrawAx",  &drawGraphAxes,    DEFAULT_GRAPH_DRAW_AXES),
                                new Setting<Vec2f>("Line Spacing",     "gLnSpace", &graphLineSpacing, DEFAULT_GRAPH_LINE_SPACING),
                                new Setting<float>("Line Width",       "gLnWidth", &graphLineWidth,   DEFAULT_GRAPH_LINE_WIDTH) },
                              true));
  mForm->add(new SettingGroup("Nodes", "node",
                              { new Setting<Vec4f>("Background Color", "nBgCol",   &nodeBgColor, DEFAULT_NODE_BG_COLOR) },
                              true));
}
ViewSettings::~ViewSettings()
{ }

void ViewSettings::toggleWindow()
{
  std::cout << "TOGGLING VIEW SETTINGS!\n";
  mState = !mState;
}

bool ViewSettings::checkExitPopup(bool busy, bool hover)
{
  //return (!busy && (ImGui::IsKeyPressed(GLFW_KEY_ESCAPE) || (!hover && ImGui::IsMouseClicked(ImGuiMouseButton_Left))));
  return ((!busy && ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)) || (!hover && ImGui::IsMouseClicked(ImGuiMouseButton_Left)));
}


void ViewSettings::reset()
{
  // Node Graph
  graphBgColor     = DEFAULT_GRAPH_BG_COLOR;
  drawGraphLines   = DEFAULT_GRAPH_DRAW_LINES;
  drawGraphAxes    = DEFAULT_GRAPH_DRAW_AXES;
  graphLineColor   = DEFAULT_GRAPH_LINE_COLOR;
  graphAxesColor   = DEFAULT_GRAPH_AXES_COLOR;
  glSpacingEqual   = DEFAULT_GL_SPACING_EQUAL;
  graphLineSpacing = DEFAULT_GRAPH_LINE_SPACING;
  graphLineWidth   = DEFAULT_GRAPH_LINE_WIDTH;    
  // Nodes
  nodeBgColor      = DEFAULT_NODE_BG_COLOR;
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
      //hover |= ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow | ImGuiHoveredFlags_ChildWindows);
      
      // // center title
      ImGui::SameLine((ImGui::GetWindowContentRegionWidth() - ImGui::CalcTextSize("ViewSettings").x)/2.0f);
      ImGui::Text("View Settings");
      
      bool hover = ImGui::IsWindowHovered();
      
      bool busy = false; // whether view should check for close (if true, another popup is open)
      ImGui::BeginChild("", mWindowSize - mWindowPadding, true);
      {
        hover |= ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);
        busy  |= mForm->draw(1.0f, busy);
      }
      ImGui::EndChild();

      if(busy) { mEscapeDebounce = true; }
      
      ImGui::Spacing();
      ImGui::Separator();
      ImGui::Spacing();
      if(ImGui::Button("Close")) { mState = false; }
      ImGui::SameLine();
      if(ImGui::Button("Reset")) { reset(); }

      if(!busy) // darken background
        { style.Colors[ImGuiCol_ModalWindowDimBg] = Vec4f(0,0,0, 0.4f); }
      else // un-darken screen to see results
        { style.Colors[ImGuiCol_ModalWindowDimBg] = Vec4f(0,0,0, 0.0f); }
      
      if(checkExitPopup(busy, hover)) // close view settings when escape pressed
        {
          if(mEscapeDebounce)      // color picker just closed -- debounce escape press
            { mEscapeDebounce = false; }
          else { mState = false; } // close view settings
        }
      ImGui::EndPopup();
    }
  else
    { ImGui::PopStyleVar(); } // ImGuiStyleVar_WindowMinSize
  ImGui::PopStyleVar();       // ImGuiStyleVar_WindowRounding
  
  return mState;
}
