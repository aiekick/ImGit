/*
Copyright 2022-2023 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "MainFrontend.h"

#include <ImGuiPack.h>

#include <Backend/MainBackend.h>

#include <Panes/ConsolePane.h>
#include <Panes/CodePane.h>
#include <Panes/CommitGraph.h>
#include <Panes/CommitPane.h>
#include <Panes/ObjectsPane.h>

#include <Systems/SettingsDialog.h>

#include <Helpers/TranslationHelper.h>

#include <Models/Gui/GitGui.h>

// panes
#define DEBUG_PANE_ICON ICON_SDFM_BUG
#define SCENE_PANE_ICON ICON_SDFM_FORMAT_LIST_BULLETED_TYPE
#define TUNING_PANE_ICON ICON_SDFM_TUNE
#define CONSOLE_PANE_ICON ICON_SDFMT_COMMENT_TEXT_MULTIPLE

// features
#define GRID_ICON ICON_SDFMT_GRID
#define MOUSE_ICON ICON_SDFMT_MOUSE
#define CAMERA_ICON ICON_SDFMT_CAMCORDER
#define GIZMO_ICON ICON_SDFMT_AXIS_ARROW

// ribbon
#define RIBBON_HEIGHT 100.0f

using namespace std::placeholders;

//////////////////////////////////////////////////////////////////////////////////
//// STATIC //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

bool MainFrontend::sCentralWindowHovered = false;

//////////////////////////////////////////////////////////////////////////////////
//// PUBLIC //////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

MainFrontend::~MainFrontend() = default;

bool MainFrontend::init() {
    ImGui::CustomStyle::Init();

    m_build_themes();

    LayoutManager::Instance()->Init("Layouts", "Default Layout");

    LayoutManager::Instance()->SetPaneDisposalRatio("LEFT", 0.25f);
    LayoutManager::Instance()->SetPaneDisposalRatio("RIGHT", 0.25f);
    LayoutManager::Instance()->SetPaneDisposalRatio("BOTTOM", 0.25f);

    LayoutManager::Instance()->AddPane(ObjectsPane::Instance(), "Objects", "", "LEFT", 0.2f, true, false);
    LayoutManager::Instance()->AddPane(CodePane::Instance(), "Code", "", "CENTRAL", 0.0f, false, false);
    LayoutManager::Instance()->AddPane(CommitGraph::Instance(), "Commit Graph", "", "CENTRAL", 0.0f, true, false);
    LayoutManager::Instance()->AddPane(CommitPane::Instance(), "Commit Infos", "", "RIGHT", 0.3f, true, false);
    LayoutManager::Instance()->AddPane(ConsolePane::Instance(), "Console", "", "BOTTOM", 0.3f, false, false);

    // InitPänes is done in m_InitPanes, because a specific order is needed

    return m_build();
}

void MainFrontend::unit() {
    LayoutManager::Instance()->UnitPanes();
}

bool MainFrontend::isValid() const {
    return false;
}

bool MainFrontend::isThereAnError() const {
    return false;
}

void MainFrontend::Display(const uint32_t& vCurrentFrame, const ImVec2& vPos, const ImVec2& vSize) {
    const auto context_ptr = ImGui::GetCurrentContext();
    if (context_ptr != nullptr) {
        const auto& io = ImGui::GetIO();

        m_DisplayPos = vPos;
        m_DisplaySize = vSize;

        MainFrontend::sCentralWindowHovered = (ImGui::GetCurrentContext()->HoveredWindow == nullptr);
        ImGui::CustomStyle::ResetCustomId();

        // m_drawLeftButtonBar();
        m_drawMainMenuBar();
        m_drawMainStatusBar();

        GitGui::Instance()->drawRibbon(RIBBON_HEIGHT);

        if (LayoutManager::Instance()->BeginDockSpace(ImGuiDockNodeFlags_PassthruCentralNode, ImVec2(0.0f, RIBBON_HEIGHT))) {
            /*if (MainBackend::Instance()->GetBackendDatasRef().canWeTuneGizmo) {
                const auto viewport = ImGui::GetMainViewport();
                ImGuizmo::SetDrawlist(ImGui::GetCurrentWindow()->DrawList);
                ImGuizmo::SetRect(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);
                ImRect rc(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);
                DrawOverlays(vCurrentFrame, rc, context_ptr, {});
            }*/
            LayoutManager::Instance()->EndDockSpace();
        }

        LayoutManager::Instance()->DrawPanes(vCurrentFrame, context_ptr, {});

        DrawDialogsAndPopups(vCurrentFrame, MainBackend::Instance()->GetDisplaySize(), context_ptr, {});

        ImGuiThemeHelper::Instance()->Draw();
        LayoutManager::Instance()->InitAfterFirstDisplay(io.DisplaySize);
    }
}

bool MainFrontend::DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr, void* vUserDatas) {
    bool res = false;
    return res;
}

bool MainFrontend::DrawOverlays(const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr, void* vUserDatas) {
    bool res = false;
    return res;
}

bool MainFrontend::DrawDialogsAndPopups(const uint32_t& vCurrentFrame, const ImVec2& vMaxSize, ImGuiContext* vContextPtr, void* vUserDatas) {
    m_ActionSystem.RunActions();
    LayoutManager::Instance()->DrawDialogsAndPopups(vCurrentFrame, vMaxSize, vContextPtr, vUserDatas);
    SettingsDialog::Instance()->Draw();
    GitGui::Instance()->drawDialogs();
    if (m_ShowImGui) {
        ImGui::ShowDemoWindow(&m_ShowImGui);
    }
    if (m_ShowImPlot) {
        ImPlot::ShowDemoWindow(&m_ShowImPlot);
    }
    if (m_ShowMetric) {
        ImGui::ShowMetricsWindow(&m_ShowMetric);
    }
    return false;
}

void MainFrontend::OpenAboutDialog() {
    m_ShowAboutDialog = true;
}

void MainFrontend::m_drawMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {

        const auto& io = ImGui::GetIO();
        LayoutManager::Instance()->DisplayMenu(io.DisplaySize);

        ImGui::Spacing();

        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Settings")) {
                SettingsDialog::Instance()->OpenDialog();
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Styles")) {
                ImGuiThemeHelper::Instance()->DrawMenu();

                ImGui::Separator();

                ImGui::MenuItem("Show ImGui", "", &m_ShowImGui);
                ImGui::MenuItem("Show ImGui Metric/Debug", "", &m_ShowMetric);
                ImGui::MenuItem("Show ImPlot", "", &m_ShowImPlot);

                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::MenuItem(" About")) {
            OpenAboutDialog();
        }

        // ImGui Infos
        const auto label = ct::toStr("Dear ImGui %s (Docking)", ImGui::GetVersion());
        const auto size = ImGui::CalcTextSize(label.c_str());
        static float s_translation_menu_size = 0.0f;

        ImGui::Spacing(ImGui::GetContentRegionAvail().x - size.x - s_translation_menu_size - ImGui::GetStyle().FramePadding.x * 2.0f);
        ImGui::Text("%s", label.c_str());

        s_translation_menu_size = TranslationHelper::Instance()->DrawMenu();

        ImGui::EndMainMenuBar();
    }
}

void MainFrontend::m_drawMainStatusBar() {
    if (ImGui::BeginMainStatusBar()) {
        Messaging::Instance()->DrawStatusBar();

        //  ImGui Infos
        const auto& io = ImGui::GetIO();
        const auto fps = ct::toStr("%.1f ms/frame (%.1f fps)", 1000.0f / io.Framerate, io.Framerate);
        const auto size = ImGui::CalcTextSize(fps.c_str());
        ImGui::Spacing(ImGui::GetContentRegionAvail().x - size.x - ImGui::GetStyle().FramePadding.x * 2.0f);
        ImGui::Text("%s", fps.c_str());

        // MainFrontend::sAnyWindowsHovered |= ImGui::IsWindowHovered();

        ImGui::EndMainStatusBar();
    }
}

///////////////////////////////////////////////////////
//// ACTIONS //////////////////////////////////////////
///////////////////////////////////////////////////////


void MainFrontend::Action_Window_CloseApp() {
    if (MainBackend::Instance()->IsNeedToCloseApp())
        return;  // block next call to close app when running
    /*
    Close app :
    -	unsaved :
        -	add action : show unsaved dialog
        -	add action : Close app
    -	saved :
        -	add action : Close app
    */

    m_ActionSystem.Clear();
    m_ActionSystem.Add([]() {
        MainBackend::Instance()->CloseApp();
        return true;
    });
}

void MainFrontend::m_Action_Cancel() {
    /*
    -	cancel :
        -	clear actions
    */
    m_ActionSystem.Clear();
    MainBackend::Instance()->NeedToCloseApp(false);
}

///////////////////////////////////////////////////////
//// APP CLOSING //////////////////////////////////////
///////////////////////////////////////////////////////

void MainFrontend::IWantToCloseTheApp() {
    Action_Window_CloseApp();
}

///////////////////////////////////////////////////////
//// DROP /////////////////////////////////////////////
///////////////////////////////////////////////////////

void MainFrontend::JustDropFiles(int count, const char** paths) {
    assert(0);

    std::map<std::string, std::string> dicoFont;
    std::string prj;

    /*for (int i = 0; i < count; ++i) {
        // file
        auto f = std::string(paths[i]);

        // lower case
        auto f_opt = f;
        for (auto& c : f_opt)
            c = (char)std::tolower((int)c);

        // well known extention
        if (f_opt.find(".ttf") != std::string::npos     // truetype (.ttf)
            || f_opt.find(".otf") != std::string::npos  // opentype (.otf)
            //||	f_opt.find(".ttc") != std::string::npos		// ttf/otf collection for futur (.ttc)
        ) {
            dicoFont[f] = f;
        }
        if (f_opt.find(PROJECT_EXT) != std::string::npos) {
            prj = f;
        }
    }*/

    // priority to project file
    if (!prj.empty()) {
    }
}

//////////////////////////////////////////////////////////////////////////////////
//// PRIVATE /////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

bool MainFrontend::m_build() {
    // toolbar
    /*static ImFontConfig icons_config3;
    icons_config3.MergeMode = false;
    icons_config3.PixelSnapH = true;
    static const ImWchar icons_ranges3[] = {ICON_MIN_SDFMT, ICON_MAX_SDFMT, 0};
    const float& font_size = 20.0f / font_scale_ratio;
    m_ToolbarFontPtr =
        ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_SDFMT, font_size, &icons_config3, icons_ranges3);
    if (m_ToolbarFontPtr != nullptr) {
        m_ToolbarFontPtr->Scale = font_scale_ratio;
        return true;
    }*/

    return true;
}

///////////////////////////////////////////////////////
//// CONFIGURATION ////////////////////////////////////
///////////////////////////////////////////////////////

std::string MainFrontend::getXml(const std::string& vOffset, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    std::string str;

    str += ImGuiThemeHelper::Instance()->getXml(vOffset);
    str += LayoutManager::Instance()->getXml(vOffset, "app");
#ifdef USE_PLACES_FEATURE
    str += vOffset + "<places>" + ImGuiFileDialog::Instance()->SerializePlaces() + "</places>\n";
#endif
    str += vOffset + "<showaboutdialog>" + (m_ShowAboutDialog ? "true" : "false") + "</showaboutdialog>\n";
    str += vOffset + "<showimgui>" + (m_ShowImGui ? "true" : "false") + "</showimgui>\n";
    str += vOffset + "<showmetric>" + (m_ShowMetric ? "true" : "false") + "</showmetric>\n";

    return str;
}

bool MainFrontend::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas) {
    UNUSED(vUserDatas);

    // The value of this child identifies the name of this element
    std::string strName;
    std::string strValue;
    std::string strParentName;

    strName = vElem->Value();
    if (vElem->GetText())
        strValue = vElem->GetText();
    if (vParent != 0)
        strParentName = vParent->Value();

    ImGuiThemeHelper::Instance()->setFromXml(vElem, vParent);
    LayoutManager::Instance()->setFromXml(vElem, vParent, "app");

    if (strName == "places") {
#ifdef USE_PLACES_FEATURE
        ImGuiFileDialog::Instance()->DeserializePlaces(strValue);
#endif
    } else if (strName == "showaboutdialog") {
        m_ShowAboutDialog = ct::ivariant(strValue).GetB();
    } else if (strName == "showimgui") {
        m_ShowImGui = ct::ivariant(strValue).GetB();
    } else if (strName == "showmetric") {
        m_ShowMetric = ct::ivariant(strValue).GetB();
    }

    return true;
}
