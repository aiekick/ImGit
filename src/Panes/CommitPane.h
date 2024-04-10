#pragma once

#include <Models/Git/GitCommit.h>

#include <ImGuiPack.h>
#include <cstdint>
#include <memory>
#include <string>

class ProjectFile;
class CommitPane : public AbstractPane {
private:
    GitCommitWeak m_SelectedCommit;

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(
        const uint32_t& vCurrentFrame, bool* vOpened = nullptr, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

    void SelectCommit(const GitCommitWeak& vCommit);

private:
    void m_DrawCommitInfos();

public:  // singleton
    static std::shared_ptr<CommitPane> Instance() {
        static std::shared_ptr<CommitPane> _instance = std::make_shared<CommitPane>();
        return _instance;
    }

public:
    CommitPane();                              // Prevent construction
    CommitPane(const CommitPane&) = default;  // Prevent construction by copying
    CommitPane& operator=(const CommitPane&) {
        return *this;
    };                       // Prevent assignment
    virtual ~CommitPane();  // Prevent unwanted destruction};
};
