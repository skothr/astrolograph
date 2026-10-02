#ifndef VIEW_SETTINGS_HPP
#define VIEW_SETTINGS_HPP

#include "nlohmann/json_fwd.hpp" // json forward declarations
using json = nlohmann::json;

#include "vector.hpp"
#include <string>
#include <map>

#define MAIN_FONT_HEIGHT  16.0f
#define TITLE_FONT_HEIGHT 24.0f
#define FONT_OVERSAMPLE   8

#define FONT_PATH "./res/fonts/"
#define FONT_NAME "UbuntuMono"
#define FONT_PATH_REGULAR     (FONT_PATH FONT_NAME "-R.ttf" )
#define FONT_PATH_ITALIC      (FONT_PATH FONT_NAME "-RI.ttf")
#define FONT_PATH_BOLD        (FONT_PATH FONT_NAME "-B.ttf" )
#define FONT_PATH_BOLD_ITALIC (FONT_PATH FONT_NAME "-BI.ttf")

// graph setting defaults
#define DEFAULT_GRAPH_BG_COLOR     Vec4f(0.05f, 0.05f, 0.05f, 1.0f)
#define DEFAULT_GRAPH_LINE_COLOR   Vec4f(0.15f, 0.15f, 0.15f, 1.0f)
#define DEFAULT_GRAPH_AXES_COLOR   Vec4f(0.50f, 0.50f, 0.50f, 1.0f)
#define DEFAULT_GRAPH_DRAW_LINES   true
#define DEFAULT_GRAPH_DRAW_AXES    true
#define DEFAULT_GL_SPACING_EQUAL   true
#define DEFAULT_GRAPH_LINE_SPACING Vec2f(64.0f, 64.0f)
#define DEFAULT_GRAPH_LINE_WIDTH   1.0f
// node setting defaults
#define DEFAULT_NODE_BG_COLOR      Vec4f(0.20f, 0.20f, 0.20f,  1.0f)


// TODO: Charts
// --> aspect colors
// --> extended object list


// forward declarations
struct ImFont;
struct ImFontConfig;

// forward declarations
class SettingForm;
  
// global view settings -- controls interface to settings window (Menu: View->Settings)
class ViewSettings
{
private:
  bool mInitialized    = false;
  SettingForm *mForm   = nullptr;
  float mLabelColWidth = 256.0f;
    
public:
  ~ViewSettings();
    
  //// Global ////
  // fonts
  ImFontConfig *fontConfig = nullptr;
  // ImFontConfig mainFontConfig;
  // ImFontConfig titleFontConfig;
  float mainTextSize  = 16.0f;
  float titleTextSize = 20.0f;
  ImFont *mainFont    = nullptr; // NOTE: Fonts deleted by ImGui
  ImFont *mainFontB   = nullptr;
  ImFont *mainFontI   = nullptr;
  ImFont *mainFontBI  = nullptr;
  ImFont *titleFont   = nullptr;
  ImFont *titleFontB  = nullptr;
  ImFont *titleFontI  = nullptr;
  ImFont *titleFontBI = nullptr;
    
  // std::map<float, ImFont*> mCustomFonts; // for loading fonts at custom sizes -- scaling/etc
  // ImFont* getFont(float fontSize);
    
    
  //// Node Graph ////
  Vec4f graphBgColor     = DEFAULT_GRAPH_BG_COLOR;
  Vec4f graphLineColor   = DEFAULT_GRAPH_LINE_COLOR;
  Vec4f graphAxesColor   = DEFAULT_GRAPH_AXES_COLOR;
  bool  drawGraphLines   = DEFAULT_GRAPH_DRAW_LINES;
  bool  drawGraphAxes    = DEFAULT_GRAPH_DRAW_AXES;
  bool  glSpacingEqual   = DEFAULT_GL_SPACING_EQUAL;
  Vec2f graphLineSpacing = DEFAULT_GRAPH_LINE_SPACING;
  float graphLineWidth   = DEFAULT_GRAPH_LINE_WIDTH;
  //// Nodes ////
  Vec4f nodeBgColor      = DEFAULT_NODE_BG_COLOR;
    
  json toJSON();
  bool fromJSON(json js);
    
  void init();
  void reset();
  SettingForm* form() { return mForm; }
};

#endif // VIEW_SETTINGS_HPP
