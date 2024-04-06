#pragma once

#include <Models/Git/GitRepositery.h>
#include <set>
#include <string>

typedef std::string RepoPathName;
typedef std::string RepoName;

class GitGui {
private:
    std::map<RepoName, GitRepositeryPtr> m_Repositeries;
    GitRepositeryWeak m_CurrentRepo;
    GitRepositeryWeak m_RepoToCloseDeferred;

public:
    bool init();
    void unit();

    bool openLocalRepositery(const RepoPathName& vRepoPathName);
    bool closeRepositery(const RepoName& vRepoName);
    bool closeRepositery(const GitRepositeryWeak& vRepo);
    bool isRepoAlreadyOpened(const RepoName& vRepoName);
    bool setCurrentRepo(const RepoName& vRepoName);

    bool drawRibbon(const float& vHeight);
    bool drawDialogs();

public:  // singleton
    static GitGui* Instance() {
        static GitGui _instance;
        return &_instance;
    }

protected:
    GitGui() = default;
    GitGui(const GitGui&) = delete;                 
    GitGui& operator=(const GitGui&) = delete;
    ~GitGui() = default;
};
