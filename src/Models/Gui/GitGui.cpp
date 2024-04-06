#include <Models/Gui/GitGui.h>

#include <Backend/MainBackend.h>
#include <ctools/FileHelper.h>

#include <Models/Git/GitBranch.h>

#include <git2.h>

bool GitGui::init() {
    git_libgit2_init();
    return true;
}

void GitGui::unit() {
    m_Repositeries.clear();
    git_libgit2_shutdown();
}

bool GitGui::openLocalRepositery(const RepoPathName& vRepoPathName) {
    auto ps = FileHelper::Instance()->ParsePathFileName(vRepoPathName);
    if (ps.isOk) {
        if (!setCurrentRepo(ps.name)) {  // if not already exist
            auto ptr = GitRepositery::create(vRepoPathName);
            if (ptr != nullptr) {
                m_Repositeries[ps.name] = ptr;
                setCurrentRepo(ps.name);  // now set freshly opened repo as current
                return true;
            }
        }
    }
    return false;
}

bool GitGui::closeRepositery(const RepoName& vRepoName) {
    if (isRepoAlreadyOpened(vRepoName)) {
        m_Repositeries.erase(vRepoName);
        return true;
    }
    return false;
}

bool GitGui::closeRepositery(const GitRepositeryWeak& vRepo) {
    auto ptr = vRepo.lock();
    if (ptr != nullptr) {
        return closeRepositery(ptr->GetRepoName());
    }
    return false;
}

bool GitGui::isRepoAlreadyOpened(const RepoName& vRepoName) {
    return (m_Repositeries.find(vRepoName) != m_Repositeries.end());
}

bool GitGui::setCurrentRepo(const RepoName& vRepoName) {
    if (isRepoAlreadyOpened(vRepoName)) {
        m_CurrentRepo = m_Repositeries.at(vRepoName);
        return true;
    }
    return false;
}

bool GitGui::drawRibbon(const float& vHeight) {
    bool change = false;
    const auto viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, vHeight));
    if (ImGui::Begin("##Ribbon", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration)) {
        if (ImGui::BeginTabBar("##RibbonTabBar", ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_FittingPolicyResizeDown | ImGuiTabBarFlags_Reorderable)) {
            auto currentPtr = m_CurrentRepo.lock();
            for (auto& repo : m_Repositeries) {
                if (repo.second != nullptr) {
                    bool opened = true;
                    if (ImGui::BeginTabItem(repo.second->GetRepoName().c_str(), &opened)) {
                        if (!opened) {  // the user was clicking on the clsoe button
                            m_RepoToCloseDeferred = repo.second;
                        }
                        m_CurrentRepo = repo.second; // set current repo

                        ImGui::EndTabItem();
                    }
                }
            }
            if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip)) {
                ImGuiFileDialog::Instance()->OpenDialog("OpenNewRepositery", "Open a local git repositery", nullptr);
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();

    // close the current repositery in deferred as requested by the user
    if (!m_RepoToCloseDeferred.expired()) {
        closeRepositery(m_RepoToCloseDeferred);
        m_RepoToCloseDeferred.reset();
    }

    return change;
}

bool GitGui::drawDialogs() {
    const auto& displaySize = MainBackend::Instance()->GetDisplaySize();
    if (ImGuiFileDialog::Instance()->Display("OpenNewRepositery", ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking, displaySize * 0.5f, displaySize)) {
        if (ImGuiFileDialog::Instance()->IsOk()) {
            openLocalRepositery(ImGuiFileDialog::Instance()->GetCurrentPath());
        }
        ImGuiFileDialog::Instance()->Close();
        return true;
    }
    return false;
}

bool GitGui::m_drawGraphNode(const float& vLastPosX, GitRepositeryPtr vRepositery, GitCommitPtr vCommitPtr, const float& vTextLineHeight) {
    assert(vRepositery != nullptr);
    assert(vCommitPtr != nullptr);
    const auto& margin = 10.0f;
    const auto& win = ImGui::GetCurrentWindow();
    const auto& drawlist = win->DrawList;
    const auto& colWidth = ImGui::GetColumnWidth();
    const auto& branchColor = vRepositery->GetBranchColorFromIndex(vCommitPtr->graphColumn);
    const auto& finalCol = ImGui::GetColorU32(ImVec4(branchColor.x, branchColor.y, branchColor.z, 1.0f));
    drawlist->AddRectFilled(ImVec2(vLastPosX, win->DC.CursorPos.y + ImGui::GetTextLineHeight() * 0.5f - 1.0f),
                            win->DC.CursorPos + ImVec2(margin + 10.0f * (vCommitPtr->graphColumn + 1U), ImGui::GetTextLineHeight() * 0.5f + 1.0f),
                            finalCol);
    const auto& lastCommits = vRepositery->GetLastCommitsPerBranch();
    for (size_t i = 0U; i <= vCommitPtr->maxGraphColumn; i++) {
        const auto& branchColor = vRepositery->GetBranchColorFromIndex(i);
        const auto& bgCol = ImGui::GetColorU32(ImVec4(branchColor.x, branchColor.y, branchColor.z, 1.0f));
        const auto& offset = margin + 10.0f * i;
        const auto& pos = win->DC.CursorPos + ImVec2(offset, 0.0f);
        const auto& lastCommit = lastCommits->at(vCommitPtr->branchPtrId);
        if (i == vCommitPtr->graphColumn && vCommitPtr->isNewbranch) {  // this node graph column
            drawlist->AddRectFilled(pos + ImVec2(10.0f, 0.0f), pos + ImVec2(12.0f, ImGui::GetTextLineHeight() * 0.5f), bgCol);
        } else if (i == vCommitPtr->graphColumn && vCommitPtr->isLastCommit) {  // last commit
            drawlist->AddRectFilled(pos + ImVec2(10.0f, ImGui::GetTextLineHeight() * 0.5f), pos + ImVec2(12.0f, vTextLineHeight), bgCol);
        } else if (vCommitPtr->isRoot) {  // first commit
            drawlist->AddRectFilled(pos + ImVec2(10.0f, 0.0f), pos + ImVec2(12.0f, ImGui::GetTextLineHeight() * 0.5f), bgCol);
        } else if (vCommitPtr->dateEpoch < lastCommit->dateEpoch) {  // others
            drawlist->AddRectFilled(pos + ImVec2(10.0f, 0.0f), pos + ImVec2(12.0f, vTextLineHeight), bgCol);
        }
        if (i == vCommitPtr->graphColumn) {
            drawlist->AddCircleFilled(pos + ImVec2(11.0f, ImGui::GetTextLineHeight() * 0.5f), 5.0f, bgCol, 12);
        }
    }
    return false;
}

bool GitGui::drawHistory() {
    auto ptr = m_CurrentRepo.lock();
    if (ptr == nullptr) {
        return false;
    }
    const auto commits_ptr = ptr->GetHistory();
    if (commits_ptr != nullptr) {
        if (commits_ptr->empty()) {
            return false;
        }
        static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable |
            ImGuiTableFlags_ScrollY | ImGuiTableFlags_NoHostExtendY;
        if (ImGui::BeginTable("##gitTable", 6, flags)) {
            ImGui::TableSetupScrollFreeze(0, 1);  // Make header always visible
            ImGui::TableSetupColumn("Branch", ImGuiTableColumnFlags_WidthFixed, -1, 0);
            ImGui::TableSetupColumn("Graph", ImGuiTableColumnFlags_WidthFixed, -1, 1);
            ImGui::TableSetupColumn("Id", ImGuiTableColumnFlags_WidthFixed, -1, 2);
            ImGui::TableSetupColumn("Messages", ImGuiTableColumnFlags_WidthStretch, -1, 3);
            ImGui::TableSetupColumn("Author", ImGuiTableColumnFlags_WidthFixed, -1, 4);
            ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthFixed, -1, 5);
            ImGui::TableHeadersRow();
            const auto& textLineHeight = ImGui::GetTextLineHeightWithSpacing();
            m_HistoryClipper.Begin((int)commits_ptr->size(), textLineHeight);
            while (m_HistoryClipper.Step()) {
                for (int i = m_HistoryClipper.DisplayStart; i < m_HistoryClipper.DisplayEnd; i++) {
                    if (i < 0) {
                        continue;
                    }
                    const auto infos = commits_ptr->at(i);
                    if (!infos.use_count()) {
                        continue;
                    }
                    if (infos->branch.use_count()) {
                        ImGui::TableNextRow();

                        bool selected = false;

                        float _branchEndPosX = 0.0f;
                        if (ImGui::TableNextColumn()) {  // branch
                            const auto& bgCol = ImGui::GetColorU32(ImVec4(infos->branch->color.x, infos->branch->color.y, infos->branch->color.z, 1.0f));
                            // https://stackoverflow.com/questions/58044749/how-to-right-align-text-in-imgui-columns
                            const auto& textSize = ImGui::CalcTextSize(infos->branch->name.c_str());
                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - textSize.x - ImGui::GetScrollX() - ImGui::GetStyle().ItemSpacing.x);
                            const auto& win = ImGui::GetCurrentWindow();
                            const auto& pos = win->DC.CursorPos;
                            const auto& drawlist = win->DrawList;
                            drawlist->AddRectFilled(
                                pos - ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.0f), pos + textSize + ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.0f), bgCol, 5.0f);
                            _branchEndPosX = pos.x + textSize.x + ImGui::GetStyle().ItemSpacing.x;
                            const bool pushed =
                                ImGui::PushStyleColorWithContrast4(bgCol, ImGuiCol_Text, ImGui::CustomStyle::puContrastedTextColor, ImGui::CustomStyle::puContrastRatio);
                            ImGui::Text("%s", infos->branch->name.c_str());
                            if (pushed)
                                ImGui::PopStyleColor();
                        }
                        if (ImGui::TableNextColumn()) {  // graph
                            m_drawGraphNode(_branchEndPosX, ptr, infos, textLineHeight);
                        }
                        if (ImGui::TableNextColumn()) {  // id
                            ImGui::Text("%s", infos->idShort.c_str());
                        }
                        if (ImGui::TableNextColumn()) {  // Message
                            ImGui::Selectable(infos->msgOneLine.c_str(), &selected, ImGuiSelectableFlags_SpanAllColumns);
                        }
                        if (ImGui::TableNextColumn()) {  // Author
                            ImGui::Text("%s", infos->authorName.c_str());
                        }
                        if (ImGui::TableNextColumn()) {  // Date
                            ImGui::Text("%s", infos->date.c_str());
                        }
                    }
                }
            }
            m_HistoryClipper.End();
            ImGui::EndTable();
        }
    }
}

std::string GitGui::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string res;
    res += vOffset + "<repositories>\n";
    for (const auto& repo : m_Repositeries) {
        if (repo.second != nullptr) {
            res += vOffset + ct::toStr("\t<repositorie path=\"%s\"/>", repo.second->GetRepoPathName().c_str());
        }
    }
    res += vOffset + "</repositories>\n";
    return res;
}

// return true for continue xml parsing of childs in this node or false for interrupt the child exploration (if we want explore child ourselves)
bool GitGui::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != nullptr)
        strParentName = vParent->Value();

    if (strParentName == "repositories") {
        if (strName == "repositorie") {
            std::string path;
            for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next()) {
                std::string attName = attr->Name();
                std::string attValue = attr->Value();
                if (attName == "path") {
                    path = attValue;
                }
            }
            openLocalRepositery(path);
        }
    }

    return true;
}
