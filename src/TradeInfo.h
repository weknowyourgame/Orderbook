#pragma once
#include "using.h"

// Info about trades that happened
struct TradeInfo {
	OrderId orderId_;
	Price price_;
	Quantity quantity_;
};
