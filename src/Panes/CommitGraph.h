#pragma once

#include <ImGuiPack.h>

#include <cstdint>
#include <cstdint>
#include <memory>
#include <string>

class ProjectFile;
class CommitGraph : public AbstractPane {
private:

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(const uint32_t& vCurrentFrame, bool* vOpened = nullptr, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

public:  // singleton
    static std::shared_ptr<CommitGraph> Instance() {
        static std::shared_ptr<CommitGraph> _instance = std::make_shared<CommitGraph>();
        return _instance;
    }

public:
    CommitGraph();                                               // Prevent construction
    CommitGraph(const CommitGraph&) = default;                     // Prevent construction by copying
    CommitGraph& operator=(const CommitGraph&) { return *this; };  // Prevent assignment
    virtual ~CommitGraph();                                      // Prevent unwanted destruction};
};
