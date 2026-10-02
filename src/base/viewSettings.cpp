#include "viewSettings.hpp"

#include <iostream>
#include <fstream>

#include "nlohmann/json.hpp"
#include "imgui.h"
#include "glfwKeys.hpp"
#include "setting.hpp"
#include "settingForm.hpp"

#define OPERATORS_EXTRA_CHARS "∇"
// #define ALCHEMIC_CHAR_RANGE u8"\u1F700" u8"\u1F77F"    //u8"🜀", u8"🝳",
#define ALCHEMIC_CHAR_RANGE u8"🜀", u8"🝳"


// static const ImWchar ranges[] =
//static const std::vector<ImWchar> ranges;
// static const ImWchar *ranges = (ImWchar*)(ALCHEMIC_CHAR_RANGE u8"\0");

ViewSettings::~ViewSettings()
{
  if(fontConfig) { delete fontConfig; }
}

json ViewSettings::toJSON()          { return (mForm ? mForm->toJSON()     : json::object()); }
bool ViewSettings::fromJSON(json js) { return (mForm ? mForm->fromJSON(js) : false);          }

void ViewSettings::init()
{
  if(!mInitialized)
    {
      ImGuiIO &io = ImGui::GetIO();
      fontConfig = new ImFontConfig();
      fontConfig->OversampleH = FONT_OVERSAMPLE;
      fontConfig->OversampleV = FONT_OVERSAMPLE;
      // ImVector<ImWchar> ranges;
      // ImFontGlyphRangesBuilder builder;
      // builder.AddText(NAKSHATRA_EXTRA_CHARS);
      // builder.AddText(PLANETS_EXTRA_CHARS); // TODO: No symbol chars?
      // builder.AddText(PLANETS_ALTERNATIVE);
      // builder.AddText(ASPECTS_EXTRA_CHARS);
      // builder.AddText(ELEMENTS_EXTRA_CHARS);
      // builder.AddText(OPERATORS_EXTRA_CHARS);
      // builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
      // builder.BuildRanges(&ranges);
      // fontConfig->GlyphRanges = ranges.Data;
      
      ImVector<ImWchar> ranges;
      ImFontGlyphRangesBuilder builder;
      builder.AddText("∇");
      builder.AddRanges(io.Fonts->GetGlyphRangesDefault());  // Add one of the default ranges
      builder.AddChar(0x2207);                                // Add a specific character
      builder.BuildRanges(&ranges);                          // Build the final result (ordered ranges with all the unique characters submitted)

      // io.Fonts->AddFontFromFileTTF("myfontfile.ttf", size_in_pixels, NULL, ranges.Data);
      // io.Fonts->Build();                                     // Build the atlas while 'ranges' is still in scope and not deleted.

      
      fontConfig->GlyphRanges = ranges.Data;
      
      fontConfig->SizePixels  = MAIN_FONT_HEIGHT+1.0f; // NOTE: font rendering sometimes clips glyphs (?) TODO: FIX
      fontConfig->GlyphOffset = Vec2f(0, -1.5f);
      mainFont    = io.Fonts->AddFontFromFileTTF(FONT_PATH_REGULAR,     MAIN_FONT_HEIGHT,  fontConfig, ranges.Data);
      mainFontB   = io.Fonts->AddFontFromFileTTF(FONT_PATH_BOLD,        MAIN_FONT_HEIGHT,  fontConfig, ranges.Data);
      mainFontI   = io.Fonts->AddFontFromFileTTF(FONT_PATH_ITALIC,      MAIN_FONT_HEIGHT,  fontConfig, ranges.Data);
      mainFontBI  = io.Fonts->AddFontFromFileTTF(FONT_PATH_BOLD_ITALIC, MAIN_FONT_HEIGHT,  fontConfig, ranges.Data);
      fontConfig->SizePixels  = TITLE_FONT_HEIGHT+1.0f;
      titleFont   = io.Fonts->AddFontFromFileTTF(FONT_PATH_REGULAR,     TITLE_FONT_HEIGHT, fontConfig, ranges.Data);
      titleFontB  = io.Fonts->AddFontFromFileTTF(FONT_PATH_BOLD,        TITLE_FONT_HEIGHT, fontConfig, ranges.Data);
      titleFontI  = io.Fonts->AddFontFromFileTTF(FONT_PATH_ITALIC,      TITLE_FONT_HEIGHT, fontConfig, ranges.Data);
      titleFontBI = io.Fonts->AddFontFromFileTTF(FONT_PATH_BOLD_ITALIC, TITLE_FONT_HEIGHT, fontConfig, ranges.Data);
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


// ImFont* ViewSettings::getSizedFont(float fontSize)
// {
//   //fontConfig->SizePixels = fontSize + 1.0f; // NOTE: font rendering sometimes clips glyphs (?) TODO: FIX
//   ImFont font = io.Fonts->AddFontFromFileTTF(FONT_PATH_REGULAR, fontSize, &fontConfig);
//   return font;
// }
