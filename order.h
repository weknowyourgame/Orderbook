#include <cstdint> 
#include "ulid.hh"

class Price {
    private:
        uint64_t price;
        int decimal;
    public:
    // Default decimal places: 6
    Price(uint64_t v, int d = 6) : price(v), decimal(d) {}
};

enum class Quote {
    ask,
    bid,
};

class Order {
    private:
        orderId = generateUUID;
        uint64_t quantity;
        Price price;

};

