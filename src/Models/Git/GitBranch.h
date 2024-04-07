#pragma once

#include <ctools/cTools.h>
#include <Headers/globals.h>

#include <string>

class GitBranch {
public:
    std::string name;          // name of the branch
    size_t nCommits = 0U;      // commits can be sahred with other branch
    size_t nUnCommits = 0U;    // commits who are only in this branch
    size_t graphColumn = 0U;   // column for the graph node
    ct::fvec4 color;           // color of the branch
    std::string target;        // target : LOCAL or another remote name
    GitCommitWeak lastCommit;  // the alst commit of the branch
    bool isDefault = false;    // default branch

public:
    GitBranch();
    ~GitBranch();
};
