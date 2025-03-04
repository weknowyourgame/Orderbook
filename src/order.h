#pragma once
#include "using.h"
#include <chrono>
#include <stdexcept>
#include <format>
#include <vector>
#include <mutex>
#include <memory>

class Order {
private:
    OrderId orderid_;
    Price price_;
    Quantity initialQuantity_;
    Quantity remainingQuantity_;
    Side side_;
    OrderType ordertype_;
    std::mutex mutex_;
    
public:
    Order(OrderId id, Price p, Quantity q, Side s, OrderType type)
        : orderid_{ id }
        , price_{ p }
        , initialQuantity_{ q }
        , remainingQuantity_{ q }
        , side_{ s }
        , ordertype_{ type }
    {}
    
    OrderId GetOrderId() const { return orderid_; }
    Price GetPrice() const { return price_; }
    Quantity GetInitialQuantity() const { return initialQuantity_; }
    Quantity GetRemainingQuantity() const { return remainingQuantity_; }
    Side GetSide() const { return side_; }
    OrderType GetOrderType() const { return ordertype_; }
    bool IsFilled() const { return remainingQuantity_ == 0; }
    
    void FillOrder(Quantity quantity) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (quantity > GetRemainingQuantity())
            throw std::logic_error(std::format("Order ({}) cannot be filled for more than its remaining quantity.", GetOrderId()));
        remainingQuantity_ -= quantity;
    }

    void ToGoodTillCancel(Price price) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (ordertype_ == OrderType::Market) {
            throw std::logic_error(std::format("Order ({}) cannot be adjusted; market orders cannot become GoodTillCancel.", orderid_));
        }
        price_ = price;
        ordertype_ = OrderType::GoodTillCancel;
    }
};

using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::vector<OrderPointer>;
class OrderModify {
    private:
        OrderId orderId_;
        Price newPrice_;
        Quantity newQuantity_;
        Side side_;
        std::mutex mutex_;
    public:
        OrderModify(OrderId orderId, Price newPrice, Quantity newQuantity, Side side) 
            : orderId_(orderId)
            , newPrice_(newPrice)
            , newQuantity_(newQuantity)
            , side_(side)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (newQuantity_ <= 0) {
                throw std::invalid_argument("Quantity must be positive");
            }
            if (newPrice_ <= 0) {
                throw std::invalid_argument("Price must be positive");
            }
        }

        OrderId GetOrderId() const { return orderId_; }
        Price GetNewPrice() const { return newPrice_; }
        Quantity GetNewQuantity() const { return newQuantity_; }
        Side GetSide() const { return side_; }

        OrderPointer ToOrderPointer(OrderType orderType) {
            std::lock_guard<std::mutex> lock(mutex_);
            return std::make_shared<Order>(
                GetOrderId(),
                GetNewPrice(),
                GetNewQuantity(),
                GetSide(),
                orderType
            );
        }

        void Modify(Price newPrice, Quantity newQuantity) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (newQuantity <= 0) {
                throw std::invalid_argument("Quantity must be positive");
            }
            if (newPrice <= 0) {
                throw std::invalid_argument("Price must be positive");
            }
            newPrice_ = newPrice;
            newQuantity_ = newQuantity;
        }
    };
