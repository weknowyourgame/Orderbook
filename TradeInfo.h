#pragma once
#include "using.h"

struct TradeInfo {
	OrderId orderId_;
	Price price_;
	Quantity quantity_;

	Side sideInfo(){ return side_; }
};
