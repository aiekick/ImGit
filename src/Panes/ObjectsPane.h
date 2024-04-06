#pragma once

#include <ImGuiPack.h>
#include <cstdint>
#include <memory>
#include <string>

class ProjectFile;
class ObjectsPane : public AbstractPane {
private:

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

public:  // singleton
    static std::shared_ptr<ObjectsPane> Instance() {
        static std::shared_ptr<ObjectsPane> _instance = std::make_shared<ObjectsPane>();
        return _instance;
    }

public:
    ObjectsPane();                              // Prevent construction
    ObjectsPane(const ObjectsPane&) = default;  // Prevent construction by copying
    ObjectsPane& operator=(const ObjectsPane&) {
        return *this;
    };                       // Prevent assignment
    virtual ~ObjectsPane();  // Prevent unwanted destruction};
};
