#pragma once
#include "using.h"
#include "orderInfo.h"

struct TradeInfo {
	Price price_;
	Quantity quantity_;
	Side side_;
	OrderId orderid_;

	Side sideInfo(){ return side_; }
};
