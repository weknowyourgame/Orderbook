#include "using.h"
#include "order.h"

class OrderModify {
    private:
        OrderId orderId_;
        Price newPrice_;
        Quantity newQuantity_;
        Side side_;

    public:
        OrderModify(OrderId orderId, Price newPrice, Quantity newQuantity, Side side) 
            : orderId_(orderId)
            , newPrice_(newPrice)
            , newQuantity_(newQuantity)
            , side_(side)
        {
            if (newQuantity_ <= 0) {
                throw std::invalid_argument("Quantity must be positive");
            }
            if (newPrice_ <= 0) {
                throw std::invalid_argument("Price must be positive");
            }
        }

        OrderId GetOrderId() const { return orderId_; }
        // Side won't be changed, order needs to be cancelled and a new order needs to be placed
        Price GetNewPrice() const { return newPrice_; }
        Quantity GetNewQuantity() const { return newQuantity_; }
        Side GetSide() const { return side_; }

        OrderPointer ToOrderPointer(OrderType orderType) {
            return std::make_shared<Order>(
                GetOrderId(),
                GetNewPrice(),
                GetNewQuantity(),
                GetSide(),
                orderType
            );
        }

        void Modify(Price newPrice, Quantity newQuantity) {
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