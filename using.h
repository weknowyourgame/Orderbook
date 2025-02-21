#pragma once
#include <iostream>
#include <cstdint>

enum class Side
{
	ASK,
	BID
};
enum class OrderStatus
{
	FILLED,
	NEW,
	PARTIALLY_FILLED, 
	CANCELLED
};
enum class OrderType
{
	GoodTillCancel,
	FillAndKill,
	FillOrKill,
	GoodForDay,
	Market
};

using Price = std::int64_t;
using Quantity = std::uint32_t;
using OrderId = std::uint64_t;
