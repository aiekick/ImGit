#pragma once

#include <ctools/cTools.h>

#include <string>
#include <unordered_map>
#include <map>
#include <set>
#include <vector>
#include <memory>

struct git_repository;
class GitBranch;
class GitCommit;

typedef std::map<std::string, std::shared_ptr<GitBranch>> GitBranchCollection;
typedef std::vector<std::shared_ptr<GitCommit>> GitHistoryCollection;

class GitRepositery;
typedef std::shared_ptr<GitRepositery> GitRepositeryPtr;
typedef std::weak_ptr<GitRepositery> GitRepositeryWeak;

class GitRepositery {
public:
    static GitRepositeryPtr create(const std::string& vRepoPathName);

private:
	std::string m_RepoPathName;
	std::string m_RepoName;
	git_repository* m_Repo = nullptr;
	GitBranchCollection m_Branchs;
	std::vector<ct::fvec4> m_BranchColors;

	// history
	std::string m_CurrentHistoryBranch = "HEAD";
	std::shared_ptr<GitBranch> m_HeadBranch;

	// map : commit id | GitCommit ptr
	std::map<std::string, std::shared_ptr<GitCommit>> m_CommitsDatabase;

	// map : branch ptr | GitCommit ptr
	std::map<intptr_t, std::shared_ptr<GitCommit>> m_LastCommitPerBranch;

	GitHistoryCollection m_History;

public:
	GitRepositery();
	~GitRepositery();

	bool Init(const std::string& vRepoPathName);
	void Unit();

	std::string GetRepoPathName() { return m_RepoPathName; }
	std::string GetRepoName() { return m_RepoName; }
	std::string GetCurrentHistoryBranch() { return m_CurrentHistoryBranch; }

	GitHistoryCollection& GetHistoryRef() { return m_History; }
	GitBranchCollection& GetBranchsRef() { return m_Branchs; }
	std::shared_ptr<GitBranch> GetHeadBranch() { return m_HeadBranch; }
	std::shared_ptr<GitBranch> GetBranch(const std::string& vBranchName);
	ct::fvec4 GetBranchColorFromIndex(size_t vIndex);
	std::map<intptr_t, std::shared_ptr<GitCommit>>* GetLastCommitsPerBranch() { return &m_LastCommitPerBranch; }

	void Select_All_Branch();
	void Select_One_Branch(const std::string& vBranchName);
	void ScanRemoteFullHistory();
	void RetrieveBranchs();

private:
	void Internal_Clear_Databases();
	void Internal_RetrieveHistory(bool vAllBranches, const std::string& vBranchName, const bool& vResetCollections = true);
	void Intenal_Finalize_RetrieveHistory(const std::string& vBranchName);
	void Internal_GetRemoteDefaultBranchName();
};