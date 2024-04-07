#pragma once

#include <memory>

#define GIT_LOCAL_TARGET "local"
#define GIT_TAG_TARGET "TAG"

class GitBranch;
typedef std::shared_ptr<GitBranch> GitBranchPtr;
typedef std::weak_ptr<GitBranch> GitBranchWeak;

class GitCommit;
typedef std::shared_ptr<GitCommit> GitCommitPtr;
typedef std::weak_ptr<GitCommit> GitCommitWeak;

class GitRepositery;
typedef std::shared_ptr<GitRepositery> GitRepositeryPtr;
typedef std::weak_ptr<GitRepositery> GitRepositeryWeak;
