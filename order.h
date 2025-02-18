#pragma once
#include "using.h"
#include "orderInfo.h"
#include <chrono>
#include <stdexcept>
#include <format>
#include <vector>

class Order {
	public:
	  Order(OrderId id, Price p, Quantity q, Side s, OrderStatus status, OrderType type)
          : orderid_{ id },
			price_{ p },
			side_{ s },
			orderstatus_{ status },
			ordertype_{ type },
			quantity_{ q },
			intialQuantity_{ q },
			remainingQuantity_{ q },
			filledQuantity_{ 0 },
			filledPrice_{ p },
			filledTime_{ std::chrono::steady_clock::now() },
    	  { }
	    
		OrderId GetOrderId() const { return orderid_; }
		Price GetPrice() const { return price_; }
		Quantity GetQuantity() const { return quantity_; }
		Side GetSide() const { return side_; }
		OrderStatus GetOrderStatus() const { return orderstatus_; }
		OrderType GetOrderType() const { return ordertype_; }
		Quantity GetInitialQuantity() const { return intialQuantity_; }
		Quantity GetRemainingQuantity() const { return remainingQuantity_; }
		Quantity GetFilledQuantity() const { return filledQuantity_; }
		Price GetFilledPrice() const { return filledPrice_; }
		std::chrono::steady_clock::time_point GetFilledTime() const { return filledTime_; }
		bool IsFilled() const { return remainingQuantity_ == 0; }

		void Cancel() {
			orderstatus_ = OrderStatus::CANCELED;
		}
		
		void FillOrder(Quantity q, Price p) {
			filledQuantity_ += q;
			remainingQuantity_ -= q;
			filledPrice_ = p;

			if (IsFilled()) {
				orderstatus_ = OrderStatus::FILLED;
			} else {
				orderstatus_ = OrderStatus::PARTIALLY_FILLED;
			}
			filledTime_ = std::chrono::steady_clock::now();
			filledPrice_ = p;
		}
    void Fill(Quantity quantity)
    {
        if (quantity > GetRemainingQuantity())
            throw std::logic_error(std::format("Order ({}) cannot be filled for more than its remaining quantity.", GetOrderId()));

        remainingQuantity_ -= quantity;
    }
    void ToGoodTillCancel(Price price) 
    { 
        if (GetOrderType() != OrderType::Market)
            throw std::logic_error(std::format("Order ({}) cannot have its price adjusted, only market orders can.", GetOrderId()));

        price_ = price;
        ordertype_ = OrderType::GoodTillCancel;
    }

	private:
	  OrderId orderid_;
	  Price price_;
	  Quantity quantity_;
	  Quantity intialQuantity_;
	  Quantity remainingQuantity_;
	  Quantity filledQuantity_;
	  Price filledPrice_;
	  std::chrono::steady_clock::time_point filledTime_;
	  Side side_;
	  OrderStatus orderstatus_;
	  OrderType ordertype_;
};

using Orders = std::vector<Order>;