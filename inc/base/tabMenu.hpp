#ifndef TAB_MENU_HPP
#define TAB_MENU_HPP

#include <vector>
#include <string>
#include <functional>
#include "vector.hpp"

#define TABMENU_TAB_PADDING  6.0f
#define TABMENU_MENU_PADDING 5.0f

typedef std::function<void()> TabCallback;

struct TabDesc
{
  std::string label;    // tab label
  TabCallback drawMenu; // draw callback
  int         width;    // width of open menu
};

class TabMenu
{
private:
  std::vector<TabDesc> mTabs;
  int mSelected = 0;

  float mLength = 0.0f; // direction of text/tabs
  float mWidth  = 0.0f; // thickness of bar
  bool  mVertical     = false;
  bool  mFlipSide     = false; // if true, horizontal menus open on top and vertical menus open on left
  bool  mCollapsible  = false;
  
public:
  TabMenu() { }
  TabMenu(int width, int length, bool vertical=false, bool collapsible=true, bool flipMenu=false);
  
  Vec2f getSize()     const;
  float getBarWidth() const;
  float getTabLength() const;

  void setWidth(int width);
  void setLength(int length);
  void setVertical(bool vertical=true, bool flipMenu=false);
  void setCollapsible(bool collapsible=true);
  
  int add(TabDesc desc);
  void remove(const std::string &label);
  void remove(int index);
  
  void select(int index); // -1 or index of open tab will collapse bar
  void collapse();
  void draw();
};


#endif //TAB_MENU_HPP
