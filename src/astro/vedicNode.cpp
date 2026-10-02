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
  mMoonData = (chart ? chart->getObjectData(OBJ_MOON) : nullptr);
}

#define NAKSHATRA_WIDTH (40.0 / 3.0)
void VedicNode::onDraw()
{
  const float scale = getScale();
  
  Chart *chart = inputs()[VEDICNODE_INPUT_CHART]->get<Chart>();
  if(chart && mMoonData && mMoonData->valid)
    {
      // calculate moon nakshatra
      const int nakshatraId = (int)std::floor(mMoonData->longitude / NAKSHATRA_WIDTH);
      const double nakshatraDeg = mMoonData->longitude - nakshatraId*NAKSHATRA_WIDTH;
      const std::string nakshatraName = getNakshatraNameLong(nakshatraId);
      ImGui::Text("Moon Nakshatra: %s (%f°)", nakshatraName.c_str(), nakshatraDeg);
      ImGui::Text("Sidereal time:  %f", chart->getSiderealTime());

      const DateTime dt  = chart->date();
      const Location loc = chart->location();
      const double jdET  = chart->swe().getJulianDayET(dt, loc);
      const double jdUT  = chart->swe().getJulianDayUT(dt, loc);

      // calculate mahadasha periods in years
      std::array<double, 9> dashaYears;
      const double nakshatraRatio = (nakshatraDeg / NAKSHATRA_WIDTH); // natal moon ratio of first nakshatra
      dashaYears[0] = (1.0f - nakshatraRatio)*getDashaYears(nakshatraId);
      for(int i = 1; i < 9; i++) { dashaYears[i] = getDashaYears((nakshatraId + i) % 9); }
          
      // calculate ratios of the full cycle (120 years)
      std::array<double, 9> dashaRatios;
      for(int i = 0; i < 9; i++) { dashaRatios[i] = dashaYears[i] / 120.0; }

      ImGui::Separator();
      ImGui::TextUnformatted("Mahadashas:");
      ImGui::Indent();
      const double jdStart = jdET;
      double jdTotal = jdStart;
      for(int i = 0; i < 9; i++)
        {
          const DateTime dtStart  = chart->swe().getDateFromJUT(jdTotal, chart->location());
          const double   ageStart = (int)((jdTotal - jdStart) / DAYS_PER_JULIAN_YEAR);
          jdTotal += dashaYears[i]*DAYS_PER_JULIAN_YEAR;
          const DateTime dtEnd   = chart->swe().getDateFromJUT(jdTotal, chart->location());
          const double   ageEnd  = (int)((jdTotal - jdStart) / DAYS_PER_JULIAN_YEAR);
          const std::string rulerName = getNakshatraRulerNameLong((nakshatraId + i) % 9);
              
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
