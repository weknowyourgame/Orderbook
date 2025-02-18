#pragma once

enum class Side
{
	ASK,
	BID
};
enum class OrderStatus
{
	FILLED,
	NEW,
	PARTIALLY_FILLED
};
enum class OrderType
{
	GoodTillCancel,
	FillAndKill,
	FillOrKill,
	GoodForDay,
	Market
};
