// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Broker Analyzer for C, C++ and C#: http://www.viva64.com

#include <Panes/CommitPane.h>

#include <cinttypes>  // printf zu

CommitPane::CommitPane() = default;
CommitPane::~CommitPane() {
    Unit();
}

bool CommitPane::Init() {
    return true;
}

void CommitPane::Unit() {
}

///////////////////////////////////////////////////////////////////////////////////
//// IMGUI PANE ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool CommitPane::DrawPanes(const uint32_t& /*vCurrentFrame*/, bool* vOpened, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    bool change = false;
    if (vOpened != nullptr && *vOpened) {
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus /* | ImGuiWindowFlags_MenuBar*/;
        if (ImGui::Begin(GetName().c_str(), vOpened, flags)) {
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
            auto win = ImGui::GetCurrentWindowRead();
            if (win->Viewport->Idx != 0)
                flags |= ImGuiWindowFlags_NoResize;  // | ImGuiWindowFlags_NoTitleBar;
            else
                flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus /* | ImGuiWindowFlags_MenuBar*/;
#endif
            m_DrawCommitInfos();
        }

        ImGui::End();
    }
    return change;
}

bool CommitPane::DrawOverlays(const uint32_t& /*vCurrentFrame*/, const ImRect& /*vRect*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool CommitPane::DrawDialogsAndPopups(const uint32_t& /*vCurrentFrame*/, const ImVec2& /*vMaxSize*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

bool CommitPane::DrawWidgets(const uint32_t& /*vCurrentFrame*/, ImGuiContext* vContextPtr, void* /*vUserDatas*/) {
    ImGui::SetCurrentContext(vContextPtr);
    return false;
}

void CommitPane::SelectCommit(const GitCommitWeak& vCommit) {
    m_SelectedCommit = vCommit;
}

void CommitPane::m_DrawCommitInfos() {
    auto ptr = m_SelectedCommit.lock();
    if (ptr != nullptr) {
        ImGui::Header("ID's");
        ImGui::Text("ID long  : %s", ptr->id.c_str());
        ImGui::Text("ID short : %s", ptr->idShort.c_str());
        ImGui::Header("Parent(s)");
        if (ptr->parentIds.empty()) {
            ImGui::Text("-");
        }
        for (const auto& id : ptr->parentIds) {
            ImGui::Text("%s", id.c_str());
        }
        ImGui::Header("Childs(s)");
        if (ptr->childIds.empty()) {
            ImGui::Text("-");
        }
        for (const auto& id : ptr->childIds) {
            ImGui::Text("%s", id.c_str());
        }
        ImGui::Header("Message");
        ImGui::TextWrapped("%s", ptr->msg.c_str());
    }
}
