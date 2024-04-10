#pragma once

#include <git2.h>
#include <ctools/cTools.h>
#include <Headers/globals.h>
#include <Utils/SearchableVector.hpp>

#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <vector>
#include <memory>

struct git_repository;

/// <summary>
/// GitBranchCollection
/// 0 => "LOCAL" or remote name
/// 1 => branch name
/// 3 => branch ptr
/// </summary>
typedef std::unordered_map<std::string, std::map<std::string, GitBranchPtr>> GitBranchCollection;

/// <summary>
/// GitCommitsDataBase
/// 0 => commit sha256 id
/// 1 => commit ptr
/// </summary>
typedef std::unordered_map<std::string, GitCommitPtr> GitCommitsDataBase;

/// <summary>
/// GitHistoryCollection
/// all commit ptrs
/// </summary>
typedef SearchableVector<std::string, GitCommitPtr> GitHistoryCollection;

/// <summary>
/// GitTags
/// 0 => tag name
/// 1 => commit
/// </summary>
typedef std::unordered_map<std::string, GitCommitWeak> GitTags;

class GitRepositery {
public:
    static GitRepositeryPtr create(const std::string& vRepoPathName);

private:
    std::string m_RepoPathName;
    std::string m_RepoName;
    git_repository* m_Repo = nullptr;
    GitBranchCollection m_Branchs;
    std::vector<ct::fvec4> m_BranchColors;  // per branch column

    // history
    std::string m_CurrentHistoryBranch = "HEAD";
    GitBranchWeak m_HeadBranch;
    GitCommitsDataBase m_CommitsDatabase;
    GitHistoryCollection m_History;
    GitTags m_Tags;

public:
    GitRepositery();
    ~GitRepositery();

    bool Init(const std::string& vRepoPathName);
    void Unit();

    std::string GetRepoPathName() {
        return m_RepoPathName;
    }
    std::string GetRepoName() {
        return m_RepoName;
    }
    std::string GetCurrentHistoryBranch() {
        return m_CurrentHistoryBranch;
    }
    GitHistoryCollection& GetHistoryRef() {
        return m_History;
    }
    GitBranchCollection& GetBranchsRef() {
        return m_Branchs;
    }
    GitBranchWeak GetHeadBranch() {
        return m_HeadBranch;
    }
    GitBranchWeak GetBranch(const std::string& vBranchName);
    ct::fvec4 GetBranchColorFromIndex(size_t vIndex);

    void SelectAllBranch();
    void SelectOneBranch(const std::string& vBranchName);

private:
    void m_ScanFullHistory();
    void m_RetrieveBranchs();
    void m_RetrieveTags();
    void m_ClearDatabases();
    void m_RetrieveHistory(const std::string& vBanchType, const std::string& vBranchName, const bool& vResetCollections = true);
    void m_FinalizeRetrieveHistory(const std::string& vBranchName);
};