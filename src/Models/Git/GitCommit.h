#pragma once

#include <string>
#include <set>
#include <vector>
#include <memory>

#include <Headers/globals.h>

enum class GitGraphNodeEnum {
    NONE = 0,               // empty
    TRAVERSAL_NO_NODE,      // from bottom to top without node
    TRAVERSAL_NODE,         // from bottom to top with node
    START_NODE,             // from middle to top with node
    END_NODE,               // from bottom to middle with node
    CORNER_LEFT_TOP,        // a Corner from left to top (split)
    CORNER_BOTTOM_LEFT,     // a Corner from bottom to left (collapse)
    CORNER_BOTTOM_LEFT_TOP  // a Corner from bottom to left¨and from left to top (collapse / split)
};

typedef int64_t GitDate;

class GitCommit {
public:
    std::string id;                             // commit id
    std::string idShort;                        // commit id on 7 chars
    std::string authorName;                     // author name
    std::string authorMail;                     // author mail
    std::string date;                           // date
    GitDate dateEpoch = 0;                      // date in format epoh
    std::string msg;                            // message
    std::string msgOneLine;                     // message in one line (only the first line, unitl '\n')
    GitBranchWeak branch;                       // branch of the commit
    intptr_t branchPtrId = 0;                   // branch ptr conversion ot intptr for avoid calculation in loops
    std::set<std::string> branchs;              // branchs appartenance
    std::set<std::string> parentIds;            // parent short ids (mini => 1, maxi => no limit, but in usual cases, its 2)
    std::set<std::string> childIds;             // child short ids (mini => 1, maxi => no limit, but in usual cases, its 2)
    size_t msgSize = 0;                         // message length/size
    size_t startNodeColumn = 0U;                // column for the graph node
    std::vector<GitGraphNodeEnum> nodeColumns;  // type de node de graph
    bool showBranchText = false;                // show the branch text
    bool isRoot = false;                        // first commit
    bool collapsed = false;                     // collapsed on parent
    bool splitted = false;                      // splitted on two childs
    bool isNewbranch = false;                   // just new branch is created
    bool isMerging = false;                     // the commit is merging
    bool isLastCommit = false;                  // last commit fo current branch
    bool isTag = false;                         // is a tag

public:
    GitCommit();
    ~GitCommit();
};
