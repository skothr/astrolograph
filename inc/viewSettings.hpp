#ifndef VIEW_SETTINGS_HPP
#define VIEW_SETTINGS_HPP

#include "vector.hpp"

#define FONT_PATH   "./res/fonts/UbuntuMono-R.ttf"
#define MAIN_FONT_HEIGHT 16.0f
#define TITLE_FONT_HEIGHT 20.0f

struct ImFont;

namespace astro
{
  // global view settings -- controls interface to settings window (Menu: View->Settings)
  class ViewSettings
  {
  private:

    float mLabelColWidth = 256.0f;

    // returns whether setting is busy being modified
    bool colorSetting(const std::string &name, const std::string &id, Vec4f *color, bool busy);
    
  public:
    // Global
    float mainTextSize     = 10.0f;
    float titleTextSize    = 16.0f;

    ImFont *mainFont       = nullptr;
    ImFont *titleFont      = nullptr;
    
    // Node Graph
    Vec4f graphBgColor     = Vec4f(0.05f, 0.05f, 0.05f,  1.0f);
    bool  drawGraphLines   = true;
    bool  drawGraphAxes    = true;
    Vec4f graphLineColor   = Vec4f(0.15f, 0.15f, 0.15f,  1.0f);
    Vec4f graphAxesColor   = Vec4f(0.5f,  0.5f,  0.5f,   1.0f);
    Vec2f graphLineSpacing = Vec2f(100.0f, 100.0f);
    float graphLineWidth   = 1.0f;
    
    // Nodes
    Vec4f nodeBgColor      = Vec4f(0.20f, 0.20f, 0.20f,  1.0f);
    bool  mState           = false; // whether window is open
    
    ViewSettings();
    ~ViewSettings();

    bool checkExitPopup(bool hover);
    
    void openWindow()  { mState = true; }
    void closeWindow() { mState = false; }
    void toggleWindow();
    bool draw();
  };
}

#endif // VIEW_SETTINGS_HPP
