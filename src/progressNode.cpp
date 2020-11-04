#include "progressNode.hpp"
using namespace astro;

#include <GLFW/glfw3.h>
#include "imgui.h"
#include "tools.hpp"


ProgressNode::ProgressNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Progress Node"), mChart(new Chart(DateTime(), Location()))
{
  // mChart->setProgressed(true);
  outputs()[PROGRESSNODE_OUTPUT_CHART]->set(mChart);
  
  mChart->update();
  for(auto &obj : mChart->objects())
    {
      if(obj->type < OBJ_COUNT) // planets/asateroids/etc
        { mChart->showObject((ObjType)obj->type, true); }
      else                      // angles
        { mChart->showObject((ObjType)obj->type, false); }
    }
}

ProgressNode::~ProgressNode()
{
  delete mChart;
}


bool ProgressNode::onDraw()
{
  ImGui::BeginGroup();
  {
    Chart *inChart = inputs()[PROGRESSNODE_INPUT_CHART]->get<Chart>();
    
    if(inChart)
      {
        DateTime dtOrig = inChart->date();      // original date
        Location locOrig = inChart->location(); // original location
        
        DateTime *dtIn  = inputs()[PROGRESSNODE_INPUT_DATE]->get<DateTime>();
        Location *locIn = inputs()[PROGRESSNODE_INPUT_LOCATION]->get<Location>();
        DateTime dtComp = dtOrig;   // compare date
        Location locComp = locOrig; // compare location
        if(dtIn)  { dtComp = *dtIn; }
        if(locIn) { locComp = *locIn; }

        DateTime dtProg = inChart->swe().getProgressed(dtOrig, locOrig, dtComp, locComp);

        ImGui::TextUnformatted(dtComp.toString().c_str());
        ImGui::Text(" -->  %s", dtProg.toString().c_str());
        ImGui::Text(locComp.toString().c_str());
        ImGui::Spacing();

        if(mChart->date() != dtProg)      { mChart->setDate(dtProg); }
        if(mChart->location() != locComp) { mChart->setLocation(locComp); }
        mChart->update();
      }
  }
  ImGui::EndGroup();
  
  mChart->update();
  
  return true;
}

