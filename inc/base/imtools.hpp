#ifndef ASTRO_IMTOOLS_HPP
#define ASTRO_IMTOOLS_HPP

#include <imgui.h>
#include <imgui_internal.h>
#include <vector>
#include <string>
#include <cmath>

#include "astro.hpp"
#include "glfwKeys.hpp"
#include "geometry.hpp"

namespace astro
{
#define ASPECT_MAX_W        3.0f
#define ASPECT_BORDER_MAX_W 1.0f
#define ASPECT_BORDER_COLOR Vec4f(1,1,1,1)

#define ARROW_COLOR         Vec4f(0.0f, 0.0f, 0.0f, 1.0f)
#define ARROW_BORDER_COLOR  Vec4f(1.0f, 1.0f, 1.0f, 1.0f)
#define ARROW_BORDER_W      2.0f // width of border
#define ARROW_WIDTH         7.0f // size in tangential direction
#define ARROW_LENGTH        4.0f // size in radial direction
  
  // begin imgui tooltip with spacing/padding set for scaling
#define TOOLTIP_PADDING      Vec2f(4.0f, 4.0f)
#define TOOLTIP_SPACING      Vec2f(4.0f, 4.0f)
#define TOOLTIP_BORDER_SIZE  1.0f
#define TOOLTIP_GRAB_MINSIZE 12.0f




  
  inline void BeginInvariantScaling(float scale=-1.0f)
  { // same spacing at any scale
    if(scale > 0.0f) { ImGui::SetWindowFontScale(1.0f/scale); }
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    TOOLTIP_PADDING*2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize,  TOOLTIP_BORDER_SIZE);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,     TOOLTIP_PADDING);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,      TOOLTIP_SPACING);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, TOOLTIP_SPACING);
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,      TOOLTIP_PADDING);
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing,    TOOLTIP_SPACING.x);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize,      TOOLTIP_GRAB_MINSIZE);
  }
  inline void EndInvariantScaling(float scale=-1.0f) { if(scale > 0.0f) { ImGui::SetWindowFontScale(scale); } ImGui::PopStyleVar(8); }
  
  inline void BeginTooltip()
  {
    BeginInvariantScaling();
    ImGui::BeginTooltip();
  }
  inline void EndTooltip() { ImGui::EndTooltip(); EndInvariantScaling(); }


  // context window --> must call EndContext() if BeginContext() returns true
  extern bool g_closeContexts;
  extern int  g_contextsOpen;
  enum ContextWindowActivation { CONTEXT_WINDOW_BUTTON, CONTEXT_WINDOW_RCLICK, CONTEXT_WINDOW_LCLICK };
  inline bool BeginContext(const std::string &name, ContextWindowActivation activation, bool hovered=false, float scale=-1.0f)
  {
    // open context window with button or mouse click
    if((activation == CONTEXT_WINDOW_BUTTON && ImGui::Button(name.c_str())) ||
       (hovered && (activation == CONTEXT_WINDOW_RCLICK && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) ||
        (activation == CONTEXT_WINDOW_LCLICK && ImGui::IsMouseReleased(ImGuiMouseButton_Left)))
       && !g_closeContexts) { ImGui::OpenPopup(("##"+name).c_str()); }
    BeginInvariantScaling(scale);
    bool opened = ImGui::BeginPopup((std::string("##")+name).c_str()); // enter popup window
    if(g_closeContexts) { if(opened) { ImGui::CloseCurrentPopup(); ImGui::EndPopup(); } opened = false; }
    else if(opened) { g_contextsOpen++; } // (reset in nodeGraph.cpp)
    return opened;
  }
  inline void EndContext(bool opened, float scale=-1.0f) { EndInvariantScaling(scale); if(opened) { ImGui::EndPopup(); } }

  
  // improved checkbox
  inline bool Checkbox(const std::string &label, bool* v, const Vec2f &size=Vec2f(0,0))
  {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if(window->SkipItems) { return false; }

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = ImGui::GetStyle();
    const ImGuiID     id    = window->GetID(label.c_str());
    const Vec2f label_size  = ImGui::CalcTextSize(label.c_str(), NULL, true);

    float square_sz = (size.x > 0 || size.y > 0 ? std::max(size.x, size.y) : ImGui::GetFrameHeight());
    const float pad_mult  = 0.1f;
    const float frame_pad = square_sz*pad_mult;
    square_sz -= 2.0f*frame_pad;
    
    const Vec2f pos = Vec2f(window->DC.CursorPos) + Vec2f(2.0f*frame_pad);
    Vec2f       sz  = Vec2f(square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y*2.0f);
    if(size.x > 0.0f) { sz.x = size.x; } if(size.y > 0.0f) { sz.y = size.y; }
    
    const ImRect total_bb(pos, pos + sz);
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if(!ImGui::ItemAdd(total_bb, id)) { return false; }

    bool hovered, held; bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
    if(pressed) { *v = !(*v); ImGui::MarkItemEdited(id); }

    Rect2f check_bb(pos, pos + Vec2f(square_sz, square_sz));
    if(label_size.x > 0.0f)
      {
        ImGui::RenderText(ImVec2(check_bb.p1.x + style.ItemInnerSpacing.x, check_bb.p1.y + style.FramePadding.y), label.c_str());
      }

    check_bb += Vec2f(label_size.x, 0.0f);
    ImGui::RenderNavHighlight(total_bb, id);
    ImGui::RenderFrame(check_bb.p1, check_bb.p2, ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive :
                                                                    hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), true, style.FrameRounding);
    
    ImU32 check_col = ImGui::GetColorU32(ImGuiCol_CheckMark);
    if(*v)
      {
        const float pad = ImMax(1.0f, square_sz*pad_mult);
        ImGui::RenderCheckMark(window->DrawList, check_bb.p1+Vec2f(pad, pad), check_col, square_sz-2.0f*pad);
      }
    
    // if(g.LogEnabled)        { LogRenderedText(&total_bb.Min, mixed_value ? "[~]" : *v ? "[x]" : "[ ]"); }
    // IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
    return pressed;
  }



  inline bool ColorSelect(const std::string &label, Vec4f &col, Vec4f &lastCol, const Vec4f &defaultCol, float scale=1.0f)
  {
    bool busy = false;
    ImGuiColorEditFlags cFlags = (ImGuiColorEditFlags_DisplayRGB |
                                  ImGuiColorEditFlags_AlphaBar   |
                                  ImGuiColorEditFlags_AlphaPreviewHalf);// | ImGuiColorEditFlags_NoAlpha);
    BeginInvariantScaling();
    if(ImGui::ColorButton(label.c_str(), col, cFlags, Vec2f(20, 20)*scale))
      {
        lastCol = col;
        ImGui::OpenPopup((label+"popup").c_str());
      }
    ImGuiWindowFlags wFlags = (ImGuiWindowFlags_AlwaysAutoResize |
                               ImGuiWindowFlags_NoMove           |
                               ImGuiWindowFlags_NoTitleBar       |
                               ImGuiWindowFlags_NoResize );
    if(ImGui::BeginPopup((label+"popup").c_str(), wFlags))
      {
        busy = true;
        bool hover = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
        ImGui::ColorPicker4("VCpicker", col.data.data(), cFlags, lastCol.data.data());
        
        if(ImGui::Button("Select") || ImGui::IsKeyPressed(GLFW_KEY_ENTER) || (!hover && ImGui::IsMouseClicked(ImGuiMouseButton_Left))) // selects color
          { busy = false; ImGui::CloseCurrentPopup(); }
        ImGui::SameLine();
        if(ImGui::Button("Cancel") || ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)) // cancels current selection
          { col = lastCol; busy = false; ImGui::CloseCurrentPopup(); }
        ImGui::SameLine(); if(ImGui::Button("Reset")) { col = defaultCol; }
        ImGui::EndPopup();
      }
    EndInvariantScaling();

    return busy;
  }










  

  
  // draws a bordered line (simple rectangle)
  //   TODO: endpoint type (e.g. triangular, rounded, 
  inline void drawLine(ImDrawList *drawList, Vec2f p0, Vec2f p1,              // screen points to connect, and center point
                       const Vec4f &color, float lWidth,                      // color/width of border
                       const Vec4f &bColor=Vec4f(0,0,0,0), float bWidth=0.0f, // color/width of border
                       float shear0 = 1.0f, float shear1 = 1.0f)              // shear at each point (scales line width locally)
  {
    if(!drawList) { return; }
    
    // calculations
    Vec2f v  = (p1 - p0).normalized(); // vector between points
    Vec2f n  = Vec2f(v.y, -v.x);       // normal
    Vec2f mp = (p0 + p1) / 2.0f;       // midpoint

    // draw line body (ends at specified points)
    Vec2f p00 = p0 - (shear0*lWidth/2.0f)*n;
    Vec2f p01 = p0 + (shear0*lWidth/2.0f)*n;
    Vec2f p10 = p1 - (shear1*lWidth/2.0f)*n;
    Vec2f p11 = p1 + (shear1*lWidth/2.0f)*n;
    drawList->AddTriangleFilled(p00, p01, p10, ImColor(color));
    drawList->AddTriangleFilled(p10, p01, p11, ImColor(color));

    if(bWidth > 0.0 && bColor.w > 0.0)
      { // draw border
        Vec2f bp00 = p00 + (-v - n)*bWidth;
        Vec2f bp01 = p01 + (-v + n)*bWidth;
        Vec2f bp10 = p10 + ( v - n)*bWidth;
        Vec2f bp11 = p11 + ( v + n)*bWidth;    

        std::vector<Vec2f> points  { p00,  p10,  p11,  p01  }; // inner body points   (clockwise)
        std::vector<Vec2f> bPoints { bp00, bp10, bp11, bp01 }; // outer border points (clockwise)
        
        for(int i = 0; i < points.size(); i++)
          {
            int j = (i+1) % points.size();
            drawList->AddTriangleFilled(points[i], bPoints[i], points[j],  ImColor(bColor));
            drawList->AddTriangleFilled(points[j], bPoints[i], bPoints[j], ImColor(bColor));
          }
      }
  }

  
  // draws an aspect line
  inline void drawAspect(ImDrawList *drawList, Vec2f p0, Vec2f p1, Vec2f cp,                         // screen points to connect, and center point
                         const Vec4f &color, double strength, const Vec4f &bColor, double bStrength, // colors of main shape/border (strength --> [0.0,1.0])
                         const std::string &imageName="", float scale=1.0f)
  {
    if(!drawList) { return; }

    if(p0.x >= p1.x) { std::swap(p1, p0); }
    
    // clamp strengths
    strength  = std::max(0.2, strength);
    bStrength = std::max(0.2, bStrength);
    
    // calculations
    float lWidth  = (ASPECT_MAX_W*strength*scale  + 1.0f);
    float bWidth  = (ASPECT_BORDER_MAX_W*bStrength*scale + 1.0f);
    float symSize = CHART_ASP_SYMBOL_SIZE*scale;
    float symRad  = symSize*0.7f;

    float dist    = (p1 - p0).length();       // aspect line length
    Vec2f mp      = (p0 + p1)/2.0f;           // aspect center

    Vec2f v = (p1 - p0).normalized();         // aspect vector
    Vec2f n = Vec2f(v.y, -v.x);               // aspect normal


    if(dist < 1.0) { v = (mp - cp).normalized(); v = Vec2f(-v.y, v.x); }
    
    float aOffset = atan2(-v.y, v.x);         // symbol ring angle offset
    float cornerW = lWidth/sin(2.0*M_PI/6.0); // symbol ring width skew (lWidth measures perpendicular to line direction)
    float oRad = symRad + cornerW/2.0f;       // symbol ring outer radius
    float iRad = symRad - cornerW/2.0f;       // symbol ring inner radius

    float hugOffset = (lWidth/2.0f)/tan(2.0*M_PI/12.0); // v-parallel offset so lines "hug" symbol ring
    
    float endLen  = (lWidth/2.0f)*tan(2.0*M_PI/6.0);                  // length of endpoint triangles
    float hLen    = (dist/2.0f - symRad) - endLen - cornerW + hugOffset; // length of line segments on either side of symbol

    if(dist >= symSize*1.2f)
      { // draw main lines if points not too close (i.e. conjunction)
        Vec2f p00 = p0 + endLen*v - (lWidth/2.0f)*n;
        Vec2f p10 = p1 - endLen*v - (lWidth/2.0f)*n;

        Vec2f p0Corner = mp - (symRad + cornerW/2.0f)*v;
        Vec2f p1Corner = mp + (symRad + cornerW/2.0f)*v;
        
        drawList->AddTriangleFilled(p0,             p00,                      p00 + lWidth*n,  ImColor(color)); // p0 endpoint triangle
        drawList->AddTriangleFilled(p00,            p00 + lWidth*n,           p0Corner,        ImColor(color)); // p0 line segment (3 triangles)
        drawList->AddTriangleFilled(p00,            p00 + hLen*v,             p0Corner,        ImColor(color)); // "
        drawList->AddTriangleFilled(p00 + lWidth*n, p00 + hLen*v + lWidth*n,  p0Corner,        ImColor(color)); // "
        drawList->AddTriangleFilled(p10,            p10 + lWidth*n,           p1Corner,        ImColor(color)); // p1 line segment (3 triangles)
        drawList->AddTriangleFilled(p10,            p10 - hLen*v,             p1Corner,        ImColor(color)); // "
        drawList->AddTriangleFilled(p10 + lWidth*n, p10 - hLen*v + lWidth*n,  p1Corner,        ImColor(color)); // "
        drawList->AddTriangleFilled(p1,  p10,                        p10          + lWidth*n, ImColor(color));  // p1 endpoint triangle
      }
    
    // draw symbol ring
    for(int i = 0; i < 6; i++)
      {
        float a0 = i*2.0*M_PI/6.0f     + aOffset;
        float a1 = (i+1)*2.0*M_PI/6.0f + aOffset;

        Vec2f v0 = Vec2f(cos(a0), -sin(a0));
        Vec2f v1 = Vec2f(cos(a1), -sin(a1));

        Vec2f pi0 = mp + iRad*v0;
        Vec2f po0 = mp + oRad*v0;
        Vec2f pi1 = mp + iRad*v1;
        Vec2f po1 = mp + oRad*v1;
        drawList->AddTriangleFilled(pi0, po0, po1, ImColor(color));
        drawList->AddTriangleFilled(pi0, po1, pi1, ImColor(color));
      }

    if(bStrength > 0.0 && bColor.w > 0.0)
      {
        // draw outer borders
        std::vector<Vec2f> points;  // body points (starting from p0, going clockwise
        std::vector<Vec2f> bPoints; // outer border points (matching body points)

        if(dist >= symSize*1.2f)
          {
            points.push_back(p0); // P0
            bPoints.push_back(points.back() - v*(bWidth/sin(2.0*M_PI/6.0)));
            points.push_back(p0 + v*endLen - n*(lWidth/2.0f));
            bPoints.push_back(points.back() - n*bWidth);
            points.push_back(p0 + v*(endLen+hLen) - n*(lWidth/2.0f));
            bPoints.push_back(points.back() - n*bWidth - v*(bWidth/tan(2.0*M_PI/6.0)));

            double angle = 0.0 + aOffset + (240.0*M_PI/180.0); // symbol ring upper-left
            Vec2f av = Vec2f(cos(angle), -sin(angle));
            points.push_back(mp + av*oRad);
            bPoints.push_back(points.back() + av*(bWidth/sin(2.0*M_PI/6.0)));
            angle = 0.0 + aOffset + (300.0*M_PI/180.0);        // symbol ring upper-right
            av = Vec2f(cos(angle), -sin(angle));
            points.push_back(mp + av*oRad);
            bPoints.push_back(points.back() + av*(bWidth/sin(2.0*M_PI/6.0)));

            points.push_back(p1 - v*(endLen+hLen) - n*(lWidth/2.0f));
            bPoints.push_back(points.back() - n*bWidth + v*(bWidth/tan(2.0*M_PI/6.0)));
            points.push_back(p1 - v*endLen - n*(lWidth/2.0f));
            bPoints.push_back(points.back() - n*bWidth);
            points.push_back(p1); // P1
            bPoints.push_back(points.back() + v*(bWidth/sin(2.0*M_PI/6.0)));
            points.push_back(p1 - v*endLen + n*(lWidth/2.0f));
            bPoints.push_back(points.back() + n*bWidth);
            points.push_back(p1 - v*(endLen+hLen) + n*(lWidth/2.0f));
            bPoints.push_back(points.back() + n*bWidth + v*(bWidth/tan(2.0*M_PI/6.0)));

            angle = 0.0 + aOffset + (60.0*M_PI/180.0); // symbol ring lower-right
            av = Vec2f(cos(angle), -sin(angle));
            points.push_back(mp + av*oRad);
            bPoints.push_back(points.back() + av*(bWidth/sin(2.0*M_PI/6.0)));
            angle = 0.0 + aOffset + (120.0*M_PI/180.0); // symbol ring lower-left
            av = Vec2f(cos(angle), -sin(angle));
            points.push_back(mp + av*oRad);
            bPoints.push_back(points.back() + av*(bWidth/sin(2.0*M_PI/6.0)));

            points.push_back(p0 + v*(endLen+hLen) + n*(lWidth/2.0f));
            bPoints.push_back(points.back() + n*bWidth - v*(bWidth/tan(2.0*M_PI/6.0)));
            points.push_back(p0 + v*endLen + n*(lWidth/2.0f));
            bPoints.push_back(points.back() + n*bWidth);
          }
        else
          { // conjunction -- just symbol ring
            for(int i = 0; i < 6; i++)
              {
                float a0 = i*2.0*M_PI/6.0f + aOffset;
                Vec2f v0 = Vec2f(cos(a0), -sin(a0));
                points.push_back(mp + oRad*v0);
                bPoints.push_back(mp + v0*(oRad+bWidth/sin(2.0*M_PI/6.0)));
              }
          }
    
        for(int i = 0; i < points.size(); i++)
          {
            int j = (i+1) % points.size();
            drawList->AddTriangleFilled(points[i], bPoints[i], points[j],  ImColor(bColor));
            drawList->AddTriangleFilled(points[j], bPoints[i], bPoints[j], ImColor(bColor));
          }

        // draw inner borders
        points.clear(); bPoints.clear();
        for(int i = 0; i < 6; i++)
          {
            float a0 = i*2.0*M_PI/6.0f + aOffset;
            Vec2f v0 = Vec2f(cos(a0), -sin(a0));
            points.push_back(mp + iRad*v0);
            bPoints.push_back(mp + v0*(iRad-bWidth/sin(2.0*M_PI/6.0)));
          }
        for(int i = 0; i < points.size(); i++)
          {
            int j = (i+1) % points.size();
            drawList->AddTriangleFilled(points[i], bPoints[i], points[j],  ImColor(bColor));
            drawList->AddTriangleFilled(points[j], bPoints[i], bPoints[j], ImColor(bColor));
          }
      }    
    
    // draw symbol if image exists
    if(!imageName.empty())
      {
        ChartImage *img = getImage(imageName);
        if(img)
          {
            ImGui::SetCursorScreenPos(mp-Vec2f(symSize, symSize)/2.0f);
            ImGui::Image(img->id(), Vec2f(symSize, symSize), Vec2f(0,0), Vec2f(1,1), ImColor(color), Vec4f(0,0,0,0));
          }
      }
  }


  // draws a triangle denoting an object's position
  // p --> point arrow points to, v --> vector pointing away from center of circle
  inline void drawObjArrow(ImDrawList *drawList, Vec2f p, Vec2f v, float scale=1.0f, bool hollow=false)
  {
    // measurements (TODO: simplify)
    Vec2f n(v.y, -v.x); // normal (tangent to circle)
    
    float theta = atan(ARROW_LENGTH / (ARROW_WIDTH/2.0f));  // angle of corners not touching p
    float cTheta = M_PI/2.0 - theta;
    float cTheta2 = (M_PI - 2.0f*theta)/2.0f;
    float pTheta = M_PI - 2.0f*theta;                       // angle of corner touching p

    float dTt = ARROW_BORDER_W * tan(cTheta) + ARROW_BORDER_W / cos(cTheta2);
    float dTp = ARROW_BORDER_W / sin(pTheta/2.0f);
    
    // fill points
    Vec2f p0 = p + (dTp*scale)*v;
    Vec2f p1 = p0 + (ARROW_LENGTH*scale)*v - (ARROW_WIDTH/2.0f*scale)*n;
    Vec2f p2 = p1 + (ARROW_WIDTH*scale)*n;

    // border points
    Vec2f bp0 = p;
    Vec2f bp1 = p1 - (dTt*scale)*n + (ARROW_BORDER_W*scale)*v;
    Vec2f bp2 = p2 + (dTt*scale)*n + (ARROW_BORDER_W*scale)*v;

    if(hollow)
      { // draw border only
        drawList->AddTriangleFilled(bp0, p0, bp1, ImColor(ARROW_BORDER_COLOR));
        drawList->AddTriangleFilled(bp1, p0, p1, ImColor(ARROW_BORDER_COLOR));
        drawList->AddTriangleFilled(bp1, p1, bp2, ImColor(ARROW_BORDER_COLOR));
        drawList->AddTriangleFilled(bp2, p1, p2, ImColor(ARROW_BORDER_COLOR));
        drawList->AddTriangleFilled(bp2, p2, bp0, ImColor(ARROW_BORDER_COLOR));
        drawList->AddTriangleFilled(bp0, p2, p0, ImColor(ARROW_BORDER_COLOR));
      }
    else
      {
        // draw border first
        drawList->AddTriangleFilled(bp0, bp1, bp2, ImColor(ARROW_BORDER_COLOR));
        // fill second (on top)
        drawList->AddTriangleFilled(p0, p1, p2, ImColor(ARROW_COLOR));
      }
  }

}


#endif // ASTRO_IMTOOLS_HPP
