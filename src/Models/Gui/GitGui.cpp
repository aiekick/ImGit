#include <Models/Gui/GitGui.h>

#include <Backend/MainBackend.h>
#include <ctools/FileHelper.h>
#include <Models/Git/GitRepositery.h>
#include <Models/Git/GitCommit.h>

#include <Models/Git/GitBranch.h>
#include <Res/fontIcons.h>

#include <Panes/CommitPane.h>

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

bool GitGui::drawObjects() {
    auto ptr = m_CurrentRepo.lock();
    if (ptr != nullptr) {
        const auto& branchs = ptr->GetBranchsRef();
        auto target_count = (uint32_t)branchs.size();

        // LOCAL Branchs
        auto local_branchs = branchs.find(GIT_LOCAL_TARGET);
        if (local_branchs != branchs.end()) {
            --target_count; // is used for remotes
            if (ImGui::TreeNode("LOCAL_BRANCHS", "Locals (%u)", (uint32_t)local_branchs->second.size())) {
                ImGui::Indent();
                auto head_branch_ptr = ptr->GetHeadBranch().lock().get();
                for (const auto& branch : local_branchs->second) {
                    auto str = ct::toStr("%s %s", branch.first.c_str(), branch.second.get() == head_branch_ptr ? ICON_IMGIT_STAR : "");
                    if (ImGui::Selectable(str.c_str())) {
                        CTOOL_DEBUG_BREAK;
                    }
                }
                ImGui::Unindent();
                ImGui::TreePop();
            }
        }
        
        auto tags_branchs = branchs.find(GIT_TAG_TARGET);
        if (tags_branchs != branchs.end()) {
            --target_count;  // is used for remotes

        }

        // REMOTES Branchs
        if (ImGui::TreeNode("REMOTE_BRANCHS", "Remotes (%u)", target_count)) {
            for (const auto& type : branchs) {
                if (type.first == GIT_LOCAL_TARGET || type.first == GIT_TAG_TARGET) {
                    continue;
                }
                if (ImGui::TreeNode(&type.first, "%s (%u)", type.first.c_str(), (uint32_t)type.second.size())) {
                    ImGui::Indent();
                    for (const auto& branch : type.second) {
                        if (ImGui::Selectable(branch.first.c_str())) {
                            CTOOL_DEBUG_BREAK;
                        }
                    }
                    ImGui::Unindent();
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        // TAGS
        if (tags_branchs != branchs.end()) {
            if (ImGui::TreeNode("TAGS_BRANCHS", "Tags (%u)", (uint32_t)tags_branchs->second.size())) {
                ImGui::Indent();
                for (const auto& tag : tags_branchs->second) {
                    if (ImGui::Selectable(tag.first.c_str())) {
                        CTOOL_DEBUG_BREAK;
                    }
                }
                ImGui::Unindent();
                ImGui::TreePop();
            }
        }
    }
    return false;
}

bool GitGui::m_drawGraphNode(const float& vLastPosX, GitRepositeryPtr vRepositery, GitCommitPtr vCurrentCommitPtr, GitCommitPtr vLastCommitPtr, const float& vTextLineHeight) {
    assert(vRepositery != nullptr);
    assert(vCurrentCommitPtr != nullptr);
    const auto& startMargin = 5.0f;
    const auto& margin = 10.0f;
    const auto& win = ImGui::GetCurrentWindow();
    const auto& drawlist = win->DrawList;
    const auto& branchColor = vRepositery->GetBranchColorFromIndex(vCurrentCommitPtr->startNodeColumn);
    const auto& finalCol = ImGui::GetColorU32(ImVec4(branchColor.x, branchColor.y, branchColor.z, 1.0f));
    const auto& lineThickness = 1.5f;
    const auto& circleRadius = 3.0f;
    drawlist->AddLine(ImVec2(vLastPosX, win->DC.CursorPos.y + ImGui::GetTextLineHeight() * 0.5f),
                      win->DC.CursorPos + ImVec2(startMargin + margin * vCurrentCommitPtr->startNodeColumn, ImGui::GetTextLineHeight() * 0.5f),
                      finalCol,
                      lineThickness);

    ImVec2 lastPos;
    ImVec2 p0,p1;
    for (size_t idx = 0; idx < vCurrentCommitPtr->nodeColumns.size(); ++idx) {
        const auto& nodeType = vCurrentCommitPtr->nodeColumns.at(idx);
        if (nodeType != GitGraphNodeEnum::NONE) {
            const auto& branchColor = vRepositery->GetBranchColorFromIndex(idx);
            const auto& bgCol = ImGui::GetColorU32(ImVec4(branchColor.x, branchColor.y, branchColor.z, 1.0f));
            const auto& offset = startMargin + margin * idx;
            const auto& pos = win->DC.CursorPos + ImVec2(offset, 0.0f);
            const auto& centerPos = pos + ImVec2(0.0f, ImGui::GetTextLineHeight() * 0.5f);
            const auto& topPos = pos + ImVec2(0.0f, 0.0f);
            const auto& bottomPos = pos + ImVec2(0.0f, vTextLineHeight);
            const auto& leftPos = lastPos + ImVec2(0.0f, ImGui::GetTextLineHeight() * 0.5f);

            switch (nodeType) {
                case GitGraphNodeEnum::TRAVERSAL_NO_NODE:  // from bottom to top without node
                    //drawlist->AddRectFilled(pos + ImVec2(margin, 0.0f), pos + ImVec2(12.0f, vTextLineHeight), bgCol);
                    break;
                case GitGraphNodeEnum::TRAVERSAL_NODE:  // from bottom to top with node
                    drawlist->AddLine(bottomPos, topPos, bgCol, lineThickness);
                    drawlist->AddCircleFilled(centerPos, circleRadius, bgCol, 12);
                    break;
                case GitGraphNodeEnum::START_NODE:  // from middle to top with node
                    drawlist->AddLine(centerPos, topPos, bgCol, lineThickness);
                    drawlist->AddCircleFilled(centerPos, circleRadius, bgCol, 12);
                    break;
                case GitGraphNodeEnum::END_NODE:  // from bottom to middle with node
                    drawlist->AddLine(bottomPos, centerPos, bgCol, lineThickness);
                    drawlist->AddCircleFilled(centerPos, circleRadius, bgCol, 12);
                    break;
                case GitGraphNodeEnum::CORNER_LEFT_TOP:  // a Corner from left to top
                    if (idx > 0) {
                        // left to center
                        drawlist->AddLine(leftPos, centerPos, bgCol, lineThickness);
                        // center to top               ;
                        drawlist->AddLine(centerPos, topPos, bgCol, lineThickness);
                    }
                    break;
                case GitGraphNodeEnum::CORNER_BOTTOM_LEFT:  // a Corner from bottom to left
                    if (idx > 0) {
                        // bottom to center
                        drawlist->AddLine(bottomPos, centerPos, bgCol, lineThickness);
                        // center to left
                        drawlist->AddLine(centerPos, leftPos, bgCol, lineThickness);
                    }
                    break;
                case GitGraphNodeEnum::NONE:  // empty
                default: break;
            }

            lastPos = pos;
        }        
    }
    return false;
}

bool GitGui::drawHistory() {
    auto ptr = m_CurrentRepo.lock();
    if (ptr == nullptr) {
        return false;
    }
    auto& commits = ptr->GetHistoryRef();
    if (commits.empty()) {
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
        GitCommitPtr currentCommitPtr = nullptr;
        GitCommitPtr lastCommitPtr = nullptr;
        const auto& textLineHeight = ImGui::GetTextLineHeightWithSpacing();
        m_HistoryClipper.Begin((int)commits.size(), textLineHeight);
        while (m_HistoryClipper.Step()) {
            for (int i = m_HistoryClipper.DisplayStart; i < m_HistoryClipper.DisplayEnd; i++) {
                if (i < 0) {
                    continue;
                }
                lastCommitPtr = nullptr;
                if (i > 0) {
                    lastCommitPtr = commits.at(i - 1);
                }
                currentCommitPtr = commits.at(i);
                if (currentCommitPtr == nullptr) {
                    continue;
                }
                auto branch_ptr = currentCommitPtr->branch.lock();
                if (branch_ptr != nullptr) {
                    ImGui::TableNextRow();
                    bool selected = false;
                    float _branchEndPosX = 0.0f;
                    if (ImGui::TableNextColumn()) {  // branch
                        const auto& bgCol = ImGui::GetColorU32(ImVec4(branch_ptr->color.x, branch_ptr->color.y, branch_ptr->color.z, 1.0f));
                        // https://stackoverflow.com/questions/58044749/how-to-right-align-text-in-imgui-columns
                        const auto& textSize = ImGui::CalcTextSize(branch_ptr->name.c_str());
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - textSize.x - ImGui::GetScrollX() - ImGui::GetStyle().ItemSpacing.x);
                        const auto& win = ImGui::GetCurrentWindow();
                        const auto& pos = win->DC.CursorPos;
                        const auto& drawlist = win->DrawList;
                        drawlist->AddRectFilled(
                            pos - ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.0f), pos + textSize + ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.0f), bgCol, 5.0f);
                        _branchEndPosX = pos.x + textSize.x + ImGui::GetStyle().ItemSpacing.x;
                        const bool pushed =
                            ImGui::PushStyleColorWithContrast4(bgCol, ImGuiCol_Text, ImGui::CustomStyle::puContrastedTextColor, ImGui::CustomStyle::puContrastRatio);
                        ImGui::Text("%s", branch_ptr->name.c_str());
                        if (pushed) {
                            ImGui::PopStyleColor();
                        }
                    }
                    if (ImGui::TableNextColumn()) {  // graph
                        m_drawGraphNode(_branchEndPosX, ptr, currentCommitPtr, lastCommitPtr, textLineHeight);
                    }
                    if (ImGui::TableNextColumn()) {  // id
                        ImGui::Text("%s", currentCommitPtr->idShort.c_str());
                    }
                    if (ImGui::TableNextColumn()) {  // Message
                        if (m_SelectedCommit.lock() == currentCommitPtr) {
                            selected = true;
                        }
                        ImGui::PushID(i);
                        if (ImGui::Selectable(currentCommitPtr->msgOneLine.c_str(), &selected, ImGuiSelectableFlags_SpanAllColumns)) {
                            m_SelectedCommit = currentCommitPtr;
                            CommitPane::Instance()->SelectCommit(currentCommitPtr);
                        }
                        ImGui::PopID();
                    }
                    if (ImGui::TableNextColumn()) {  // Author
                        ImGui::Text("%s", currentCommitPtr->authorName.c_str());
                    }
                    if (ImGui::TableNextColumn()) {  // Date
                        ImGui::Text("%s", currentCommitPtr->date.c_str());
                    }
                }
            }
        }
        m_HistoryClipper.End();
        ImGui::EndTable();
    }
    return true;
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
