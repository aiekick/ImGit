#pragma once

#include <ImGuiPack.h>
#include <ctools/cTools.h>
#include <ctools/ConfigAbstract.h>

#include <string>
#include <memory>
#include <array>
#include <vector>
#include <functional>
#include <unordered_map>

struct GLFWwindow;
class MainBackend : public conf::ConfigAbstract {
private:
    GLFWwindow* m_MainWindowPtr = nullptr;
    const char* m_GlslVersion = "";
    ct::ivec2 m_DisplaySize;

    // mouse
    ct::fvec4 m_MouseFrameSize;
    ct::fvec2 m_MousePos;
    ct::fvec2 m_LastNormalizedMousePos;
    ct::fvec2 m_NormalizedMousePos;

    bool m_ConsoleVisiblity = false;
    uint32_t m_CurrentFrame = 0U;

    bool m_NeedToCloseApp = false;  // when app closing app is required

    bool m_NeedToNewProject = false;
    bool m_NeedToLoadProject = false;
    bool m_NeedToCloseProject = false;
    std::string m_ProjectFileToLoad;

    std::function<void(std::set<std::string>)> m_ChangeFunc;
    std::set<std::string> m_PathsToTrack;

public:  // getters
    ImVec2 GetDisplaySize() {
        return ImVec2((float)m_DisplaySize.x, (float)m_DisplaySize.y);
    }

public:
    virtual ~MainBackend();

    void run();

    bool init();
    void unit();

    bool isThereAnError() const;

    void NeedToNewProject(const std::string& vFilePathName);
    void NeedToLoadProject(const std::string& vFilePathName);
    void NeedToCloseProject();

    bool SaveProject();
    void SaveAsProject(const std::string& vFilePathName);

    void PostRenderingActions();

    bool IsNeedToCloseApp();
    void NeedToCloseApp(const bool& vFlag = true);
    void CloseApp();

    void setAppTitle(const std::string& vFilePathName = {});

    ct::dvec2 GetMousePos();
    int GetMouseButton(int vButton);

    std::string getAppRelativeFilePathName(const std::string& vFilePathName);

public:  // configuration
    std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "") override;
    bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "") override;

    void SetConsoleVisibility(const bool& vFlag);
    void SwitchConsoleVisibility();
    bool GetConsoleVisibility();

private:
    void m_RenderOffScreen();

    bool m_InitWindow();
    bool m_InitImGui();
    void m_InitPlugins();
    void m_InitModels();
    void m_InitSystems();
    void m_InitPanes();
    void m_InitSettings();

    void m_UnitWindow();
    void m_UnitModels();
    void m_UnitImGui();
    void m_UnitPlugins();
    void m_UnitSystems();
    void m_UnitPanes();
    void m_UnitSettings();

    void m_MainLoop();
    void m_Update();
    void m_IncFrame();

public:  // singleton
    static MainBackend* Instance() {
        static MainBackend _instance;
        return &_instance;
    }
};
