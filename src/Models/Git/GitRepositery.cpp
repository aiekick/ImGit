#include <Models/Git/GitRepositery.h>

#include <Models/Git/GitBranch.h>
#include <Models/Git/GitCommit.h>

#include <git2.h>
#include <iostream>
#include <ctools/FileHelper.h>
#include <algorithm>
#include <cstring>

/*
pour trouver le 1er commit :
 - il est le plus vieux
 - il appartient au plus de branches
 - il n'a pas de parents
*/

std::shared_ptr<GitRepositery> GitRepositery::create(const std::string& vRepoPathName) {
    auto res = std::make_shared<GitRepositery>();
    if (!res->Init(vRepoPathName)) {
        res.reset();
    } else {
        res->RetrieveBranchs();
        res->ScanRemoteFullHistory();
    }

    return res;
}

GitRepositery::GitRepositery() {
}

GitRepositery::~GitRepositery() {
    Unit();
}

bool GitRepositery::Init(const std::string& vRepoPathName) {
    if (!vRepoPathName.empty()) {
        int err = git_repository_open(&m_Repo, vRepoPathName.c_str());
        if (err == 0) {
            m_RepoPathName = vRepoPathName;
            m_RepoName = vRepoPathName;
            auto ps = FileHelper::Instance()->ParsePathFileName(m_RepoPathName);
            if (ps.isOk) {
                m_RepoName = ps.name;
            }
            return true;
        } else if (err < 0) {
            std::cout << "Repositery found but cant be opened" << std::endl;
            return false;
        }
    }

    std::cout << "Repositery not found" << std::endl;
    return false;
}

void GitRepositery::Unit() {
    git_repository_free(m_Repo);
}

std::shared_ptr<GitBranch> GitRepositery::GetBranch(const std::string& vBranchName) {
    if (m_HeadBranch->name == vBranchName)
        return m_HeadBranch;

    if (m_Branchs.find(vBranchName) != m_Branchs.end()) {
        return m_Branchs[vBranchName];
    }
    return nullptr;
}

ct::fvec4 GitRepositery::GetBranchColorFromIndex(size_t vIndex) {
    if (vIndex < m_BranchColors.size())
        return m_BranchColors[vIndex];
    CTOOL_DEBUG_BREAK;
    return 0.0f;
}

void GitRepositery::Select_All_Branch() {
    Intenal_Finalize_RetrieveHistory("");
}
void GitRepositery::Select_One_Branch(const std::string& vBranchName) {
    Intenal_Finalize_RetrieveHistory(vBranchName);
}

void GitRepositery::ScanRemoteFullHistory() {
    // methode actuelle mais ne permet pas de savoir quelle commit ava avec quelle branche
    // Internal_RetrieveHistory(true, "");

    Internal_Clear_Databases();
    Internal_RetrieveHistory(false, m_HeadBranch->name, false);
    for (const auto& branchName : m_Branchs) {
        Internal_RetrieveHistory(false, branchName.second->name, false);
    }
    Intenal_Finalize_RetrieveHistory("");
}

void GitRepositery::Internal_Clear_Databases() {
    m_CommitsDatabase.clear();
}

void GitRepositery::Internal_RetrieveHistory(bool vAllBranches, const std::string& vBranchName, const bool& vResetCollections) {
    if (vResetCollections) {
        Internal_Clear_Databases();
    }

    if (m_HeadBranch.use_count()) {
        struct log_state {
            git_repository* repo = nullptr;
            const char* repodir = nullptr;
            git_revwalk* walker = nullptr;
            int hide = 0;
            int sorting = GIT_SORT_TIME | GIT_SORT_REVERSE;
            int revisions = 0;
        } s;

        s.repo = m_Repo;

        if (!s.walker) {
            if (git_revwalk_new(&s.walker, s.repo) != 0) {
                printf("Could not create revision walker\n");
            } else {
                git_revwalk_sorting(s.walker, s.sorting);

                if (vAllBranches) {
                    if (git_revwalk_push_glob(s.walker, "refs/remotes/*") != 0) {
                        printf("Could not find repository for branch %s\n", vBranchName.c_str());
                    }
                    m_CurrentHistoryBranch.clear();
                } else if (vBranchName.empty()) {
                    auto str = "refs/remotes/" + m_HeadBranch->name;
                    if (git_revwalk_push_ref(s.walker, str.c_str()) != 0) {
                        printf("Could not find repository for branch %s\n", m_HeadBranch->name.c_str());
                    }
                    m_CurrentHistoryBranch = m_HeadBranch->name;
                } else {
                    auto str = "refs/remotes/" + vBranchName;
                    if (git_revwalk_push_ref(s.walker, str.c_str()) != 0) {
                        printf("Could not find repository for branch %s\n", vBranchName.c_str());
                    }
                    m_CurrentHistoryBranch = vBranchName;
                }
            }
        }

        if (m_CurrentHistoryBranch.empty())
            return;

        // reset stats
        if (m_CurrentHistoryBranch == m_HeadBranch->name) {
            m_HeadBranch->nCommits = 0U;
            m_HeadBranch->nUnCommits = 0U;
        } else {
            m_Branchs[m_CurrentHistoryBranch]->nCommits = 0U;
            m_Branchs[m_CurrentHistoryBranch]->nUnCommits = 0U;
        }

        // on parcours tous les commits
        git_oid oid;
        git_commit* commit = nullptr;
        //git_commit* parentCommit = nullptr;
        while (git_revwalk_next(&oid, s.walker) == 0) {
            git_commit_free(commit);

            // on parcours les commit
            if (git_commit_lookup(&commit, s.repo, &oid) == 0) {
                char buf[GIT_OID_HEXSZ + 1];

                git_oid_tostr(buf, sizeof(buf), git_commit_id(commit));
                std::string commitId = buf;

                if (m_CurrentHistoryBranch == m_HeadBranch->name)
                    m_HeadBranch->nCommits++;
                else
                    m_Branchs[m_CurrentHistoryBranch]->nCommits++;

                std::shared_ptr<GitCommit> gitCommit = nullptr;

                if (m_CommitsDatabase.find(commitId) == m_CommitsDatabase.end()) {
                    gitCommit = std::make_shared<GitCommit>();

                    gitCommit->id = commitId;
                    gitCommit->idShort = std::string(buf, ct::mini(strlen(buf), (size_t)7U));

                    auto orgGitRepo = git_commit_owner(commit);
                    if (orgGitRepo != m_Repo) {
                        printf("Repo not the same\n");
                    }

                    uint32_t count_parent = git_commit_parentcount(commit);
                    if (count_parent > 1U) {
                        gitCommit->msg = "Merge :";
                        for (uint32_t i = 0U; i < count_parent; ++i) {
                            git_oid_tostr(buf, 8, git_commit_parent_id(commit, i));
                            gitCommit->msg += " " + std::string(buf);
                        }
                        gitCommit->msg += "\n";
                    }

                    const git_signature* sig = git_commit_author(commit);
                    if (sig) {
                        gitCommit->authorName = sig->name;
                        gitCommit->authorMail = sig->email;
                        gitCommit->dateEpoch = sig->when.time;

                        // date
                        static char out[200 + 1];
                        time_t t = (time_t)sig->when.time + 60 * sig->when.offset;
                        auto* intm = std::localtime(&t);
                        strftime(out, 200, "%Y/%m/%d %H:%M:%S", intm);
                        gitCommit->date = std::string(out);
                    }

                    gitCommit->msg += git_commit_message(commit);
                    gitCommit->msgSize = gitCommit->msg.size();

                    gitCommit->branch = GetBranch(m_CurrentHistoryBranch);
                    assert(gitCommit->branch.use_count());
                    gitCommit->graphColumn = gitCommit->branch->graphColumn;

                    if (m_CurrentHistoryBranch == m_HeadBranch->name)
                        m_HeadBranch->nUnCommits++;
                    else
                        m_Branchs[m_CurrentHistoryBranch]->nUnCommits++;

                    // conversion in one line only
                    gitCommit->msgOneLine = gitCommit->msg;
                    auto el = gitCommit->msgOneLine.find('\n');
                    if (el != std::string::npos) {
                        gitCommit->msgOneLine = gitCommit->msgOneLine.substr(0, el);
                    }

                    if (m_CommitsDatabase.empty())
                        gitCommit->isRoot = true;  // first commit

                    gitCommit->branchPtrId = (intptr_t)(gitCommit->branch.get());

                    m_CommitsDatabase[gitCommit->id] = gitCommit;
                    m_LastCommitPerBranch[gitCommit->branchPtrId] = gitCommit;
                } else {
                    gitCommit = m_CommitsDatabase[commitId];
                }

                /*if (gitCommit.use_count())
                {
                    gitCommit->branchs.emplace(m_CurrentHistoryBranch);

                    if (gitCommit->branchs.size() == 1U)
                    {
                        gitCommit->branch = *gitCommit->branchs.begin();
                    }
                }*/
            }
        }

        for (auto co : m_LastCommitPerBranch) {
            co.second->isLastCommit = true;
        }

        git_revwalk_free(s.walker);
    }
}

void GitRepositery::Intenal_Finalize_RetrieveHistory(const std::string& vBranchName) {
    m_History.clear();

    if (vBranchName.empty()) {
        for (const auto& c : m_CommitsDatabase) {
            m_History.push_back(c.second);
        }
    } else {
        for (const auto& c : m_CommitsDatabase) {
            if (c.second->branch->name == vBranchName) {
                m_History.push_back(c.second);
            }
        }
    }

    std::sort(m_History.begin(), m_History.end(), [](const std::shared_ptr<GitCommit>& a, const std::shared_ptr<GitCommit>& b) -> bool {
        if (a->dateEpoch == b->dateEpoch)
            return (a->id > b->id);
        return (a->dateEpoch > b->dateEpoch);
    });

    size_t maxGraphColumn = 0U;

    for (auto it = m_History.rbegin(); it != m_History.rend(); ++it) {
        if ((*it)->graphColumn > maxGraphColumn) {
            (*it)->isNewbranch = true;
        }
        maxGraphColumn = ct::maxi(maxGraphColumn, (*it)->graphColumn);
        (*it)->maxGraphColumn = maxGraphColumn;
    }

    m_CurrentHistoryBranch = vBranchName;
}

void GitRepositery::Internal_GetRemoteDefaultBranchName() {
    m_HeadBranch = std::make_shared<GitBranch>();
    if (m_HeadBranch.use_count()) {
        git_branch_iterator* it;
        if (!git_branch_iterator_new(&it, m_Repo, GIT_BRANCH_REMOTE)) {
            git_reference* ref;
            git_branch_t type;
            while (git_branch_next(&ref, &type, it) == 0) {
                const char* name;
                if (git_branch_name(&name, ref) == 0) {
                    const std::string& nameStr = name;
                    if (nameStr.find("/HEAD") != std::string::npos) {
                        m_HeadBranch->name = git_reference_symbolic_target(ref);
                        ct::replaceString(m_HeadBranch->name, "refs/remotes/", "");
                        git_reference_free(ref);
                        break;
                    } else if (nameStr.find("master") != std::string::npos) {
                        m_HeadBranch->name = nameStr;
                        ct::replaceString(m_HeadBranch->name, "refs/remotes/", "");
                        git_reference_free(ref);
                        break;
                    }
                }
                git_reference_free(ref);
            }
            git_branch_iterator_free(it);
        }
    }
}

// https://www.shadertoy.com/view/ld3fzf
static inline ct::fvec4 GetRainBow(float r) {
    ct::fvec4 c(0.0f, 23.0f, 21.0f, 1.0f);
    return ct::cos(c + r * 6.3f) * 0.5f + 0.5f;
}

void GitRepositery::RetrieveBranchs() {
    m_Branchs.clear();
    m_BranchColors.clear();
    Internal_GetRemoteDefaultBranchName();

    if (m_HeadBranch.use_count()) {
        size_t graphColumnCount = 0U;

        git_branch_iterator* it;
        if (!git_branch_iterator_new(&it, m_Repo, GIT_BRANCH_REMOTE)) {
            git_reference* ref;
            git_branch_t type;
            while (!git_branch_next(&ref, &type, it)) {
                const char* name;
                if (git_branch_name(&name, ref) == 0) {
                    const std::string& nameStr = name;
                    if (nameStr.find("/HEAD") == std::string::npos) {
                        if (m_HeadBranch->name != nameStr) {
                            auto res = std::make_shared<GitBranch>();
                            res->name = nameStr;
                            res->graphColumn = ++graphColumnCount;
                            m_Branchs[nameStr] = res;
                        }
                    }
                }
                git_reference_free(ref);
            }
            git_branch_iterator_free(it);
        }

        float count = (float)(m_Branchs.size() + 1);
        float idx = 1.0f;
        m_HeadBranch->color = GetRainBow(idx++ / count);
        m_BranchColors.push_back(m_HeadBranch->color);
        for (auto& b : m_Branchs) {
            b.second->color = GetRainBow(idx++ / count);
            m_BranchColors.push_back(b.second->color);
        }
    }
}
