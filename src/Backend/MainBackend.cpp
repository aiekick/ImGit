#include "MainBackend.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <Headers/ImGitBuild.h>

#define IMGUI_IMPL_API
#include <3rdparty/imgui_docking/backends/imgui_impl_opengl3.h>
#include <3rdparty/imgui_docking/backends/imgui_impl_glfw.h>

#include <ctools/FileHelper.h>

#include <cstdio>     // printf, fprintf
#include <chrono>     // timer
#include <cstdlib>    // abort
#include <fstream>    // std::ifstream
#include <iostream>   // std::cout
#include <algorithm>  // std::min, std::max
#include <stdexcept>  // std::exception

#include <Backend/MainBackend.h>

#include <LayoutManager.h>

#include <ImGuiPack.h>

#include <ctools/cTools.h>
#include <ctools/Logger.h>
#include <ctools/FileHelper.h>
#include <Frontend/MainFrontend.h>

#include <Panes/ConsolePane.h>

#include <Systems/SettingsDialog.h>

#include <Models/Gui/GitGui.h>

// we include the cpp just for embedded fonts
#include <Res/fontIcons.cpp>
#include <Res/Roboto_Medium.cpp>

#define INITIAL_WIDTH 1700
#define INITIAL_HEIGHT 700

//////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

static void glfw_error_callback(int error, const char* description) {
    LogVarError("glfw error %i : %s", error, description);
}

static void glfw_window_close_callback(GLFWwindow* window) {
    glfwSetWindowShouldClose(window, GLFW_FALSE);  // block app closing
    MainFrontend::Instance()->Action_Window_CloseApp();
}

//////////////////////////////////////////////////////////////////////////////////
//// PUBLIC //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

MainBackend::~MainBackend() = default;

void MainBackend::run() {
    if (init()) {
        m_MainLoop();
        unit();
    }
}

// todo : to refactor ! i dont like that
bool MainBackend::init() {
    SetConsoleVisibility(false);
    if (m_InitWindow() && m_InitImGui() && m_InitModels() && m_InitSystems() && m_InitPanes() && m_InitSettings()) {
        LoadConfigFile("config.xml", "app");
        return true;
    }
    return false;
}

// todo : to refactor ! i dont like that
void MainBackend::unit() {
    SaveConfigFile("config.xml", "app");
    m_UnitSettings();
    m_UnitPanes();
    m_UnitSystems();
    m_UnitModels();
    m_UnitImGui();
    m_UnitWindow();
}

bool MainBackend::isThereAnError() const {
    return false;
}
// actions to do after rendering
void MainBackend::PostRenderingActions() {

}

bool MainBackend::IsNeedToCloseApp() {
    return m_NeedToCloseApp;
}

void MainBackend::NeedToCloseApp(const bool& vFlag) {
    m_NeedToCloseApp = vFlag;
}

void MainBackend::CloseApp() {
    // will escape the main loop
    glfwSetWindowShouldClose(m_MainWindowPtr, 1);
}

void MainBackend::setAppTitle(const std::string& vFilePathName) {
    auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
    if (ps.isOk) {
        char bufTitle[1024];
        snprintf(bufTitle, 1023, "ImGit Beta %s - Project : %s.lum", ImGit_BuildId, ps.name.c_str());
        glfwSetWindowTitle(m_MainWindowPtr, bufTitle);
    } else {
        char bufTitle[1024];
        snprintf(bufTitle, 1023, "ImGit Beta %s", ImGit_BuildId);
        glfwSetWindowTitle(m_MainWindowPtr, bufTitle);
    }
}

ct::dvec2 MainBackend::GetMousePos() {
    ct::dvec2 mp;
    glfwGetCursorPos(m_MainWindowPtr, &mp.x, &mp.y);
    return mp;
}

int MainBackend::GetMouseButton(int vButton) {
    return glfwGetMouseButton(m_MainWindowPtr, vButton);
}

std::string MainBackend::getAppRelativeFilePathName(const std::string& vFilePathName) {
    if (!vFilePathName.empty()) {
        if (FileHelper::Instance()->IsFileExist(vFilePathName)) {
            auto file_path_name = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
            return FileHelper::Instance()->GetPathRelativeToApp(file_path_name);
        }
    }
    return {};
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// CONSOLE ///////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainBackend::SetConsoleVisibility(const bool& vFlag) {
    m_ConsoleVisiblity = vFlag;

    if (m_ConsoleVisiblity) {
        // on cache la console
        // on l'affichera au besoin comme blender fait
#ifdef WIN32
        ShowWindow(GetConsoleWindow(), SW_SHOW);
#endif
    } else {
        // on cache la console
        // on l'affichera au besoin comme blender fait
#ifdef WIN32
        ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif
    }
}

void MainBackend::SwitchConsoleVisibility() {
    m_ConsoleVisiblity = !m_ConsoleVisiblity;
    SetConsoleVisibility(m_ConsoleVisiblity);
}

bool MainBackend::GetConsoleVisibility() {
    return m_ConsoleVisiblity;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// RENDER ////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainBackend::m_RenderOffScreen() {
    // m_DisplaySizeQuadRendererPtr->SetImageInfos(m_MergerRendererPtr->GetBackDescriptorImageInfo(0U));
}

void MainBackend::m_MainLoop() {
    int display_w, display_h;
    while (!glfwWindowShouldClose(m_MainWindowPtr)) {

            // maintain active, prevent user change via imgui dialog
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Enable Docking
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Disable Viewport

#ifndef _DEBUG
            glfwWaitEventsTimeout(1.0);
#endif
            glfwPollEvents();

            glfwGetFramebufferSize(m_MainWindowPtr, &display_w, &display_h);

            m_Update();  // to do absolutly before imgui rendering

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            auto* viewport_ptr = ImGui::GetMainViewport();
            if (viewport_ptr) {
                m_DisplayPos = viewport_ptr->WorkPos;
                m_DisplaySize = viewport_ptr->WorkSize;
            } else {
                m_DisplayPos = {};
                m_DisplaySize.x = (float)display_w;
                m_DisplaySize.y = (float)display_h;
            }

            MainFrontend::Instance()->Display(m_CurrentFrame, m_DisplayPos, m_DisplaySize);

            ImGui::Render();

            glViewport(0, 0, display_w, display_h);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            auto* backup_current_context = glfwGetCurrentContext();

            // Update and Render additional Platform Windows
            // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste
            // this code elsewhere.
            //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }
            glfwMakeContextCurrent(backup_current_context);

#ifdef USE_THUMBNAILS
            ImGuiFileDialog::Instance()->ManageGPUThumbnails();
#endif

            glfwSwapBuffers(m_MainWindowPtr);

            // mainframe post actions
            PostRenderingActions();

            ++m_CurrentFrame;

        // will pause the view until we move the mouse or press keys
        // glfwWaitEvents();
    }
}

void MainBackend::m_Update() {
}

void MainBackend::m_IncFrame() {
    ++m_CurrentFrame;
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string MainBackend::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    std::string str;
    str += MainFrontend::Instance()->getXml(vOffset, vUserDatas);
    str += SettingsDialog::Instance()->getXml(vOffset, vUserDatas);
    str += GitGui::Instance()->getXml(vOffset, vUserDatas);
    return str;
}

bool MainBackend::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    MainFrontend::Instance()->setFromXml(vElem, vParent, vUserDatas);
    SettingsDialog::Instance()->setFromXml(vElem, vParent, vUserDatas);
    GitGui::Instance()->setFromXml(vElem, vParent, vUserDatas);
    return true;
}

//////////////////////////////////////////////////////////////////////////////////
//// PRIVATE /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

bool MainBackend::m_InitWindow() {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return false;

    // GL 3.0 + GLSL 130
    m_GlslVersion = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Create window with graphics context
    m_MainWindowPtr = glfwCreateWindow(1280, 720, "ImGit", nullptr, nullptr);
    if (m_MainWindowPtr == nullptr) {
        std::cout << "Fail to create the window" << std::endl;
        return false;
    }
    glfwMakeContextCurrent(m_MainWindowPtr);
    glfwSwapInterval(1);  // Enable vsync

    if (gladLoadGL() == 0) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return false;
    }

    glfwSetWindowCloseCallback(m_MainWindowPtr, glfw_window_close_callback);

    return true;
}

void MainBackend::m_UnitWindow() {
    glfwDestroyWindow(m_MainWindowPtr);
    glfwTerminate();
}

bool MainBackend::m_InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;    // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;  // Enable Viewport
    io.FontAllowUserScaling = true;                      // activate zoom feature with ctrl + mousewheel
#ifdef USE_DECORATIONS_FOR_RESIZE_CHILD_WINDOWS
    io.ConfigViewportsNoDecoration = false;  // toujours mettre une frame aux fenetres enfants
#endif

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // fonts
    {
        {  // main font
            if (ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_RM, 15.0f) == nullptr) {
                assert(0);  // failed to load font
            }
        }
        {  // icon font
            static const ImWchar icons_ranges[] = {ICON_MIN_IMGIT, ICON_MAX_IMGIT, 0};
            ImFontConfig icons_config;
            icons_config.MergeMode = true;
            icons_config.PixelSnapH = true;
            if (ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_IMGIT, 15.0f, &icons_config, icons_ranges) == nullptr) {
                assert(0);  // failed to load font
            }
        }
    }

    // Setup Platform/Renderer bindings
    if (ImGui_ImplGlfw_InitForOpenGL(m_MainWindowPtr, true) &&  //
        ImGui_ImplOpenGL3_Init(m_GlslVersion)) {
        // ui init
        if (MainFrontend::Instance()->init()) {
            return true;
        }
    }
    return false;
}

bool MainBackend::m_InitModels() {
    return GitGui::Instance()->init();
}

bool MainBackend::m_InitPanes() {
    if (LayoutManager::Instance()->InitPanes()) {
        // a faire apres InitPanes() sinon ConsolePane::Instance()->paneFlag vaudra 0 et changeras apres InitPanes()
        Messaging::Instance()->sMessagePaneId = ConsolePane::Instance()->GetFlag();
        return true;
    }
    return false;
}

bool MainBackend::m_InitSettings() {
    return SettingsDialog::Instance()->init();
}

bool MainBackend::m_InitSystems() {
    return true;
}

void MainBackend::m_UnitModels() {
    GitGui::Instance()->unit();
}

void MainBackend::m_UnitSystems() {
}

void MainBackend::m_UnitPanes() {
}

void MainBackend::m_UnitSettings() {
    SettingsDialog::Instance()->unit();
}

void MainBackend::m_UnitImGui() {
    MainFrontend::Instance()->unit();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}
