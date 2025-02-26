#pragma once
#include "tradeInfo.h"

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

using Trades = std::vector<Trade>;