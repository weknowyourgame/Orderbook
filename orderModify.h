#include "using.h"

class OrderModify {
    private:
        OrderId orderId_;
        Price newPrice_;
        Quantity newQuantity_;
        Side side_;

    public:
        OrderModify(OrderId orderId, Price newPrice, Quantity newQuantity, Side side)
            : side_(side)
            , orderId_(orderId)
            , newPrice_(newPrice)
            , newQuantity_(newQuantity)
        {
            if (newQuantity_ <= 0) {
                throw std::invalid_argument("Quantity must be positive");
            }
            if (newPrice_ <= 0) {
                throw std::invalid_argument("Price must be positive");
            }
        }

        OrderId GetOrderId() const { return orderId_; }
        // Side wont be changed, order needs to be cancelled and a new order needs to be placed
        Side GetSide() const { return side_; }
        Price GetNewPrice() const { return newPrice_; }
        Quantity GetNewQuantity() const { return newQuantity_; }

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
