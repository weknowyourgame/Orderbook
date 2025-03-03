#include "orderbook.h"
#include "TerminalColors.h"
#include <thread>
#include <random>
#include <iomanip>
#include <chrono>

class OrderGenerator {
private:
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<> price_dist;
    std::uniform_int_distribution<> qty_dist;
    std::uniform_int_distribution<> side_dist;
    std::uniform_int_distribution<> type_dist;
    OrderId nextOrderId{1};

public:
    OrderGenerator() 
        : gen(rd())
        , price_dist(90, 110)
        , qty_dist(1, 100)
        , side_dist(0, 1)
        , type_dist(0, 4) {}

    OrderPointer generateOrder() {
        Price price = price_dist(gen);
        Quantity qty = qty_dist(gen);
        Side side = side_dist(gen) == 0 ? Side::BID : Side::ASK;
        OrderType type = static_cast<OrderType>(type_dist(gen));
        
        return std::make_shared<Order>(
            nextOrderId++,
            price,
            qty,
            side,
            type
        );
    }
};

void displayOrderBook(const OrderBook& book) {
    using namespace TerminalColors;
    std::cout << CLEAR_SCREEN;
    
    // Header
    std::cout << BOLD << "╔════════════════════════════════╗" << RESET << "\n";
    std::cout << BOLD << "║         ORDER BOOK             ║" << RESET << "\n";
    std::cout << BOLD << "╠════════════════════════════════╣" << RESET << "\n";
    
    // Display asks (in red)
    auto asks = book.GetAsks(5);
    std::cout << RED << "║ SELLS:                         ║" << RESET << "\n";
    for (const auto& level : asks) {
        std::cout << RED << "║ " << std::setw(8) << level.price_ 
                  << " | " << std::setw(8) << level.quantity_ 
                  << "        ║" << RESET << "\n";
    }
    
    std::cout << BOLD << "╠════════════════════════════════╣" << RESET << "\n";
    
    // Display bids (in green)
    auto bids = book.GetBids(5);
    std::cout << GREEN << "║ BUYS:                          ║" << RESET << "\n";
    for (const auto& level : bids) {
        std::cout << GREEN << "║ " << std::setw(8) << level.price_ 
                  << " | " << std::setw(8) << level.quantity_ 
                  << "        ║" << RESET << "\n";
    }
    
    std::cout << BOLD << "╚════════════════════════════════╝" << RESET << "\n";
}

void displayTrade(const Trade& trade) {
    using namespace TerminalColors;
    std::cout << BOLD << "TRADE: " << RESET;
    std::cout << "Price: " << trade.GetBidTrade().price_ 
              << " Quantity: " << trade.GetBidTrade().quantity_ << "\n";
}

int main() {
    OrderBook book;
    OrderGenerator generator;
    
    while (true) {
        auto order = generator.generateOrder();
        
        // Display order type
        std::cout << "\nNew " 
                  << (order->GetSide() == Side::BID ? "BUY" : "SELL")
                  << " Order: Price=" << order->GetPrice()
                  << " Qty=" << order->GetQuantity() << "\n";
        
        auto trades = book.AddOrder(order);
        
        // Display the order book
        displayOrderBook(book);
        
        // Display trades if any
        if (!trades.empty()) {
            std::cout << "\nTrades executed:\n";
            for (const auto& trade : trades) {
                displayTrade(trade);
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
    return 0;
}