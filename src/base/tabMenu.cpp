#include "tabMenu.hpp"

#include <imgui.h>
#include "imtools.hpp"


TabMenu::TabMenu(int width, int length, bool vertical, bool collapsible, bool flipMenu)
  : mWidth(width), mLength(length), mVertical(vertical), mCollapsible(collapsible)
{ }

Vec2f TabMenu::getSize() const
{
  float width  = getBarWidth() + (mSelected >= 0 ? (mTabs[mSelected].width + TABMENU_MENU_PADDING) : 0);
  float length = mLength;
  return (mVertical ? Vec2f(width, length) : Vec2f(length, width));
}

float TabMenu::getBarWidth() const
{
  if(mWidth == 0.0f)
    {
      float barW = 0.0f;
      for(auto &tab : mTabs)
        { barW = std::max(barW, ImGui::CalcTextSize(tab.label.c_str()).y); }
      return barW + 2.0f*TABMENU_TAB_PADDING;
    }
  else { return mWidth; }
}

float TabMenu::getTabLength() const
{
  float tabL = 0.0f;
  for(auto &tab : mTabs)
    { tabL = std::max(tabL, ImGui::CalcTextSize(tab.label.c_str()).x); }
  return tabL + 2.0f*TABMENU_TAB_PADDING;
}

void TabMenu::setWidth(int width)   { mWidth  = width;  }
void TabMenu::setLength(int length) { mLength = length; }
void TabMenu::setVertical(bool vertical, bool flipMenu) { mVertical = vertical; mFlipSide = flipMenu; }
void TabMenu::setCollapsible(bool collapsible)          { mCollapsible = collapsible; }

int TabMenu::add(TabDesc desc)
{
  int index = mTabs.size();
  mTabs.push_back(desc);
  return index;
}

// remove by label
void TabMenu::remove(const std::string &label)
{
  for(int i = 0; i < mTabs.size(); i++)
    {
      if(mTabs[i].label == label)
        { mTabs.erase(mTabs.begin() + i); break; }
    }
}
// remove by index
void TabMenu::remove(int index)
{
  if(mTabs.size() > index) { mTabs.erase(mTabs.begin() + index); }
}

void TabMenu::select(int index) { if(index >= 0 || mCollapsible) { mSelected = ((mCollapsible && index == mSelected) ? -1 : index); } }
void TabMenu::collapse()        { select(-1); }

void TabMenu::draw()
{
  Vec4f inactiveColor      = Vec4f(0.2f,  0.2f,  0.2f,  1.0f);
  Vec4f hoveredColor       = Vec4f(0.35f, 0.35f, 0.35f, 1.0f);
  Vec4f clickedColor       = Vec4f(0.8f,  0.8f,  0.8f,  1.0f);  
  Vec4f activeColor        = Vec4f(0.5f,  0.5f,  0.5f,  1.0f);
  Vec4f activeHoveredColor = Vec4f(0.65f, 0.65f, 0.65f, 1.0f);
  Vec4f activeClickedColor = Vec4f(0.8f,  0.8f,  0.8f,  1.0f);

  float barW = getBarWidth();
  float tabL = getTabLength();
  Vec2f spacing = ImGui::GetStyle().ItemInnerSpacing;
  ImDrawList *drawList = ImGui::GetWindowDrawList();
  
  Vec2f p0  = ImGui::GetCursorPos();
  Vec2f sp0 = ImGui::GetCursorScreenPos();
  ImGui::BeginGroup();
  {    
    // draw tabs
    for(int i = 0; i < mTabs.size(); i++)
      {
        const TabDesc &tab = mTabs[i];
        Vec2f textSize = ImGui::CalcTextSize(tab.label.c_str());
        if(mVertical) { std::swap(textSize.x, textSize.y); }
        Vec2f tabSize = (mVertical ? Vec2f(barW, tabL) : Vec2f(tabL, barW));
        Vec2f padding = (tabSize - textSize)/2.0f;
        
        Vec2f p  = ImGui::GetCursorPos();
        Vec2f sp = ImGui::GetCursorScreenPos();
        
        ImGui::PushStyleColor(ImGuiCol_Button,        (mSelected == i ? activeColor        : inactiveColor));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (mSelected == i ? activeHoveredColor : hoveredColor ));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  (mSelected == i ? activeClickedColor : clickedColor ));
        if(ImGui::Button(((mVertical ? "" : tab.label) + "##tab" + std::to_string(i)).c_str(), tabSize)) { select(i); }
        ImGui::PopStyleColor(3);
        Vec2f np = ImGui::GetCursorPos();
        
        if(mVertical)
          { AddTextVertical(drawList, tab.label.c_str(), sp + padding, Vec4f(1.0f, 1.0f, 1.0f, 1.0f)); }
        else
          { ImGui::SameLine(); }

        if(i == mSelected)
          {
            Vec2f menuOffset = (mFlipSide ? -Vec2f(0.0f, tab.width+TABMENU_MENU_PADDING) : Vec2f(0.0f, getBarWidth()+TABMENU_MENU_PADDING));
            if(mVertical) { std::swap(menuOffset.x, menuOffset.y); }
            ImGui::SetCursorPos(p0 + menuOffset);
            
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
            ImGui::PushStyleColor(ImGuiCol_ChildBg,  Vec4f(0.1f, 0.1f, 0.1f, 1.0f));
            ImGui::BeginChild("##tabMenuChild", (mVertical ? Vec2f(tab.width, mLength) : Vec2f(mLength, tab.width)), true);
            tab.drawMenu();
            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            ImGui::SetCursorPos(np);
          }
      }

    if(!mFlipSide) { mTabs[mSelected].drawMenu(); } // draw open menu to right or bottom of tab bar
  }
  ImGui::EndGroup();
}
