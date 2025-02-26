#pragma once
#include "using.h"
#include <chrono>
#include <stdexcept>
#include <format>
#include <vector>

class Order {
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
	Quantity GetQuantity() const { return quantity_; }
	Side GetSide() const { return side_; }
	OrderType GetOrderType() const { return ordertype_; }
	Quantity GetInitialQuantity() const { return initialQuantity_; }
	Quantity GetRemainingQuantity() const { return remainingQuantity_; }
	bool IsFilled() const { return remainingQuantity_ == 0; }
	
	void FillOrder(Quantity quantity) {
			if (quantity > GetRemainingQuantity())
            throw std::logic_error(std::format("Order ({}) cannot be filled for more than its remaining quantity.", GetOrderId()));

        remainingQuantity_ -= quantity;
	}

    void ToGoodTillCancel(Price price) {
        if (ordertype_ != OrderType::Market) {
            throw std::logic_error(
				// format is a c++20 feature
                std::format("Order ({}) cannot have its price adjusted, only market orders can.", orderid_));
        }

        price_ = price;
        ordertype_ = OrderType::GoodTillCancel;
    }

	private:
	OrderId orderid_;
	Price price_;
	Quantity quantity_;
	Quantity initialQuantity_;
	Quantity remainingQuantity_;
	Side side_;
	OrderType ordertype_;
};

using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::vector<OrderPointer>;