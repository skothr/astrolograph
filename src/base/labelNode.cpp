#include "labelNode.hpp"
using namespace astro;

#include <imgui.h>
#include <imgui_internal.h>
#include "viewSettings.hpp"
#include "setting.hpp"


LabelNode::LabelNode(const std::string &text)
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Label Node"), mText(text)
{
  mSettings.push_back(new Setting<std::string>("Text",  "text",  &mText));
}

void LabelNode::onDraw()
{
  ImGui::PushFont(getViewSettings()->titleFont);
  if(!mEditing) { ImGui::TextUnformatted(mText.c_str()); }

  bool dclicked = false;
  if(mSelected && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) { mEditing = true; dclicked = true; }
  
  if(mEditing)
    {
      static char buf[256];
      strcpy(buf, mText.c_str());
      Vec2f tSize1 = ImGui::CalcTextSize(mText.c_str());
      Vec2f tSize2 = ImGui::CalcTextSize(buf);
      ImGui::SetNextItemWidth(std::max(tSize1.x, tSize2.x)+20);
      ImGui::SetItemDefaultFocus();
      ImGui::InputText("##textInput", buf, 256);
      if(dclicked) { ImGui::SetKeyboardFocusHere(-1); }
      mText = buf;
      if(!mSelected || ImGui::IsKeyPressed(GLFW_KEY_ENTER)) { mEditing = false; }
    }
  ImGui::PopFont();
}
