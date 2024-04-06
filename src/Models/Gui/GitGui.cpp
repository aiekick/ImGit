#include <Models/Gui/GitGui.h>
#include <ImGuiPack/ImGuiPack.h>
#include <Backend/MainBackend.h>
#include <Project/ProjectFile.h>
#include <ctools/FileHelper.h>

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
        if (!setCurrentRepo(ps.name)) { // if not already exist
            auto ptr = GitRepositery::create(vRepoPathName);
            if (ptr != nullptr) {
                m_Repositeries[ps.name] = ptr;
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
                        if (!opened) { // the user was clicking on the clsoe button
                            m_RepoToCloseDeferred = repo.second;
                        }
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
            if (openLocalRepositery(ImGuiFileDialog::Instance()->GetCurrentPath())) {
                ProjectFile::Instance()->SetProjectChange();
            }
        } 
        ImGuiFileDialog::Instance()->Close();
        return true;
    }
    return false;
}
