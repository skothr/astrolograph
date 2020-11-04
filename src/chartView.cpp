#include "chartView.hpp"
using namespace astro;

#include <GLFW/glfw3.h> // for keys

#include "ephemeris.hpp"
#include "tools.hpp"
#include "vector.hpp"

ChartView::ChartView()
  : mFocusObjects(OBJ_COUNT + ANGLE_END-ANGLE_OFFSET, false)
{ }

//// ZODIAC (OUTER RING/SIGNS) ////
void ChartView::renderZodiac(Chart *chart, const ViewParams &params, ImDrawList *draw_list)
{
  Vec2f cc = params.center; // shorthand
  Vec2f t0(0.0f, 0.0f);
  Vec2f t1(1.0f, 1.0f);
  
  // draw chart zodiac
  Vec2f cp1 = params.oRadius*Vec2f(cos(0.0f), -sin(0.0f));           // first edge point on circle  (angle=0)
  Vec2f cp2 = params.oRadius*Vec2f(cos(M_PI/6.0f), -sin(M_PI/6.0f)); // second edge point on circle (angle=(pi/12))
  Vec2f mp = (cp1+cp2)/2.0f;                                         // midpoint
  float sr = params.oRadius - mp.length();                           // distance from edge of dodecagon to midpoint of side

  draw_list->AddNgon(cc, params.oRadius, ImColor(Vec4f(1.0f, 1.0f, 1.0f, 1.0f)), 12, OUTLINE_W); // outer dodecagon
  draw_list->AddNgon(cc, params.iRadius, ImColor(Vec4f(1.0f, 1.0f, 1.0f, 1.0f)), 90, OUTLINE_W); // inner circle
  
  // draw object ring
  draw_list->AddNgon(cc, params.objRadius, ImColor(Vec4f(1.0f, 1.0f, 1.0f, 1.0f)), 128, OBJRING_OUTLINE_W);

  // draw sign symbols
  float signRadius = (params.iRadius+params.oRadius-sr)/2.0f;
  for(int i = 0; i < 12; i++)
    {
      float angle = screenAngle(chart, chart->swe().getSignCusp(i));
      Vec2f v(cos(angle), -sin(angle));

      // draw sign division
      Vec2f pi = cc + params.objRadius*v;
      Vec2f po = cc + (params.oRadius)*v;
      // painter->drawLine(LineDesc{pi, po, 2.0f, Vec4f(0.5f, 0.5f, 0.5f, 1.0f)});
      draw_list->AddLine(pi, po, ImColor(Vec4f(0.5f, 0.5f, 0.5f, 1.0f)), OUTLINE_W);
      
      // draw sign symbol
      angle += M_PI/180.0f*15.0f;
      Vec2f v2(cos(angle), -sin(angle));
      Vec2f pc = cc + signRadius*v2;

      // RenderImage test;
      std::string sName = SIGN_NAMES[i];
      
      ChartImage *img = getImage(sName);
      if(img)
        {
          Vec2f imSize = Vec2f(64.0f, 64.0f)*params.sizeRatio;
          ImGui::SetCursorScreenPos(pc-imSize/2.0f);
          Vec4f tintCol(1.0f, 1.0f, 1.0f, 1.0f);
          Vec4f borderCol(0.0f, 0.0f, 0.0f, 0.0f);
          ImGui::Image((ImTextureID)img->texId, imSize, t0, t1, ImColor(tintCol), borderCol);
          if(ImGui::IsItemHovered())
            {
              ImGui::BeginTooltip();
              {
                ImGui::BeginTable("##tooltip-sign", 3, ImGuiTableFlags_SizingPolicyStretchX);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthAlwaysAutoResize);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthAlwaysAutoResize);
                {
                  ImGui::TableNextRow();
                  ImGui::TableSetColumnIndex(0);
                  // sign name
                  ImGui::TextUnformatted(sName.c_str());
                  ImGui::Separator();
                  ImGui::TableNextColumn();
                  ImGui::TableNextColumn();
                  // contained objects
                  for(auto obj : chart->objects())
                    {
                      int si = chart->swe().getSign(obj->angle);
                      if(si == i)
                        {
                          ImGui::TableNextRow();
                          ImGui::TableSetColumnIndex(0);
                          // object name
                          std::string oName = getObjName(obj->type);
                          ImGui::TextUnformatted(oName.c_str());
                          ImGui::TableNextColumn();
                          // object symbol
                          ChartImage *oImg = getWhiteImage(oName);
                          Vec4f oColor = getObjColor(oName);
                          ImGui::Image((ImTextureID)oImg->texId, Vec2f(20.0f, 20.0f), t0, t1, oColor, borderCol);
                          ImGui::TableNextColumn();
                          // object angle
                          ImGui::TextUnformatted(angle_string(obj->angle - chart->swe().getSignCusp(i), true).c_str());
                        }
                    }
                }
                ImGui::EndTable();
              }
              ImGui::EndTooltip();
            }
        }
    }
}

//// HOUSES ////
void ChartView::renderHouses(Chart *chart, const ViewParams &params, ImDrawList *draw_list)
{
  Vec2f cc = params.center; // shorthand
  Vec2f t0(0.0f, 0.0f);
  Vec2f t1(1.0f, 1.0f);
  
  // draw houses
  float numOffset = CHART_HOUSE_NUM_OFFSET;
  float ocRadius =  CHART_HOUSE_CIRCLE_RADIUS; // radius of circles around outer house numbers
  for(int i = 1; i <= 12; i++)
    {
      float angle1 = chart->swe().getHouseCusp(i);                    // this house's cusp
      float angle2 = chart->swe().getHouseCusp((i < 12 ? (i+1) : 1)); // next house's cusp
      float houseSize = astro::angleDiffDegrees(angle2, angle1);         // angular size of house

      // house cusp line
      float angle = screenAngle(chart, angle1);
      Vec2f v(cos(angle), -sin(angle));
      Vec2f pe = cc + params.eRadius*v;
      Vec2f pi = cc + params.iRadius*v;
      Vec2f pa = cc + (params.oRadius + numOffset - ocRadius)*v;
      draw_list->AddLine(pe, pa, ImColor(Vec4f(0.7f, 0.7f, 0.7f, 0.5f)), 1.5f);
      // painter->drawLine(LineDesc{pe, pi, 1.5f, Vec4f(0.25f, 0.25f, 0.25f, 1.0f)}); //, Vec4f(1.0f, 1.0f, 1.0f, 1.0f), 2);

      // inner house number text
      float tAngle = screenAngle(chart, (angle1 + houseSize/2.0f)); // center angle of house
      Vec2f tSize = ImGui::CalcTextSize(std::to_string(i).c_str());
      Vec2f tp = cc + (params.eRadius+numOffset)*Vec2f(cos(tAngle), -sin(tAngle)) - tSize/2.0f;
      std::stringstream ss; ss << i;
      
      ImGui::SetCursorScreenPos(tp);
      ImGui::TextColored(Vec4f(0.7f, 0.7f, 0.7f, 0.7f), ss.str().c_str());

      // outer house number text
      tAngle = screenAngle(chart, angle1); // angle of house cusp
      tp = cc + (params.oRadius + numOffset)*Vec2f(cos(tAngle), -sin(tAngle)) - tSize/2.0f;// - Vec2f(8.0f, 8.0f);
      
      draw_list->AddCircle(tp + tSize/2.0f, ocRadius, ImColor(Vec4f(0.7f, 0.7f, 0.7f, 0.7f)), 12, 1.0f);
      
      ImGui::SetCursorScreenPos(tp);
      ImGui::TextColored(Vec4f(0.7f, 0.7f, 0.7f, 0.7f), ss.str().c_str());
      if(ImGui::IsItemHovered())
        {
          ImGui::BeginTooltip();
          {
            ImGui::BeginTable("##tooltip-house", 3, ImGuiTableFlags_SizingPolicyStretchX);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthAlwaysAutoResize);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthAlwaysAutoResize);
            {
              ImGui::TableNextRow();
              ImGui::TableSetColumnIndex(0);
              // house name
              ImGui::Text("house %d  ", i);
              ImGui::Separator();
              ImGui::Spacing();
              ImGui::TableNextColumn();
              int hSign = chart->swe().getSign(angle1);
              ImGui::Image((ImTextureID)getWhiteImage(getSignName(hSign))->texId, Vec2f(20.0f, 20.0f), t0, t1,
                           ImColor(ELEMENT_COLORS[getSignElement(hSign)]), ImColor(Vec4f(0,0,0,0)));
              ImGui::TableNextColumn();
              ImGui::TextUnformatted(angle_string(angle1 - chart->swe().getSignCusp(hSign), true, false).c_str());
              // contained objects
              for(auto obj : chart->objects())
                {
                  int hi = chart->swe().getHouse(obj->angle);
                  if(hi == i)
                    {
                      ImGui::TableNextRow();
                      ImGui::TableSetColumnIndex(0);
                      // object name
                      std::string oName = getObjName(obj->type);
                      ImGui::TextUnformatted((oName+"  ").c_str());
                      ImGui::TableNextColumn();
                      // object symbol
                      ChartImage *oImg = getWhiteImage(oName);
                      Vec4f oColor = getObjColor(oName);
                      ImGui::Image((ImTextureID)oImg->texId, Vec2f(20.0f, 20.0f), t0, t1, oColor, ImColor(Vec4f(0,0,0,0)));
                      ImGui::TableNextColumn();
                      // object angle
                      ImGui::TextUnformatted(angle_string(obj->angle - chart->swe().getHouseCusp(i), true, false).c_str());
                    }
                }
            }
            ImGui::EndTable();
          }
          ImGui::EndTooltip();
        }
    }
}


void ChartView::renderAngles(Chart *chart, const ViewParams &params, ImDrawList *draw_list)
{
  Vec2f cc = params.center; // shorthand
  Vec2f t0(0.0f, 0.0f);
  Vec2f t1(1.0f, 1.0f);

  for(int a = ANGLE_OFFSET; a < ANGLE_END; a++)
    {
      std::string name = ANGLE_NAMES[a-ANGLE_OFFSET];
      std::string longName = ANGLE_NAMES_LONG[a-ANGLE_OFFSET];
      float angle = screenAngle(chart, (chart->swe().getAngle((ObjType)a)));
      Vec2f v = Vec2f(cos(angle), -sin(angle));
      Vec2f p = cc + params.angRadius*v;
      ChartImage *img = getWhiteImage(name);
      Vec2f imSize = Vec2f(params.symbolSize, params.symbolSize);
      if(img)
        {
          ImGui::SetCursorScreenPos(p - imSize/2.0f);
          Vec4f tintCol  (1.0f, 1.0f, 1.0f, 1.0f);
          Vec4f borderCol(0.0f, 0.0f, 0.0f, 0.0f);
          ImGui::Image((ImTextureID)img->texId, imSize, t0, t1, ImColor(tintCol), borderCol);
          if(ImGui::IsItemHovered())
            {
              ImGui::BeginTooltip();
              ImGui::Text("%s", longName.c_str());
              // sign symbol
              ImGui::SameLine();
              double oAngle = chart->swe().getAngle((ObjType)a);
              int oSign = chart->swe().getSign(oAngle);
              ChartImage *sImg = getWhiteImage(getSignName(oSign));
              Vec4f sColor = ELEMENT_COLORS[getSignElement(oSign)];
              ImGui::Image((ImTextureID)sImg->texId, Vec2f(20.0f, 20.0f), t0, t1, ImColor(sColor), borderCol);
              // angle
              ImGui::SameLine();
              double aAngle = chart->swe().getAngle((ObjType)a);
              ImGui::Text("%s", angle_string(aAngle - chart->swe().getSignCusp(chart->swe().getSign(aAngle)), false).c_str());
              ImGui::EndTooltip();
            }
          if(chart->objects()[getObjId(name)-ANGLE_OFFSET+OBJ_COUNT]->focused)
            { draw_list->AddNgon(p, params.symbolSize*0.75f, ImColor(Vec4f(1.0f, 1.0f, 1.0f, 1.0f)), 8, 4.0f); }
        }
      // draw axis line (ascendent tinted red)
      Vec4f lineColor = (a == ANGLE_ASC ? Vec4f(1.0f, 0.4f, 0.4f, 0.4f) : Vec4f(1.0f, 1.0f, 1.0f, 0.4f));
      float lineWidth = (a == ANGLE_ASC ? 5.0f : 3.0f);
      draw_list->AddLine(cc, cc+(params.oRadius+CHART_HOUSE_NUM_OFFSET - CHART_HOUSE_CIRCLE_RADIUS)*v, ImColor(lineColor), lineWidth);
    }
}

//// ASPECTS ////
void ChartView::renderAspects(Chart *chart, const ViewParams &params, ImDrawList *draw_list)
{
  Vec2f cc = params.center; // shorthand
  Vec2f t0(0.0f, 0.0f);
  Vec2f t1(1.0f, 1.0f);
  
  bool showFocus = true;
  bool anyFocused = false;
  for(auto obj : chart->objects())
    { anyFocused |= (showFocus && obj->focused); }
  for(auto asp : chart->aspects())
    { anyFocused |= (showFocus && asp.focused); }
  for(int i = 0; i < ASPECT_COUNT; i++)
    { anyFocused |= (showFocus && chart->getAspectFocus((AspectType)i)); }

  
  // get lunar node opposition aspect
  const Aspect *lAsp = nullptr;
  for(int i = chart->aspects().size()-1; i >= 0; i--)
    {
      const Aspect &asp = chart->aspects()[i];
      if((asp.obj1->type == OBJ_NORTHNODE && asp.obj2->type == OBJ_SOUTHNODE) ||
         (asp.obj1->type == OBJ_SOUTHNODE && asp.obj2->type == OBJ_NORTHNODE) )
        { lAsp = &asp; break; } // skip lunar node opposition (drawn differently)
    }
  
  // draw lunar node opposition separately (underneath)
  if(lAsp)
    {
      float angle1 = screenAngle(chart, (lAsp->obj1->angle));
      float angle2 = screenAngle(chart, (lAsp->obj2->angle));
      
      Vec2f v1(cos(angle1), -sin(angle1));
      Vec2f v2(cos(angle2), -sin(angle2));
      
      Vec2f p1 = cc + params.objRadius*v1;
      Vec2f p2 = cc + params.objRadius*v2;
      Vec2f pc = (p1 + p2)/2.0f;

      Vec2f n1 = (p1 - pc).normalized();
      Vec2f n2 = (p2 - pc).normalized();

      Vec4f color = Vec4f(0.85f, 0.5f, 0.85f, 0.8f);
      draw_list->AddLine(p1, p2, ImColor(color), 2.5f);
    }
  
  bool ttColSetup = false; // set to true when tooltip columns have been set up
  
  // draw other aspects (TODO: backwards?)
  //for(int i = chart->aspects().size()-1; i >= 0; i--)
  for(int i = 0; i < chart->aspects().size(); i++)
    {
      const Aspect &asp = chart->aspects()[i];
      if((asp.obj1->type == OBJ_NORTHNODE && asp.obj2->type == OBJ_SOUTHNODE) ||
         (asp.obj1->type == OBJ_SOUTHNODE && asp.obj2->type == OBJ_NORTHNODE) )
        { lAsp = &asp; continue; } // skip lunar node opposition (drawn differently)
      if(((!chart->getAspectVisible(asp.type)) || (!asp.obj1->visible) || (!asp.obj2->visible)))// && !showFocus)
        { continue; } // skip if switched off (but always show if in focus)

      std::string aName = getAspectName(asp.type);
      float angle1 = screenAngle(chart, (asp.obj1->angle));
      float angle2 = screenAngle(chart, (asp.obj2->angle));
      
      Vec2f v1(cos(angle1), -sin(angle1));
      Vec2f v2(cos(angle2), -sin(angle2));
      
      Vec2f p1 = cc + params.objRadius*v1;
      Vec2f p2 = cc + params.objRadius*v2;
      Vec2f pc = (p1 + p2)/2.0f;

      Vec2f n1 = (p1 - pc).normalized();
      Vec2f n2 = (p2 - pc).normalized();

      const bool sq = true;
      float stren = (sq ? asp.strength*asp.strength : asp.strength);
      
      Vec4f color = getAspect(asp.type)->color;
      color.w = ((anyFocused && !(asp.focused || chart->getAspectFocus(asp.type) ||
                                  asp.obj1->focused || asp.obj2->focused)) ? 0.0 : 0.8)*stren;
      
      float symSize = params.symbolSize*0.9f;
      float sqStrength = std::max(0.2f, stren); // clamp to threshold

      Vec4f bgColor = Vec4f(0.0f, 0.0f, 0.0f, color.w);
      if(asp.focused || chart->getAspectFocus(asp.type) || asp.obj1->focused || asp.obj2->focused)
        { bgColor = Vec4f(1.0f, 1.0f, 1.0f, color.w); }
      if(bgColor.w < 0.01f) { continue; }

      float lineDist = symSize*0.7f;
      float lWidth = 5.0f*sqStrength + 1.0f;
      if((p1-p2).length() <= symSize*1.2f)
        { // (currently only conjunction aspects)
          p1 = cc + (params.objRadius-symSize)*v1;
          p2 = cc + (params.objRadius-symSize)*v2;
          pc = (p1 + p2)/2.0f;
        }
      else
        {
          draw_list->AddLine(p1, pc+lineDist*n1, ImColor(Vec4f(color.x, color.y, color.z, color.w)), lWidth);
          draw_list->AddLine(pc+lineDist*n2, p2, ImColor(Vec4f(color.x, color.y, color.z, color.w)), lWidth);
        }

      // draw aspect symbol and hexagon
      ChartImage *img = getImage(aName);
      Vec2f imSize = Vec2f(symSize, symSize);
      if(img)
        {
          ImGui::SetCursorScreenPos(pc-imSize/2.0f);
          Vec4f borderCol(0.0f, 0.0f, 0.0f, 0.0f);
          // draw aspect symbol inside hexagon
          draw_list->AddCircle(pc, lineDist, ImColor(color), 6, 3);
          ImGui::Image((ImTextureID)img->texId, imSize, t0, t1, color, borderCol);
          
          bool hover = ImGui::IsItemHovered();
          if(hover)
            { // display tooltip with aspect info
              Vec4f c1 = getObjColor(getObjName(asp.obj1->type));
              Vec4f c2 = getObjColor(getObjName(asp.obj2->type));
              ImGui::BeginTooltip();
              {
                ImGui::BeginTable("#tooltip-aspects", 3, ImGuiTableFlags_NoClip);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthAlwaysAutoResize);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthAlwaysAutoResize);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                
                // angle name
                ImGui::TextUnformatted(aName.c_str());
                ImGui::TableNextColumn();
                // aspect object1 symbol
                ImGui::Image((ImTextureID)getWhiteImage(getObjName(asp.obj1->type))->texId, Vec2f(20.0f, 20.0f), t0, t1, c1, borderCol);
                // aspect symbol
                ImGui::SameLine();
                Vec2f screenPos = Vec2f(ImGui::GetCursorScreenPos()) + Vec2f(10.0f, 10.0f);
                // draw aspect symbol full-color
                ImGui::Image((ImTextureID)img->texId, Vec2f(20.0f, 20.0f), t0, t1, ImColor(color.x, color.y, color.z, 0.7f), borderCol);
                // weight surrounding hexagon color by aspect strength
                ImGui::GetWindowDrawList()->AddCircle(screenPos, lineDist*20.0f/params.symbolSize, ImColor(color), 6, 2);
                // aspect object2 symbol
                ImGui::SameLine();
                ImGui::Image((ImTextureID)getWhiteImage(getObjName(asp.obj2->type))->texId, Vec2f(20.0f, 20.0f), t0, t1, c2, borderCol);
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(angle_string(asp.orb, true).c_str());
                  
                ImGui::EndTable();
              }
              ImGui::EndTooltip();
            }
        }
    }
}

//// ASPECTS ////
void ChartView::renderCompareAspects(ChartCompare *compare, const ViewParams &params, ImDrawList *draw_list)
{
  Vec2f cc = params.center; // shorthand
  Vec2f t0(0.0f, 0.0f);
  Vec2f t1(1.0f, 1.0f);

  Chart *oChart = compare->getOuterChart();
  Chart *iChart = compare->getInnerChart();

  if(!oChart || !iChart) { return; }
  
  bool showFocus = true;
  bool anyFocused = false;
  for(auto obj : oChart->objects())     { anyFocused |= (showFocus && obj->focused); }
  for(auto obj : iChart->objects())     { anyFocused |= (showFocus && obj->focused); }
  for(auto asp : compare->aspects())    { anyFocused |= (showFocus && asp.focused); }
  for(int i = 0; i < ASPECT_COUNT; i++) { anyFocused |= (showFocus && compare->getAspectFocus((AspectType)i)); }
  
  bool ttColSetup = false; // set to true when tooltip columns have been set up

  float obj1Radius = params.objRadius - params.objRingW;
  float obj2Radius = (params.objRadius - params.objRingW);// - params.objRingW);
  
  // draw other aspects (backwards --> ??)
  //for(int i = compare->aspects().size()-1; i >= 0; i--)
  for(int i = 0; i < compare->aspects().size(); i++)
    {
      const Aspect &asp = compare->aspects()[i];
      if(((!compare->getAspectVisible(asp.type)) || (!asp.obj1->visible) || (!asp.obj2->visible)))// && !showFocus)
        { continue; } // skip if switched off (but always show if in focus)

      float angle1 = screenAngle(compare, (asp.obj1->angle));
      float angle2 = screenAngle(compare, (asp.obj2->angle));
      
      Vec2f v1(cos(angle1), -sin(angle1));
      Vec2f v2(cos(angle2), -sin(angle2));
      
      Vec2f p1 = cc + obj1Radius*v1;
      Vec2f p2 = cc + obj2Radius*v2; // offset obj2 to inner object ring
      Vec2f pc = (p1 + p2)/2.0f;

      Vec2f n1 = (p1 - pc).normalized();
      Vec2f n2 = (p2 - pc).normalized();

      const bool sq = true;
      float stren = (sq ? asp.strength*asp.strength : asp.strength);
      
      Vec4f color = getAspect(asp.type)->color;
      color.w = ((anyFocused && !(asp.focused || compare->getAspectFocus(asp.type) ||
                                  asp.obj1->focused || asp.obj2->focused)) ? 0.0 : 0.8)*stren;
      
      float symSize = params.symbolSize*0.9f;
      float sqStrength = std::max(0.2f, stren); // clamp to threshold

      Vec4f bgColor = Vec4f(0.0f, 0.0f, 0.0f, color.w);
      if(asp.focused || compare->getAspectFocus(asp.type) || asp.obj1->focused || asp.obj2->focused)
        { bgColor = Vec4f(1.0f, 1.0f, 1.0f, color.w); }
      if(bgColor.w < 0.01f)
        { continue; }

      float lineDist = symSize*0.7f;
      float lWidth = 5.0f*sqStrength + 1.0f;
      if((p1-p2).length() <= symSize*1.2f + params.objRingW)
        { // (currently only conjunction aspects)
          p1 = cc + (obj2Radius-symSize)*v1;
          p2 = cc + (obj2Radius-symSize)*v2;
          pc = (p1 + p2)/2.0f;
        }
      else
        {
          draw_list->AddLine(p1, pc+lineDist*n1, ImColor(Vec4f(color.x, color.y, color.z, color.w)), lWidth);
          draw_list->AddLine(pc+lineDist*n2, p2, ImColor(Vec4f(color.x, color.y, color.z, color.w)), lWidth);
        }

      // draw aspect symbol and hexagon
      ChartImage *img = getImage(getAspectName(asp.type));
      Vec2f imSize = Vec2f(symSize, symSize);
      if(img)
        {
          ImGui::SetCursorScreenPos(pc-imSize/2.0f);
          Vec4f borderCol(0.0f, 0.0f, 0.0f, 0.0f);
          // draw aspect symbol inside hexagon
          draw_list->AddCircle(pc, lineDist, ImColor(color), 6, 3);
          ImGui::Image((ImTextureID)img->texId, imSize, t0, t1, color, borderCol);
          
          bool hover = ImGui::IsItemHovered();
          if(hover)
            { // display tooltip with aspect info
              Vec4f c1 = getObjColor(getObjName(asp.obj1->type));
              Vec4f c2 = getObjColor(getObjName(asp.obj2->type));
              ImGui::BeginTooltip();
              {
                ImDrawList *draw_list_tt = ImGui::GetWindowDrawList();
                std::string aName = getAspectName(asp.type);
                ImGui::BeginTable("#tooltip-aspects", 3, ImGuiTableFlags_NoClip);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthAlwaysAutoResize);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthAlwaysAutoResize);
                {
                  ImGui::TableNextRow();
                  ImGui::TableSetColumnIndex(0);
                  // angle name
                  ImGui::TextUnformatted(aName.c_str());
                  ImGui::TableNextColumn();
                  // aspect object1 symbol
                  ImGui::Image((ImTextureID)getWhiteImage(getObjName(asp.obj1->type))->texId, Vec2f(20.0f, 20.0f), t0, t1, c1, borderCol);
                  // aspect symbol
                  ImGui::SameLine();
                  Vec2f screenPos = Vec2f(ImGui::GetCursorScreenPos()) + Vec2f(10.0f, 10.0f);
                  // draw aspect symbol full-color
                  ImGui::Image((ImTextureID)img->texId, Vec2f(20.0f, 20.0f), t0, t1, ImColor(color.x, color.y, color.z, 0.7f), borderCol);
                  // weight surrounding hexagon color by aspect strength
                  draw_list_tt->AddCircle(screenPos, lineDist*20.0f/params.symbolSize, ImColor(color), 6, 2);
                  // aspect object2 symbol
                  ImGui::SameLine();
                  ImGui::Image((ImTextureID)getWhiteImage(getObjName(asp.obj2->type))->texId, Vec2f(20.0f, 20.0f), t0, t1, c2, borderCol);
                  ImGui::TableNextColumn();
                  ImGui::TextUnformatted(angle_string(asp.orb, true).c_str());
                }
                ImGui::EndTable();
              }
              ImGui::EndTooltip();
            }
        }
    }
}

//// OBJECTS ////
//    level --> which object ring to draw (outer ring is 0) 
void ChartView::renderObjects(Chart *chart, int level, const ViewParams &params, ImDrawList *draw_list)
{
  Vec2f cc = params.center; // shorthand
  Vec2f t0(0.0f, 0.0f);
  Vec2f t1(1.0f, 1.0f);

  
  // draw object ring
  float ringRadius = params.objRadius - params.objRingW*level;
  //if(level == 0)
    {
      draw_list->AddNgon(cc, ringRadius, ImColor(Vec4f(1.0f, 1.0f, 1.0f, 1.0f)), 128, OBJRING_OUTLINE_W);

      // draw degree ticks
      for(int i = 0; i < 360; i++)
        {
          float rAngle = i*M_PI/180.0f;
          Vec2f v(cos(rAngle), sin(rAngle));

          int signAngle = i % 30;
          float tickLen = 0.0f;
          if(signAngle % 15 == 0)
            { tickLen = DEGREE_TICK_SIZE_15; }
          else if(signAngle % 10 == 0)
            { tickLen = DEGREE_TICK_SIZE_10; }
          else if(signAngle % 5 == 0)
            { tickLen = DEGREE_TICK_SIZE_5; }
          else
            { tickLen = DEGREE_TICK_SIZE_1; }
          tickLen *= params.sizeRatio;
      
          Vec2f p1 = cc + v*(ringRadius - tickLen/2.0f);
          Vec2f p2 = cc + v*(ringRadius + tickLen/2.0f);
          draw_list->AddLine(p1, p2, ImColor(Vec4f(1.0f, 1.0f, 1.0f, 0.7f)), 1.0f*params.sizeRatio);
        }
    }

  // draw object angle markers
  float markerSize = 6.0f*params.sizeRatio;
  for(int i = 0; i < chart->objects().size(); i++)
    {
      if(chart->objects()[i]->type < OBJ_COUNT)
        { // skip angles
          float rAngle = screenAngle(chart, (chart->objects()[i]->angle));
          Vec2f v(cos(rAngle), -sin(rAngle));
          // Vec2f pp = cc + ringRadius*v;
          Vec2f pp = cc + params.objRadius*v;
      
          // draw angle marker
          Vec2f n(v.y, -v.x);
          Vec2f p1 = pp + (level == 0 ? 1.0f : -1.0f)*markerSize*v + markerSize*n;
          Vec2f p2 = pp + (level == 0 ? 1.0f : -1.0f)*markerSize*v - markerSize*n;
          std::vector<Vec2f> points = { p1, pp, p2 };

          draw_list->AddTriangleFilled(points[0], points[1], points[2], ImColor(Vec4f(1.0f, 1.0f, 1.0f, 1.0f)));
          draw_list->AddTriangle(points[0], points[1], points[2], ImColor(Vec4f(0.2f, 0.2f, 0.2f, 1.0f)), 2.0f);
        }
    }
  
  // draw objects
  for(int i = 0; i < chart->objects().size(); i++)
    {
      if(chart->objects()[i]->type < OBJ_COUNT)
        { // skip angles
          std::string oName = getObjName(chart->objects()[i]->type);
          if(!chart->objects()[i]->visible) { continue; }
      
          float rAngle = screenAngle(chart, (chart->objects()[i]->angle));
          Vec2f v(cos(rAngle), -sin(rAngle));

          Vec2f pp = cc + ringRadius*v;
          Vec2f objP = pp + (params.sizeRatio*CHART_OBJRING_W/2.0f)*v - Vec2f(params.symbolSize, params.symbolSize)/2.0f;

          ChartImage *img = getWhiteImage(oName);
          if(img)
            {
              Vec2f imSize = Vec2f(params.symbolSize, params.symbolSize);
              ImGui::SetCursorScreenPos(objP);
              Vec4f color = getObjColor(oName);
              Vec4f intCol(color.x, color.y, color.z, color.w);
              Vec4f borderCol(0.0f, 0.0f, 0.0f, 0.0f);
              
              ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, imSize.x/2.0f);
              ImGui::Image((ImTextureID)img->texId, imSize, t0, t1, ImColor(color), borderCol);
              ImGui::PopStyleVar();
              
              bool hover = ImGui::IsItemHovered();
              if(hover)
                {
                  double oAngle = chart->objects()[i]->angle;
                  int oSign = chart->swe().getSign(oAngle);
                  ChartImage *sImg = getWhiteImage(getSignName(oSign));
                  Vec4f sColor = ELEMENT_COLORS[getSignElement(oSign)];
                  color = Vec4f(sColor.x, sColor.y, sColor.z, sColor.w);

                  double oDegree = oAngle - chart->swe().getSignCusp(oSign);
                  
                  ImGui::BeginTooltip();
                  {
                    ImGui::BeginTable("##tooltip-obj", 3, ImGuiTableFlags_SizingPolicyStretchX);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthAlwaysAutoResize);
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthAlwaysAutoResize);
                    {
                      ImGui::TableNextRow();
                      ImGui::TableSetColumnIndex(0);
                      // object name
                      ImGui::TextUnformatted((oName+"  ").c_str());
                      ImGui::TableNextColumn();
                      // object sign
                      ImGui::Image((ImTextureID)sImg->texId, Vec2f(20.0f, 20.0f), t0, t1, ImColor(color), borderCol);
                      ImGui::TableNextColumn();
                      // object angle
                      ImGui::TextUnformatted(angle_string(oDegree, true, false).c_str());
                    }
                    ImGui::EndTable();

                    if(ImGui::GetIO().KeyAlt) // ALT --> show inside degrees text
                      {
                        // display inside degrees text underneath
                        ImGui::Spacing();
                        int iDegree = (int)std::floor(oDegree);
                        ImGui::Text("%s:%d -- %s", getSignName(oSign).c_str(), iDegree+1, Chart::getInsideDegreeTextShort(oSign, iDegree).c_str());
                        
                        if(ImGui::GetIO().KeyShift) // ALT+SHIFT --> show long text
                          {
                            // display inside degrees text underneath
                            int iDegree = (int)std::floor(oDegree);
                            ImGui::TextWrapped("Explanation: %s", Chart::getInsideDegreeTextLong(oSign, iDegree).c_str());
                          }

                      }                    
                  }
                  ImGui::EndTooltip();

                  mFocusObjects[i] = ImGui::GetIO().KeyShift; // focus when shift key held
                }
              else
                {
                  mFocusObjects[i] = chart->objects()[i]->focused;
                }

              // set focus
              chart->setObjFocus((ObjType)i, mFocusObjects[i]);
              
              
              if(chart->objects()[i]->focused)
                { draw_list->AddNgon(objP + Vec2f(params.symbolSize, params.symbolSize)/2.0f, params.symbolSize*0.75f, ImColor(Vec4f(1.0f, 1.0f, 1.0f, 1.0f)), 8, 4.0f); }
            }
        }
    }
}

//// FULL CHART ////
void ChartView::renderChart(Chart *chart, const Vec2f &chartSize)
{
  Vec2f cp = ImGui::GetCursorPos();
  ViewParams params(ImGui::GetCursorScreenPos(), chartSize);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_ChildBg, Vec4f(0,0,0,1));
  if(mSelected)
    {
      ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
      ImGui::PushStyleColor(ImGuiCol_Border, Vec4f(1,0,0,1));
    }
  ImGui::BeginChild("Chart View", chartSize, true, 0);
  {
    if(mSelected)
      {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
      }
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    if(chart)
      {
        renderZodiac (chart, params, draw_list);
        renderHouses (chart, params, draw_list);
        renderObjects(chart, 0, params, draw_list);
        renderAspects(chart, params, draw_list);
        renderAngles (chart, params, draw_list);
      }
    else
      {
        Chart temp(DateTime::now(), Location(NYSE_LAT, NYSE_LON, NYSE_ALT));
        renderZodiac(&temp, ViewParams(ImGui::GetCursorScreenPos(), chartSize), ImGui::GetWindowDrawList());
      }
  }
  ImGui::EndChild();
  ImGui::SetCursorPos(cp + Vec2f(0, chartSize.y));
}

void ChartView::renderChartCompare(ChartCompare *compare, const Vec2f &chartSize)
{
  Chart *oChart = compare->getOuterChart();
  Chart *iChart = compare->getInnerChart();

  Vec2f cp = ImGui::GetCursorPos();
  
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(0.0f, 0.0f));
  ImGui::PushStyleColor(ImGuiCol_ChildBg, Vec4f(0,0,0,1));
  if(mSelected)
    {
      ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
      ImGui::PushStyleColor(ImGuiCol_Border, Vec4f(1,0,0,1));
    }
  ImGui::BeginChild("Chart View", chartSize, true, 0);
  {
    if(mSelected)
      {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
      }
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    
    if(!iChart && !oChart) // only render zodiac if no input charts
      {
        Chart temp(DateTime::now(), Location(NYSE_LAT, NYSE_LON, NYSE_ALT));
        renderZodiac(&temp, ViewParams(ImGui::GetCursorScreenPos(), chartSize), ImGui::GetWindowDrawList());
      }
    else
      {
        ViewParams params(ImGui::GetCursorScreenPos(), chartSize);
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
            
        renderZodiac ((oChart ? oChart : iChart), params, draw_list);
        renderHouses ((iChart ? iChart : oChart), params, draw_list); // use inner chart houses (?)
        renderAngles ((iChart ? iChart : oChart), params, draw_list); // use inner chart angles (?)
        
        if(iChart && oChart) { renderCompareAspects(compare, params, draw_list); }
        if(oChart)           { renderObjects(oChart, 0, params, draw_list); } // outer ring
        if(iChart)           { renderObjects(iChart, 1, params, draw_list); } // inner ring
      }
  }
  ImGui::EndChild();
  ImGui::SetCursorPos(cp);
}

bool ChartView::draw(Chart *chart, float chartWidth)
{
  renderChart(chart, Vec2f(chartWidth, chartWidth));
  return true;
}

bool ChartView::draw(ChartCompare *compare, float chartWidth)
{
  if(compare)
    { renderChartCompare(compare, Vec2f(chartWidth, chartWidth)); }
  return true;
}

