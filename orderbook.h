#include <unordered_map>
#include <map>
#include <list>
#include "LevelInfo.h"
#include "order.h"
#include "orderModify.h"
#include "trade.h"
#include "using.h"

class OrderBook {
    private:
        struct OrderEntry {
            OrderPointer order_{nullptr};
            OrderPointers::iterator location_;
        };

        std::map<Price, OrderPointers, std::greater<Price>> _bids;
        std::map<Price, OrderPointers, std::less<Price>> _asks;
        std::unordered_map<OrderId, OrderEntry> orders_;

        bool CanMatch(Price price, Side side) {
            if (side == Side::BID) {
                if (_asks.empty()) return false;
                const auto& [bestAsk, _] = *_asks.begin();
                return price >= bestAsk;
            } else {
                if (_bids.empty()) return false;
                const auto& [bestBid, _] = *_bids.begin();
                return price <= bestBid;
            }
        }

        Trades MatchOrder() {
            Trades trades;

            while (true) {
                if (_bids.empty() || _asks.empty()) break;

                auto& [bidPrice, bids] = *_bids.begin();
                auto& [askPrice, asks] = *_asks.begin();
                if (bidPrice < askPrice) break;

                while (!bids.empty() && !asks.empty()) {
                    auto bid = bids.front();
                    auto ask = asks.front();
                    Quantity quantity = std::min(bid->GetRemainingQuantity(), ask->GetRemainingQuantity());

                    bid->FillOrder(quantity);
                    ask->FillOrder(quantity);

                    if (bid->IsFilled()) {
                        bids.erase(bids.begin());
                        orders_.erase(bid->GetOrderId());
                    }

                    if (ask->IsFilled()) {
                        asks.erase(asks.begin());
                        orders_.erase(ask->GetOrderId());
                    }

                    trades.push_back(Trade{
                        TradeInfo{bid->GetOrderId(), bidPrice, quantity},
                        TradeInfo{ask->GetOrderId(), askPrice, quantity}
                    });
                }

                if (bids.empty()) {
                    _bids.erase(bidPrice);
                    data_.erase(bidPrice);
                }

                if (asks.empty()) {
                    _asks.erase(askPrice);
                    data_.erase(askPrice);
                }

                if (!bids.empty()) {
                    auto& order = bids.front();
                    if (order->GetOrderType() == OrderType::FillAndKill) {
                        CancelOrder(order->GetOrderId());
                    }
                }

                if (!asks.empty()) {
                    auto& order = asks.front();
                    if (order->GetOrderType() == OrderType::FillAndKill) {
                        CancelOrder(order->GetOrderId());
                    }
                }
            }
            return trades;
        }

        std::map<Price, LevelInfo> data_;

        void CancelOrderInternal(OrderId orderId) {
            if (!orders_.contains(orderId)) return;

            const auto& [order, iterator] = orders_.at(orderId);
            orders_.erase(orderId);

            if (order->GetSide() == Side::BID) {
                auto& orders = _bids[order->GetPrice()];
                orders.erase(iterator);
                if (orders.empty()) _bids.erase(order->GetPrice());
            } else {
                auto& orders = _asks[order->GetPrice()];
                orders.erase(iterator);
                if (orders.empty()) _asks.erase(order->GetPrice());
            }
        }

    public:
        Trades AddOrder(OrderPointer order) {
            if (orders_.contains(order->GetOrderId())) return {};

            OrderPointers::iterator iterator;
            if (order->GetSide() == Side::BID) {
                auto& orders = _bids[order->GetPrice()];
                orders.push_back(order);
                iterator = std::prev(orders.end());
            } else {
                auto& orders = _asks[order->GetPrice()];
                orders.push_back(order);
                iterator = std::prev(orders.end());
            }

            orders_.insert({order->GetOrderId(), OrderEntry{order, iterator}});
            return MatchOrder();
        }

        void CancelOrder(OrderId orderId) {
            CancelOrderInternal(orderId);
        }

        Trades ModifyOrder(OrderModify order) {
            if (!orders_.contains(order.GetOrderId())) return {};

            const auto& [existingOrder, _] = orders_.at(order.GetOrderId());
            OrderType orderType = existingOrder->GetOrderType();

            CancelOrder(order.GetOrderId());
            return AddOrder(order.ToOrderPointer(orderType));
        }

        LevelInfos GetBids(size_t levels) const {
            LevelInfos result;
            size_t count = 0;
            for (const auto& [price, orders] : _bids) {
                if (count >= levels) break;
                Quantity totalQuantity = 0;
                for (const auto& order : orders) {
                    totalQuantity += order->GetRemainingQuantity();
                }
                result.push_back({price, totalQuantity});
                count++;
            }
            return result;
        }

        LevelInfos GetAsks(size_t levels) const {
            LevelInfos result;
            size_t count = 0;
            for (const auto& [price, orders] : _asks) {
                if (count >= levels) break;
                Quantity totalQuantity = 0;
                for (const auto& order : orders) {
                    totalQuantity += order->GetRemainingQuantity();
                }
                result.push_back({price, totalQuantity});
                count++;
            }
            return result;
        }
};