#include "orderbook.h"
#include "TerminalColors.h"
#include <thread>
#include <random>
#include <iomanip>
#include <chrono>
#include <mutex>
#include <atomic>
#include <condition_variable>

std::mutex consoleMutex;
std::atomic<int> orderCount{0};
std::atomic<int> tradeCount{0};
std::chrono::time_point<std::chrono::steady_clock> lastResetTime;

class OrderGenerator {
private:
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<> price_dist;
    std::uniform_int_distribution<> qty_dist;
    std::uniform_int_distribution<> side_dist;
    std::uniform_int_distribution<> type_dist;
    OrderId nextOrderId{1};
    std::mutex mutex_;

public:
    OrderGenerator() 
        : gen(rd())
        , price_dist(80, 120)
        , qty_dist(1, 1000)
        , side_dist(0, 1)
        , type_dist(0, 4) {}

    OrderPointer generateOrder() {
        Price price = price_dist(gen);
        Quantity qty = qty_dist(gen);
        Side side = side_dist(gen) == 0 ? Side::BID : Side::ASK;
        OrderType type = static_cast<OrderType>(type_dist(gen));
        
        std::lock_guard<std::mutex> lock(mutex_);
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
    
    std::cout << BOLD << "╔════════════════════════════════╗" << RESET << "\n";
    std::cout << BOLD << "║         ORDER BOOK             ║" << RESET << "\n";
    std::cout << BOLD << "╠════════════════════════════════╣" << RESET << "\n";
    
    auto asks = book.GetAsks(5);
    std::cout << RED << "║ SELLS:                         ║" << RESET << "\n";
    for (const auto& level : asks) {
        std::cout << RED << "║ " << std::setw(8) << level.price_ 
                  << " | " << std::setw(8) << level.quantity_ 
                  << "        ║" << RESET << "\n";
    }
    
    std::cout << BOLD << "╠════════════════════════════════╣" << RESET << "\n";
    
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
    try {
        std::cout << BOLD << "TRADE: " << RESET;
        std::cout << "Price: " << trade.GetBidTrade().price_ 
                  << " Quantity: " << trade.GetBidTrade().quantity_ << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error in displayTrade: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown error in displayTrade" << std::endl;
    }
}

void displayMetrics() {
    using namespace TerminalColors;
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastResetTime).count();
    
    if (elapsed >= 1) {
        std::cout << BOLD << "Orders/sec: " << orderCount.load() / elapsed
                  << " | Trades/sec: " << tradeCount.load() / elapsed << RESET << "\n";
        
        orderCount.store(0);
        tradeCount.store(0);
        lastResetTime = now;
    }
}

void orderProcessingThread(OrderBook& book, OrderGenerator& generator, std::atomic<bool>& running) {
    // Thread-local random generator for thread safety
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 99);
    
    while (running) {
        auto order = generator.generateOrder();
        Trades trades;
        try {
            trades = book.AddOrder(order);
            
            orderCount++;
            if (!trades.empty()) {
                tradeCount += trades.size();
                
                // Display trades when they occur
                std::lock_guard<std::mutex> lock(consoleMutex);
                for (const auto& trade : trades) {
                    displayTrade(trade);
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error adding order: " << e.what() << std::endl;
            continue;
        }
        
        // Use thread-safe random generation
        if (dist(gen) == 0) {
            std::lock_guard<std::mutex> lock(consoleMutex);
            std::cout << "\nNew " 
                      << (order->GetSide() == Side::BID ? "BUY" : "SELL")
                      << " Order: Price=" << order->GetPrice()
                      << " Qty=" << order->GetInitialQuantity() << "\n";
        }

        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

void displayThread(OrderBook& book, std::atomic<bool>& running) {
    lastResetTime = std::chrono::steady_clock::now();
    
    while (running) {
        try {
            std::lock_guard<std::mutex> lock(consoleMutex);
            displayOrderBook(book);
            displayMetrics();
        } catch (const std::exception& e) {
            std::cerr << "Error in display thread: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown error in display thread" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    try {
        OrderBook book;
        OrderGenerator generator;
        
        std::atomic<bool> running{true};
        
        std::thread processingThread(orderProcessingThread, std::ref(book), std::ref(generator), std::ref(running));
        std::thread displayThreadHandle(displayThread, std::ref(book), std::ref(running));
        
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        
        running = false;
        
        if (processingThread.joinable()) {
            processingThread.join();
        }
        
        if (displayThreadHandle.joinable()) {
            displayThreadHandle.join();
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred" << std::endl;
        return 2;
    }
}