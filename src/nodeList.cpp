#include "nodeList.hpp"
using namespace astro;

#include "imgui.h"

#include "nodeGraph.hpp"


NodeList::NodeList(NodeGraph *graph)
  : mGraph(graph)
{

}

NodeList::~NodeList()
{

}

#define LIST_PADDING Vec2f(10,10)
void NodeList::draw()
{
  const std::unordered_map<int, Node*> &nodes = mGraph->getNodes();
  ViewSettings *viewSettings = mGraph->getViewSettings();
  
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
  
  ImGui::SetNextWindowPos(mViewPos);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,  0);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  LIST_PADDING);
  ImGui::PushStyleColor(ImGuiCol_ChildBg,  Vec4f(0.1f, 0.1f, 0.1f, 1.0f));
  ImGui::BeginChild("nodeListMain", mViewSize, true, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse);
  {
    ImGui::PushFont(viewSettings->titleFont);
    float titleOffset = (ImGui::GetContentRegionMax().x - ImGui::CalcTextSize("Nodes").x)/2.0f;
    ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(titleOffset, 0.0f));
    ImGui::TextUnformatted("Nodes");
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
    ImGui::BeginChild("nodeListList", Vec2f(0,0), true);
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
  }
  ImGui::EndChild();
  ImGui::PopStyleVar(2);
  ImGui::PopStyleColor();
}
