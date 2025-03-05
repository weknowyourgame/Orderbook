#include <unordered_map>
#include <map>
#include <list>
#include "order.h"
#include "Trade.h"
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

        std::map<Price, LevelInfo> data_;

        void UpdateDataMap(Price price, Quantity quantity, Side side, bool isAdd) {
            if (isAdd) {
                data_[price].price_ = price;
                data_[price].quantity_ += quantity;
            } else {
                data_[price].quantity_ -= quantity;
                if (data_[price].quantity_ == 0) {
                    data_.erase(price);
                }
            }
        }

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

                // Get the best bid and ask
                auto bidIt = _bids.begin();
                auto askIt = _asks.begin();
                
                if (bidIt == _bids.end() || askIt == _asks.end()) break;
                
                Price bidPrice = bidIt->first;
                Price askPrice = askIt->first;
                
                // Check if they can match
                if (bidPrice < askPrice) break;

                auto& bids = bidIt->second;
                auto& asks = askIt->second;
                
                // Safety check for empty containers
                if (bids.empty() || asks.empty()) {
                    if (bids.empty()) _bids.erase(bidPrice);
                    if (asks.empty()) _asks.erase(askPrice);
                    continue;
                }

                while (!bids.empty() && !asks.empty()) {
                    auto bid = bids.front();
                    auto ask = asks.front();
                    
                    // Safety check for null pointers
                    if (!bid || !ask) {
                        if (!bid) bids.erase(bids.begin());
                        if (!ask) asks.erase(asks.begin());
                        continue;
                    }
                    
                    Quantity quantity = std::min(bid->GetRemainingQuantity(), ask->GetRemainingQuantity());

                    // Fill the orders
                    try {
                        bid->FillOrder(quantity);
                        ask->FillOrder(quantity);
                        
                        // Update data map if needed
                        UpdateDataMap(bidPrice, quantity, Side::BID, false);
                        UpdateDataMap(askPrice, quantity, Side::ASK, false);
                        
                        // Create trade record
                        trades.push_back(Trade{
                            TradeInfo{bid->GetOrderId(), bidPrice, quantity},
                            TradeInfo{ask->GetOrderId(), askPrice, quantity}
                        });
                        
                        // Remove filled orders
                        if (bid->IsFilled()) {
                            bids.erase(bids.begin());
                            orders_.erase(bid->GetOrderId());
                        }
                        
                        if (ask->IsFilled()) {
                            asks.erase(asks.begin());
                            orders_.erase(ask->GetOrderId());
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Error during order matching: " << e.what() << std::endl;
                        break;
                    }
                }

                // Clean up empty price levels
                if (bids.empty()) {
                    _bids.erase(bidPrice);
                    data_.erase(bidPrice);
                }

                if (asks.empty()) {
                    _asks.erase(askPrice);
                    data_.erase(askPrice);
                }

                // Handle FillAndKill orders
                if (!bids.empty()) {
                    auto& order = bids.front();
                    if (order && order->GetOrderType() == OrderType::FillAndKill) {
                        CancelOrder(order->GetOrderId());
                    }
                }

                if (!asks.empty()) {
                    auto& order = asks.front();
                    if (order && order->GetOrderType() == OrderType::FillAndKill) {
                        CancelOrder(order->GetOrderId());
                    }
                }
            }
            return trades;
        }

        void CancelOrderInternal(OrderId orderId) {
            if (!orders_.contains(orderId)) return;

            const auto& [order, iterator] = orders_.at(orderId);
            orders_.erase(orderId);

            if (order->GetSide() == Side::BID) {
                auto& orders = _bids[order->GetPrice()];
                UpdateDataMap(order->GetPrice(), order->GetRemainingQuantity(), Side::BID, false);
                orders.erase(iterator);
                if (orders.empty()) _bids.erase(order->GetPrice());
            } else {
                auto& orders = _asks[order->GetPrice()];
                UpdateDataMap(order->GetPrice(), order->GetRemainingQuantity(), Side::ASK, false);
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
                UpdateDataMap(order->GetPrice(), order->GetRemainingQuantity(), Side::BID, true);
            } else {
                auto& orders = _asks[order->GetPrice()];
                orders.push_back(order);
                iterator = std::prev(orders.end());
                UpdateDataMap(order->GetPrice(), order->GetRemainingQuantity(), Side::ASK, true);
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