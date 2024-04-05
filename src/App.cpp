// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "App.h"

#include <Headers/ImGitBuild.h>
#include <Backend/MainBackend.h>

#include <ctools/FileHelper.h>
#include <ctools/Logger.h>

// messaging
//#include <Res/sdfmToolbarFont.cpp>
#define MESSAGING_CODE_INFOS 0
#define MESSAGING_LABEL_INFOS "Infos"
#define MESSAGING_CODE_WARNINGS 1
#define MESSAGING_LABEL_WARNINGS "Warnings"
#define MESSAGING_CODE_ERRORS 2
#define MESSAGING_CODE_DEBUG 3
#define MESSAGING_LABEL_ERRORS "Errors"
#define MESSAGING_LABEL_DEBUG "Debug"

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

int App::run(int /*argc*/, char** argv) {

    printf("-----------\n");
    printf("[[ ImGit Beta %s ]]\n", ImGit_BuildId);

    FileHelper::Instance()->SetAppPath(argv[0]);
    FileHelper::Instance()->SetCurDirectory(FileHelper::Instance()->GetAppPath());

    m_InitMessaging();

    MainBackend::Instance()->run();

    return 0;
}

void App::m_InitMessaging() {
    Messaging::Instance()->AddCategory(MESSAGING_CODE_INFOS, "Infos(s)", MESSAGING_LABEL_INFOS, ImVec4(0.0f, 0.8f, 0.0f, 1.0f));
    Messaging::Instance()->AddCategory(MESSAGING_CODE_WARNINGS, "Warnings(s)", MESSAGING_LABEL_WARNINGS, ImVec4(0.8f, 0.8f, 0.0f, 1.0f));
    Messaging::Instance()->AddCategory(MESSAGING_CODE_ERRORS, "Errors(s)", MESSAGING_LABEL_ERRORS, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
    Messaging::Instance()->AddCategory(MESSAGING_CODE_DEBUG, "Debug(s)", MESSAGING_LABEL_DEBUG, ImVec4(0.8f, 0.8f, 0.0f, 1.0f));
    Messaging::Instance()->SetLayoutManager(LayoutManager::Instance());
    Logger::sStandardLogFunction = [](const int& vType, const std::string& vMessage) {
        MessageData msg_datas;
        const auto& type = vType;
        Messaging::Instance()->AddMessage(vMessage, type, false, msg_datas, {});
    };
}