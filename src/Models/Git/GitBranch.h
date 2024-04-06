#pragma once

#include <ctools/cTools.h>

#include <string>

class GitBranch;
typedef std::shared_ptr<GitBranch> GitBranchPtr;
typedef std::weak_ptr<GitBranch> GitBranchWeak;

class GitBranch
{
public:
	std::string name;			// name of the branch
	size_t nCommits = 0U;		// commits can be sahred with other branch
	size_t nUnCommits = 0U;		// commits who are only in this branch
	size_t graphColumn = 0U;	// column for the graph node
	ct::fvec4 color;			// color of the branch

public:
	GitBranch();
	~GitBranch();
};