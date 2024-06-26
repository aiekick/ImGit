#include <Models/Git/GitRepositery.h>

#include <Models/Git/GitBranch.h>
#include <Models/Git/GitCommit.h>

#include <iostream>
#include <ctools/FileHelper.h>
#include <algorithm>
#include <cstring>

#include <ctools/Logger.h>

#include <git2/refs.h>

/*
pour trouver le 1er commit :
 - il est le plus vieux
 - il appartient au plus de branches
 - il n'a pas de parents
*/

GitRepositeryPtr GitRepositery::create(const std::string& vRepoPathName) {
    auto res = std::make_shared<GitRepositery>();
    if (!res->Init(vRepoPathName)) {
        res.reset();
    } else {
        res->m_RetrieveBranchs();
        res->m_RetrieveTags();
        res->m_ScanFullHistory();
    }
    return res;
}

GitRepositery::GitRepositery() = default;

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
            LogVarError("Repositery found but cant be opened");
            return false;
        }
    }
    LogVarError("Repositery not found");
    return false;
}

void GitRepositery::Unit() {
    git_repository_free(m_Repo);
}

GitBranchWeak GitRepositery::GetBranch(const std::string& vBranchName) {
    for (const auto& type : m_Branchs) {
        if (type.second.find(vBranchName) != type.second.end()) {
            return type.second.at(vBranchName);
        }
    }
    return {};
}

ct::fvec4 GitRepositery::GetBranchColorFromIndex(size_t vIndex) {
    if (vIndex < m_BranchColors.size()) {
        return m_BranchColors[vIndex];
    }
    return {};
}

void GitRepositery::SelectAllBranch() {
    m_FinalizeRetrieveHistory("");
}
void GitRepositery::SelectOneBranch(const std::string& vBranchName) {
    m_FinalizeRetrieveHistory(vBranchName);
}

void GitRepositery::m_ScanFullHistory() {
    // methode actuelle mais ne permet pas de savoir quelle commit ava avec quelle branche
    // Internal_RetrieveHistory(true, "");

    m_ClearDatabases();
    for (const auto& type : m_Branchs) {
        for (const auto& branch : type.second) {
            m_RetrieveHistory(type.first, branch.first, false);
        }
    }
    for (const auto& tag : m_Tags) {
        m_RetrieveHistory(GIT_TAG_TARGET, tag.first, false);
    }
    m_FinalizeRetrieveHistory("");
}

void GitRepositery::m_ClearDatabases() {
    m_CommitsDatabase.clear();
}

void GitRepositery::m_RetrieveHistory(const std::string& vBanchType, const std::string& vBranchName, const bool& vResetCollections) {
    if (vResetCollections) {
        m_ClearDatabases();
    }
    struct log_state {
        git_repository* repo = nullptr;
        const char* repodir = nullptr;
        git_revwalk* walker = nullptr;
        int hide = 0;
        int sorting = GIT_SORT_TIME | GIT_SORT_REVERSE;
        int revisions = 0;
    } s;
    s.repo = m_Repo;
    GitBranchPtr currentBranch = nullptr;
    if (!s.walker) {
        if (git_revwalk_new(&s.walker, s.repo) != 0) {
            LogVarError("Could not create revision walker");
        } else {
            git_revwalk_sorting(s.walker, s.sorting);
            if (vBanchType == GIT_LOCAL_TARGET) {
                auto str = "refs/heads/" + vBranchName;
                if (git_revwalk_push_ref(s.walker, str.c_str()) != 0) {
                    LogVarError("Could not find repository for branch %s", vBranchName.c_str());
                    git_revwalk_free(s.walker);
                    return;
                }
                m_CurrentHistoryBranch = vBranchName;
            } else if (vBanchType == GIT_TAG_TARGET) {
                auto str = "refs/tags/" + vBranchName;
                if (git_revwalk_push_ref(s.walker, str.c_str()) != 0) {
                    LogVarError("Could not find repository for branch %s", vBranchName.c_str());
                    git_revwalk_free(s.walker);
                    return;
                }
                m_CurrentHistoryBranch = vBranchName;
            } else {
                auto str = "refs/remotes/" + vBanchType + "/" + vBranchName;
                if (git_revwalk_push_ref(s.walker, str.c_str()) != 0) {
                    LogVarError("Could not find repository for branch %s", vBranchName.c_str());
                    git_revwalk_free(s.walker);
                    return;
                }
                m_CurrentHistoryBranch = vBranchName;
            }
        }
    }

    if (m_CurrentHistoryBranch.empty()) {
        return;
    }

    if (m_Branchs.find(vBanchType) == m_Branchs.end()) { // not found
        currentBranch = std::make_shared<GitBranch>();
        m_Branchs[vBanchType][vBranchName] = currentBranch;
    } else if (m_Branchs.at(vBanchType).find(vBranchName) == m_Branchs.at(vBanchType).end()) {  // not found
        currentBranch = std::make_shared<GitBranch>();
        m_Branchs[vBanchType][vBranchName] = currentBranch;
    } else {
        currentBranch = m_Branchs.at(vBanchType).at(vBranchName);
    }

    // reset stats
    currentBranch->nCommits = 0U;
    currentBranch->nUnCommits = 0U;

    // on parcours tous les commits
    git_oid oid;
    git_commit* commit = nullptr;
    // git_commit* parentCommit = nullptr;
    while (git_revwalk_next(&oid, s.walker) == 0) {
        git_commit_free(commit);

        // on parcours les commit
        if (git_commit_lookup(&commit, s.repo, &oid) == 0) {
            char buf[GIT_OID_HEXSZ + 1];

            git_oid_tostr(buf, sizeof(buf), git_commit_id(commit));
            std::string commitId = buf;

            currentBranch->nCommits++;

            GitCommitPtr gitCommit = nullptr;

            if (m_CommitsDatabase.find(commitId) == m_CommitsDatabase.end()) {
                gitCommit = std::make_shared<GitCommit>();

                gitCommit->id = commitId;
                gitCommit->idShort = std::string(buf, ct::mini(strlen(buf), (size_t)7U));

                auto orgGitRepo = git_commit_owner(commit);
                if (orgGitRepo != m_Repo) {
                    LogVarError("Repo not the same");
                }

                uint32_t count_parent = git_commit_parentcount(commit);
                if (count_parent > 1U) {
                    gitCommit->msg = "Merge :";
                    gitCommit->parentIds.clear();
                    gitCommit->isMerging = true;
                    for (uint32_t i = 0U; i < count_parent; ++i) {
                        auto oid = git_commit_parent_id(commit, i);
                        {  // append short id to merge message
                            git_oid_tostr(buf, 8, oid);
                            auto parent_short_id = std::string(buf);
                            gitCommit->msg += " " + parent_short_id;
                        }
                        {  // add long id to parent ids container
                            git_oid_tostr(buf, sizeof(buf), oid);
                            auto parent_long_id = std::string(buf);
                            gitCommit->parentIds.emplace(parent_long_id);
                        }
                    }
                    gitCommit->msg += "\n";
                } else if (count_parent == 1U) {
                    gitCommit->parentIds.clear();
                    auto oid = git_commit_parent_id(commit, 0);
                    git_oid_tostr(buf, sizeof(buf), oid);
                    auto parent_long_id = std::string(buf);
                    gitCommit->parentIds.emplace(parent_long_id);
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

                if (vBanchType != GIT_TAG_TARGET) {
                    gitCommit->branch = GetBranch(m_CurrentHistoryBranch);
                    auto branch_ptr = gitCommit->branch.lock();
                    assert(branch_ptr != nullptr);
                    gitCommit->branchPtrId = (intptr_t)(branch_ptr.get());
                } else {
                    currentBranch->name = "deleted";
                    gitCommit->branch = currentBranch;
                }

                currentBranch->nUnCommits++;

                // conversion in one line only
                gitCommit->msgOneLine = gitCommit->msg;
                auto el = gitCommit->msgOneLine.find('\n');
                if (el != std::string::npos) {
                    gitCommit->msgOneLine = gitCommit->msgOneLine.substr(0, el);
                }

                if (m_CommitsDatabase.empty()) {
                    gitCommit->isRoot = true;  // first commit
                }
                
                if (vBanchType == GIT_TAG_TARGET) {
                    gitCommit->isTag = true;
                    m_Tags[vBranchName] = gitCommit;
                }

                m_CommitsDatabase[gitCommit->id] = gitCommit;
                currentBranch->lastCommit = gitCommit;
            } else {
                gitCommit = m_CommitsDatabase[commitId];
            }
        }
    }

    git_revwalk_free(s.walker);
}

/// <summary>
/// this function will explor the parent until be on the level 0
/// </summary>
/// <param name="vCommitPtr"></param>
/// <param name="vLevel"></param>
void recurs_explore_merging(GitCommit *vCommitPtr) {

}

void GitRepositery::m_FinalizeRetrieveHistory(const std::string& vBranchName) {
    m_History.clear();

    if (vBranchName.empty()) {
        for (const auto& c : m_CommitsDatabase) {
            m_History.try_add(c.second->id, c.second);
        }
    } else {
        for (const auto& c : m_CommitsDatabase) {
            auto branch_ptr = c.second->branch.lock();
            if (branch_ptr != nullptr) {
                if (branch_ptr->name == vBranchName) {
                    m_History.try_add(c.second->id, c.second);
                }
            }
        }
    }

    std::sort(m_History.begin(), m_History.end(), [](const GitCommitWeak& a, const GitCommitWeak& b) -> bool {
        assert(!a.expired());
        assert(!b.expired());
        const auto& pa = a.lock();
        const auto& pb = b.lock();
        if (pa->dateEpoch == pb->dateEpoch)
            return (pa->id > pb->id);
        return (pa->dateEpoch > pb->dateEpoch);
    });

    // consolide childs from parents
    for (const auto& c : m_CommitsDatabase) {
        auto& childCommitPtr = c.second;
        if (childCommitPtr != nullptr) {
            for (const auto& parent_id : childCommitPtr->parentIds) {
                if (m_CommitsDatabase.find(parent_id) != m_CommitsDatabase.end()) {  // found
                    auto& parentCommitPtr = m_CommitsDatabase.at(parent_id);
                    if (parentCommitPtr != nullptr) {
                        parentCommitPtr->childIds.emplace(childCommitPtr->id);
                    }
                }
            }
        }
    }
    
    m_ConfigureHistoryForGraph();

    m_CurrentHistoryBranch = vBranchName;

    // set last commit from branchs as last commit
    for (const auto& type : m_Branchs) {
        for (const auto& branch : type.second) {
            if (branch.second != nullptr) {
                auto last_commit_ptr = branch.second->lastCommit.lock();
                if (last_commit_ptr != nullptr) {
                    last_commit_ptr->isLastCommit = true;
                }
            }
        }
    }
}

// https://www.shadertoy.com/view/ld3fzf
static inline ct::fvec4 GetRainBow(float r) {
    ct::fvec4 c(0.0f, 23.0f, 21.0f, 1.0f);
    return ct::cos(c + r * 6.3f) * 0.5f + 0.5f;
}

void GitRepositery::m_RetrieveBranchs() {
    m_Branchs.clear();
    m_BranchColors.clear();

    git_branch_iterator* it;
    if (!git_branch_iterator_new(&it, m_Repo, GIT_BRANCH_ALL)) {
        git_reference* ref;
        git_branch_t type;
        while (!git_branch_next(&ref, &type, it)) {
            const char* name;
            if (git_branch_name(&name, ref) == 0) {
                auto ptr = std::make_shared<GitBranch>();
                ptr->name = name;
                if (type == git_branch_t::GIT_BRANCH_LOCAL) {
                    ptr->target = GIT_LOCAL_TARGET;
                    if (git_branch_is_head(ref) == 1) {
                        m_HeadBranch = ptr;
                    }
                    m_Branchs[ptr->target][ptr->name] = ptr;
                }else {
                    auto arr = ct::splitStringToVector(ptr->name, "/");
                    if (arr.size() == (size_t)2) {
                        ptr->target = arr.at(0);
                        ptr->name = arr.at(1);
                        if (ptr->name != "HEAD") {
                            m_Branchs[ptr->target][ptr->name] = ptr;
                        }
                    } else {
                        LogVarError("Target have more than two names : %s", ptr->name.c_str());
                    }
                }
            }
            git_reference_free(ref);
        }
        git_branch_iterator_free(it);
    }

    float idx = 1.0f;
    float count = (float)(m_Branchs.size() + 1);
    for (auto& type : m_Branchs) {
        for (auto& branch : type.second) {
            branch.second->color = GetRainBow(idx++ / count);
            m_BranchColors.push_back(branch.second->color);
        }
    }
}

void GitRepositery::m_RetrieveTags() {
    git_reference_iterator* it;
    git_oid oid;
    char oid_str[GIT_OID_HEXSZ + 1];
    if (git_reference_iterator_new(&it, m_Repo) == 0) {
        const char* refname;
        while (!git_reference_next_name(&refname, it)) {
            std::string name = refname;
            if (name.find("refs/tags/") != std::string::npos) {
                if (!git_reference_name_to_id(&oid, m_Repo, refname)) {
                    if (git_oid_tostr(oid_str, GIT_OID_HEXSZ + 1, &oid) != nullptr) {
                        ct::replaceString(name, "refs/tags/", "");
                        std::string sha1 = oid_str;
                        m_Tags[name];
                    }
                }
            } 
        }
        git_reference_iterator_free(it);
    }
}

void GitRepositery::m_CommitRunner(const GitHistoryCollection& vCollection, GitCommitWeak vCommit, const int32_t& vLevel) {
    auto commit_ptr = vCommit.lock();
    if (commit_ptr != nullptr) {
        commit_ptr->startNodeColumn = vLevel;
        // we fill empty columns with a empty node
        while (commit_ptr->nodeColumns.size() < vLevel) {
            commit_ptr->nodeColumns.push_back(GitGraphNodeEnum::TRAVERSAL_NO_NODE);
        }
        // no parents so its a start node
        if (commit_ptr->parentIds.empty()) {
            commit_ptr->nodeColumns.push_back(GitGraphNodeEnum::START_NODE);
        } else {
            // no childs sp its a end node
            if (commit_ptr->childIds.empty()) {
                commit_ptr->nodeColumns.push_back(GitGraphNodeEnum::END_NODE);
            } else {
                // else what else than a traversal node
                commit_ptr->nodeColumns.push_back(GitGraphNodeEnum::TRAVERSAL_NODE);
                // if 2 or more childs and no root line we collapse and stop the runner
                if (commit_ptr->childIds.size() > 1 && vLevel > 0) {
                    commit_ptr->nodeColumns.push_back(GitGraphNodeEnum::CORNER_LEFT_TOP);
                    commit_ptr->collapsed = true;
                    // we are at the end of way so we stop the runner
                    return;
                }
            }
            // start or continue recursion with one new runner for each parents
            size_t idx = 0;
            GitCommitWeak commit;
            for (const auto& parent_id : commit_ptr->parentIds) {
                if (vCollection.getValue(parent_id, commit)) {
                    m_CommitRunner(vCollection, commit, vLevel + idx);
                    if (idx > 0) {
                        commit_ptr->nodeColumns.push_back(GitGraphNodeEnum::CORNER_BOTTOM_LEFT);
                        commit_ptr->splitted = true;
                    }
                }
                ++idx;
            }
        }
    }
}

void GitRepositery::m_ConfigureHistoryForGraph() {
    if (!m_History.empty()) {
        m_CommitRunner(m_History, *m_History.begin(), 0);

        /*
        * o
        * |
        * o
        * |\
        * | o
        * o x <- to fill
        * | o
        * |/
        * o
        * |
        * o
        */

        // then fill holes on last columns
        GitCommit* last_commit_ptr = nullptr;
        for (const auto& commit : m_History) {
            auto commit_ptr = commit.lock();
            if (commit_ptr != nullptr) {
                // we fill empty columns with a empty node
                if (last_commit_ptr != nullptr && !last_commit_ptr->collapsed) {
                    while (commit_ptr->nodeColumns.size() < last_commit_ptr->nodeColumns.size()) {
                        commit_ptr->nodeColumns.push_back(GitGraphNodeEnum::TRAVERSAL_NO_NODE);
                    }
                }
            }
            last_commit_ptr = commit_ptr.get();
        }
    }
}