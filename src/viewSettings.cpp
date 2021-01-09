#include "viewSettings.hpp"
using namespace astro;

#include <iostream>
#include <fstream>

#include "nlohmann/json.hpp"
#include "imgui.h"
#include "glfwKeys.hpp"
#include "astro.hpp"
#include "setting.hpp"
#include "settingForm.hpp"

json ViewSettings::toJSON()          { return (mForm ? mForm->toJSON()     : json::object()); }
bool ViewSettings::fromJSON(json js) { return (mForm ? mForm->fromJSON(js) : false);          }

void ViewSettings::init()
{
  if(!mInitialized)
    {
      ImGuiIO &io = ImGui::GetIO();
      ImFontConfig config;
      config.OversampleH = FONT_OVERSAMPLE;
      config.OversampleV = FONT_OVERSAMPLE;
      ImVector<ImWchar> ranges;
      ImFontGlyphRangesBuilder builder;
      builder.AddText(NAKSHATRA_EXTRA_CHARS);
      builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
      builder.BuildRanges(&ranges);
      config.GlyphRanges = ranges.Data;
  
      config.SizePixels  = MAIN_FONT_HEIGHT+1.0f; // NOTE: font rendering sometimes clips glyphs (?) TODO: FIX
      config.GlyphOffset = Vec2f(0, -1.5f);       
      mainFont    = io.Fonts->AddFontFromFileTTF(FONT_PATH_REGULAR,     MAIN_FONT_HEIGHT,  &config);
      mainFontB   = io.Fonts->AddFontFromFileTTF(FONT_PATH_BOLD,        MAIN_FONT_HEIGHT,  &config);
      mainFontI   = io.Fonts->AddFontFromFileTTF(FONT_PATH_ITALIC,      MAIN_FONT_HEIGHT,  &config);
      mainFontBI  = io.Fonts->AddFontFromFileTTF(FONT_PATH_BOLD_ITALIC, MAIN_FONT_HEIGHT,  &config);
      config.SizePixels  = TITLE_FONT_HEIGHT+1.0f;
      titleFont   = io.Fonts->AddFontFromFileTTF(FONT_PATH_REGULAR,     TITLE_FONT_HEIGHT, &config);
      titleFontB  = io.Fonts->AddFontFromFileTTF(FONT_PATH_BOLD,        TITLE_FONT_HEIGHT, &config);
      titleFontI  = io.Fonts->AddFontFromFileTTF(FONT_PATH_ITALIC,      TITLE_FONT_HEIGHT, &config);
      titleFontBI = io.Fonts->AddFontFromFileTTF(FONT_PATH_BOLD_ITALIC, TITLE_FONT_HEIGHT, &config);
      io.Fonts->Build();

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
      mInitialized = true;
    }
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
