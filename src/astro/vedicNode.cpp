#include "vedicNode.hpp"
using namespace astro;

#include "imgui.h"

VedicNode::VedicNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Vedic Node")
{ }
VedicNode::~VedicNode()
{ }


void VedicNode::onUpdate()
{
  Chart *chart = inputs()[VEDICNODE_INPUT_CHART]->get<Chart>();
  if(chart && !mMoonData) { mMoonData = chart->getObjectData(OBJ_MOON); }
  else if(!chart)         { mMoonData = nullptr; }
}

#define NAKSHATRA_WIDTH (13.0 + 1.0/3.0)
void VedicNode::onDraw()
{
  float scale = getScale();
  
  Chart *chart = inputs()[VEDICNODE_INPUT_CHART]->get<Chart>();
  if(chart && mMoonData && mMoonData->valid)
    {
      // calculate moon nakshatra
      int nakshatraId = (int)std::floor(mMoonData->longitude / NAKSHATRA_WIDTH);
      double nakshatraDeg = mMoonData->longitude - nakshatraId*NAKSHATRA_WIDTH;
      std::string nakshatraName = getNakshatraNameLong(nakshatraId);
      ImGui::Text("Moon Nakshatra: %s (%f°)", nakshatraName.c_str(), nakshatraDeg);
      ImGui::Text("Sidereal time:  %f", chart->getSiderealTime());

      DateTime dt  = chart->date();
      Location loc = chart->location();
      double jdET  = chart->swe().getJulianDayET(dt, loc);
      double jdUT  = chart->swe().getJulianDayUT(dt, loc);

      // calculate mahadasha periods in years
      std::array<double, 9> dashaYears;
      double nakshatraRatio = (nakshatraDeg / (13.0+1.0/3.0)); // natal moon ratio of first nakshatra
      dashaYears[0] = (1.0f - nakshatraRatio)*getDashaYears(nakshatraId);
      for(int i = 1; i < 9; i++) { dashaYears[i] = getDashaYears((nakshatraId + i) % 9); }
          
      // calculate ratios of the full cycle (120 years)
      std::array<double, 9> dashaRatios;
      for(int i = 0; i < 9; i++) { dashaRatios[i] = dashaYears[i] / 120.0; }

      ImGui::Separator();
      ImGui::TextUnformatted("Mahadashas:");
      ImGui::Indent();
      double jdStart = jdET;
      double jdTotal = jdStart;
      for(int i = 0; i < 9; i++)
        {
          DateTime dtStart  = chart->swe().getDateFromJUT(jdTotal, chart->location());
          double   ageStart = (int)((jdTotal - jdStart) / DAYS_PER_JULIAN_YEAR);
          jdTotal += dashaYears[i]*DAYS_PER_JULIAN_YEAR;
          DateTime dtEnd   = chart->swe().getDateFromJUT(jdTotal, chart->location());
          double   ageEnd  = (int)((jdTotal - jdStart) / DAYS_PER_JULIAN_YEAR);
          std::string rulerName = getNakshatraRulerNameLong((nakshatraId+i)%9);
              
          std::stringstream ss;
          ss << (i+1) << ". " << std::left << std::setw(8) << rulerName << " -->  "
             << std::right << std::setw(7) << std::fixed << std::setprecision(4) << dashaYears[i] << " years ("
             << std::setfill('0') << std::setw(2) << dtStart.month() << "/"
             << std::setfill('0') << std::setw(2) << dtStart.day()   << "/"
             << std::setw(4) << dtStart.year() << "-"
             << std::setfill('0') << std::setw(2) << dtEnd.month()   << "/"
             << std::setfill('0') << std::setw(2) << dtEnd.day()     << "/"
             << std::setw(4) << dtEnd.year() << "  |  Age " << std::setfill(' ')
             << std::setw(3) << (int)ageStart << "-" << std::setw(3) << (int)ageEnd << ")";
          ImGui::Text("%s", ss.str().c_str());
        }
      ImGui::Unindent();
    }
}
