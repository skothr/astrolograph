#include "nodeList.hpp"

#include "imgui.h"
// #include "imgui_internal.h"
#include "imtools.hpp"
#include "nodeGraph.hpp"
#include "viewSettings.hpp"


NodeList::NodeList(NodeGraph *graph, ViewSettings *viewSettings)
  : mGraph(graph), mViewSettings(viewSettings)
{

}

NodeList::~NodeList()
{

}


float NodeList::getWidth() const
{
  if(mCollapsed)
    {
      Vec2f padding = ImGui::GetStyle().FramePadding;
      Vec2f tabSize = ImGui::CalcTextSize(NODELIST_TITLE);
      std::swap(tabSize.x, tabSize.y);
      tabSize += 2.0f*padding;
      return tabSize.x + padding.x;
    }
  else
    { return mViewSize.x; }
}

void NodeList::draw()
{
  Vec2f padding = ImGui::GetStyle().FramePadding;
  Vec2f spacing = ImGui::GetStyle().ItemInnerSpacing;
  Vec2f tabSize = ImGui::CalcTextSize(NODELIST_TITLE);
  std::swap(tabSize.x, tabSize.y);
  tabSize += 2.0f*padding;

  mViewPos  = ImGui::GetCursorScreenPos();
  mViewSize = Vec2f(ImGui::GetContentRegionMax()) - mViewPos;
  
  std::unordered_map<int, Node*> nodes;
  if(mGraph) { nodes = mGraph->getNodes(); }
  
  std::unordered_map<std::string, int> typeCount;
  for(auto t : NodeGraph::NODE_TYPES) { typeCount.emplace(t.first, 0); }
  
  std::vector<Node*> nodeList;
  nodeList.reserve(nodes.size());
  for(auto n : nodes)
    {
      nodeList.push_back(n.second);
      typeCount[n.second->type()]++;
    }
  std::sort(nodeList.begin(), nodeList.end(), [](Node *n1, Node *n2){ return n1->id() < n2->id(); });
  
  // ImGui::SetNextWindowPos(mViewPos);
  // ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
  // ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  NODELIST_PADDING);
  // ImGui::PushStyleColor(ImGuiCol_ChildBg,  Vec4f(0.1f, 0.1f, 0.1f, 1.0f));
  // ImGuiWindowFlags wFlags = (ImGuiWindowFlags_NoDecoration      |
  //                            ImGuiWindowFlags_NoScrollWithMouse );
  // if(ImGui::BeginChild("##nodeListMain", mViewSize-Vec2f(tabSize.x + 2.0f*spacing.x+1.0f, 0), true, wFlags))
  // {
  ImGui::PushFont(mViewSettings->titleFont);
  float titleOffset = (ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(NODELIST_TITLE).x)/2.0f;
  ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(titleOffset, 0.0f));
  ImGui::TextUnformatted(NODELIST_TITLE);
  ImGui::PopFont();
  ImGui::Separator();
        
  // overall stats
  if(ImGui::TreeNode(("Total: "+std::to_string(nodeList.size())).c_str()))
    {
      ImGui::Indent();
      for(auto t : typeCount) { ImGui::Text("%-18s x%d", t.first.c_str(), t.second); }
      ImGui::Unindent();
      ImGui::TreePop();
    }

  // list of nodes
  if(ImGui::BeginChild("##nodeListList", Vec2f(0,0), true))
    {
      for(const auto &n : nodeList)
        {
          if(ImGui::TreeNode((n->name()+"##"+std::to_string(n->id())).c_str()))
            {
              ImGui::Indent();
              ImGui::Text("ID: %d", n->id());
              ImGui::Text("Type: %s", n->type().c_str());
              ImGui::TreePop();
              ImGui::Unindent();
            }
        }
    }
  ImGui::EndChild();

  // // TODO: generalize vertical tabs
  // ImGui::SameLine();
  // Vec2f p0 = ImGui::GetCursorScreenPos();
  // ImGui::BeginGroup();
  // {
  //   // button colors
  //   Vec4f inactiveColor      = Vec4f(0.2f,  0.2f,  0.2f,  1.0f);
  //   Vec4f hoveredColor       = Vec4f(0.35f, 0.35f, 0.35f, 1.0f);
  //   Vec4f clickedColor       = Vec4f(0.8f,  0.8f,  0.8f,  1.0f);  
  //   Vec4f activeColor        = Vec4f(0.5f,  0.5f,  0.5f,  1.0f);
  //   Vec4f activeHoveredColor = Vec4f(0.65f, 0.65f, 0.65f, 1.0f);
  //   Vec4f activeClickedColor = Vec4f(0.8f,  0.8f,  0.8f,  1.0f);
    
  //   ImGui::PushStyleColor(ImGuiCol_Button,        (!mCollapsed ? activeColor        : inactiveColor));
  //   ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (!mCollapsed ? activeHoveredColor : hoveredColor ));
  //   ImGui::PushStyleColor(ImGuiCol_ButtonActive,  (!mCollapsed ? activeClickedColor : clickedColor ));
  //   if(ImGui::Button("##nodeListTab", tabSize)) { mCollapsed = !mCollapsed; }
  //   AddTextVertical(ImGui::GetWindowDrawList(), NODELIST_TITLE, p0+Vec2f(padding.x, tabSize.y-padding.y), Vec4f(1.0f, 1.0f, 1.0f, 1.0f));
  //   ImGui::PopStyleColor(3);
    
  // }
  // ImGui::EndGroup();
}
