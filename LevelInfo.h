#pragma once
#include "using.h"
#include <vector>

// Per level info -> shown in an arr like L1 and L2
struct LevelInfo
{
	Price price_;
	Quantity quantity_;
};

using LevelInfos = std::vector<LevelInfo>;
