#include "progressNode.hpp"
using namespace astro;

#include "glfwKeys.hpp"
#include "imgui.h"
#include "tools.hpp"


ProgressNode::ProgressNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Progress Node"), mChart(new Chart(DateTime(), Location()))
{
  outputs()[PROGRESSNODE_OUTPUT_CHART]->set(mChart);
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

void ProgressNode::onUpdate()
{
  Chart *inChart = inputs()[PROGRESSNODE_INPUT_CHART]->get<Chart>();
  if(inChart)
    {
      DateTime dtOrig  = inChart->date();      // original date
      Location locOrig = inChart->location(); // original location
      DateTime *dtIn   = inputs()[PROGRESSNODE_INPUT_DATE]->get<DateTime>();
      Location *locIn  = inputs()[PROGRESSNODE_INPUT_LOCATION]->get<Location>();
      DateTime dtComp  = (dtIn  ? *dtIn  : dtOrig);   // compare date
      Location locComp = (locIn ? *locIn : locOrig);  // compare location
      
      DateTime dtProg = mChart->swe().getProgressed(dtOrig, locOrig, dtComp, locComp);
      DateTime dtUnprog = mChart->swe().getUnprogressed(dtOrig, locOrig, mChart->date(), mChart->location());
      
      if(dtIn && mChart->date() != dtProg)
        {
          if(mChart->changed()) { *dtIn = dtUnprog; }
          else                  { mChart->setDate(dtProg); }
        }
      if(locIn && *locIn != mChart->location())
        {
          if(mChart->changed()) { *locIn = mChart->location(); }
          else                  { mChart->setLocation(*locIn); }
        }
      mChart->update();
    }
}

bool ProgressNode::onDraw()
{
  Chart *inChart = inputs()[PROGRESSNODE_INPUT_CHART]->get<Chart>();
  if(inChart)
    {
      DateTime dtOrig = inChart->date();      // original date
      Location locOrig = inChart->location(); // original location
        
      // DateTime *dtIn   = inputs()[PROGRESSNODE_INPUT_DATE]->get<DateTime>();
      // Location *locIn  = inputs()[PROGRESSNODE_INPUT_LOCATION]->get<Location>();
      // DateTime dtComp  = (dtIn  ? *dtIn  : dtOrig);   // compare date
      // Location locComp = (locIn ? *locIn : locOrig);  // compare location

      // DateTime dtProg = mChart->date();//mChart->swe().getProgressed(dtOrig, locOrig, dtComp, locComp);

      ImGui::Text("Original:   %s", dtOrig.toString().c_str());
      ImGui::Text("            %s", locOrig.toString().c_str());
      ImGui::Separator();
      ImGui::Text("Progressed: %s", mChart->date().toString().c_str());
      ImGui::Text("            %s", mChart->location().toString().c_str());

      // if(mChart->date() != dtProg)      { mChart->setDate(dtProg); }
      // if(mChart->location() != locComp) { mChart->setLocation(locComp); }
      //mChart->update();
    }
  // mChart->update();
  return true;
}

