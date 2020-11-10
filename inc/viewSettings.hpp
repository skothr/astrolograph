#ifndef VIEW_SETTINGS_HPP
#define VIEW_SETTINGS_HPP

#include "vector.hpp"


namespace astro
{
  // global view settings -- controls interface to settings window (Menu: View->Settings)
  struct ViewSettings
  {
    // Node Graph
    Vec4f graphBgColor     = Vec4f(0.05f, 0.05f, 0.05f,  1.0f);
    bool  drawGraphLines   = true;
    Vec4f graphLineColor   = Vec4f(0.15f, 0.15f, 0.15f,  1.0f);
    Vec4f graphAxesColor   = Vec4f(0.5f,  0.5f,  0.5f,   1.0f);
    Vec2f graphLineSpacing = Vec2f(100.0f, 100.0f);
    float graphLineWidth   = 1.0f;
    
    // Nodes
    Vec4f nodeBgColor      = Vec4f(0.20f, 0.20f, 0.20f,  1.0f);
    bool  mState           = false; // whether window is open
    ViewSettings() { }

    bool checkExitPopup(bool hover);
    
    void openWindow();
    void closeWindow();
    void toggleWindow();
    bool draw();
  };
}

#endif // VIEW_SETTINGS_HPP
