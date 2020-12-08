#ifndef VIEW_SETTINGS_HPP
#define VIEW_SETTINGS_HPP

#include "vector.hpp"

#define FONT_PATH   "./res/fonts/UbuntuMono-R.ttf"
#define MAIN_FONT_HEIGHT 16.0f
#define TITLE_FONT_HEIGHT 20.0f


// graph setting defaults
#define DEFAULT_GRAPH_BG_COLOR     Vec4f(0.05f, 0.05f, 0.05f,  1.0f)
#define DEFAULT_GRAPH_DRAW_LINES   true
#define DEFAULT_GRAPH_DRAW_AXES    true
#define DEFAULT_GRAPH_LINE_COLOR   Vec4f(0.15f, 0.15f, 0.15f,  1.0f)
#define DEFAULT_GRAPH_AXES_COLOR   Vec4f(0.50f, 0.50f, 0.50f,  1.0f)
#define DEFAULT_GL_SPACING_EQUAL   true
#define DEFAULT_GRAPH_LINE_SPACING Vec2f(64.0f, 64.0f)
#define DEFAULT_GRAPH_LINE_WIDTH   1.0f
// node setting defaults
#define DEFAULT_NODE_BG_COLOR      Vec4f(0.20f, 0.20f, 0.20f,  1.0f)


struct ImFont;

namespace astro
{
  // forward declarations
  class SettingForm;
  
  // global view settings -- controls interface to settings window (Menu: View->Settings)
  class ViewSettings
  {
  private:
    bool  mState          = false; // whether window is open
    SettingForm *mForm    = nullptr;
    float mLabelColWidth  = 256.0f;
    Vec2f mWindowSize     = Vec2f(512, 512);
    Vec2f mWindowPadding  = Vec2f(10.0f, 10.0f);
    bool  mEscapeDebounce = false;

    bool checkExitPopup(bool busy, bool hover); // checks for view settings popup exit conditions
    
  public:
    // Global
    float mainTextSize     = 16.0f;
    float titleTextSize    = 20.0f;
    ImFont *mainFont       = nullptr;
    ImFont *titleFont      = nullptr;
    
    // Node Graph
    Vec4f graphBgColor     = DEFAULT_GRAPH_BG_COLOR;
    bool  drawGraphLines   = DEFAULT_GRAPH_DRAW_LINES;
    bool  drawGraphAxes    = DEFAULT_GRAPH_DRAW_AXES;
    Vec4f graphLineColor   = DEFAULT_GRAPH_LINE_COLOR;
    Vec4f graphAxesColor   = DEFAULT_GRAPH_AXES_COLOR;
    bool  glSpacingEqual   = DEFAULT_GL_SPACING_EQUAL;
    Vec2f graphLineSpacing = DEFAULT_GRAPH_LINE_SPACING;
    float graphLineWidth   = DEFAULT_GRAPH_LINE_WIDTH;
    // Nodes
    Vec4f nodeBgColor      = DEFAULT_NODE_BG_COLOR;

    // TODO: Charts
    // --> aspect colors
    // --> extended object list
    
    ViewSettings();
    ~ViewSettings();

    void reset();
    
    void openWindow()   { mState = true; }
    void closeWindow()  { mState = false; }
    void toggleWindow();
    bool draw(const Vec2f &frameSize);
  };
}

#endif // VIEW_SETTINGS_HPP
