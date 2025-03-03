#pragma once
#include "TradeInfo.h"
#include "using.h"
#include <vector>

// aggregation of 2 tradeInfo objects -> for a trade to happen in the orderbook -> a bid order needs to be filled by an ask
class Trade {
    private:
    TradeInfo bidTrade_;
    TradeInfo askTrade_;
    
    public:
        Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade)
            : bidTrade_{ bidTrade }
            , askTrade_{ askTrade }
        { }

    const TradeInfo& GetBidTrade() const { return bidTrade_; }
    const TradeInfo& GetAskTrade() const { return askTrade_; }
};

// Per level info -> shown in an arr like L1 and L2
struct LevelInfo
{
	Price price_;
	Quantity quantity_;
};

using LevelInfos = std::vector<LevelInfo>;
using Trades = std::vector<Trade>;