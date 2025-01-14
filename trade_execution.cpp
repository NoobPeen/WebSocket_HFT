#include "trade_execution.h"
#include "websocket_handler.h"
#include <iostream>
#include <stdexcept>
#include "latency_module.h"

std::atomic<int> TradeExecution::request_id{ 1 }; // Initialize static atomic counter

TradeExecution::TradeExecution(WebSocketHandler& websocket)
    : websocket_(websocket) {}

TradeExecution::~TradeExecution() {
    // Perform cleanup, such as clearing the subscribers
    market_data_subscribers_.clear();
}

// Helper function to generate the next unique request ID
int TradeExecution::getNextRequestId() {
    return request_id++;
}

// Method to handle incoming market data and notify subscribers
void TradeExecution::handleMarketData(const json& data) {
    if (data.contains("symbol")) {
        std::string symbol = data["symbol"];
        if (market_data_subscribers_.count(symbol)) {
            market_data_subscribers_[symbol](data);  // Call the subscriber's callback
        }
        else {
            std::cerr << "No subscribers for symbol: " << symbol << std::endl;
        }
    }
    else {
        std::cerr << "Invalid market data: Missing 'symbol'" << std::endl;
    }
}

// Method called when new market data is received
void TradeExecution::onMarketDataReceived(const json& market_data) {
    auto market_data_start = LatencyModule::start();  // Start the timer
    handleMarketData(market_data);
    LatencyModule::end(market_data_start, "Market Data Processing Latency");  // Measure latency
}

// Method to authenticate
json TradeExecution::authenticate(const std::string& client_id, const std::string& client_secret) {
    try {
        json auth_message = {
            {"jsonrpc", "2.0"},
            {"id", getNextRequestId()},
            {"method", "public/auth"},
            {"params", {
                {"grant_type", "client_credentials"},
                {"client_id", client_id},
                {"client_secret", client_secret}
            }}
        };
        websocket_.sendMessage(auth_message);
        auto response = websocket_.readMessage();

        if (!response.contains("result")) {
            throw std::runtime_error("Authentication failed: " + response.dump());
        }
        return response["result"];
    }
    catch (const std::exception& e) {
        std::cerr << "Error in authenticate: " << e.what() << std::endl;
        throw; // Re-throw for higher-level handling
    }
}

// Method to get available instruments
json TradeExecution::getInstruments(const std::string& currency, const std::string& kind, bool expired) {
    try {
        json request = {
            {"jsonrpc", "2.0"},
            {"id", getNextRequestId()},
            {"method", "public/get_instruments"},
            {"params", {{"currency", currency}, {"kind", kind}, {"expired", expired}}}
        };
        websocket_.sendMessage(request);
        return websocket_.readMessage();
    }
    catch (const std::exception& e) {
        std::cerr << "Error in getInstruments: " << e.what() << std::endl;
        throw;
    }
}

// Method to place a buy order
json TradeExecution::placeBuyOrder(const std::string& instrument_name, double amount, double price) {
    try {
        json buy_request = {
            {"jsonrpc", "2.0"},
            {"id", getNextRequestId()},
            {"method", "private/buy"},
            {"params", {
                {"instrument_name", instrument_name},
                {"amount", amount},
                {"type", "limit"},
                {"price", price}
            }}
        };
        websocket_.sendMessage(buy_request);
        auto response = websocket_.readMessage();
        
        // Add debug logging
        std::cout << "Buy order response: " << response.dump(2) << std::endl;
        
        return response;
    }
    catch (const std::exception& e) {
        std::cerr << "Error in placeBuyOrder: " << e.what() << std::endl;
        throw;
    }
}

// Method to cancel an order
json TradeExecution::cancelOrder(const std::string& order_id) {
    try {
        json cancel_request = {
            {"jsonrpc", "2.0"},
            {"id", getNextRequestId()},
            {"method", "private/cancel"},
            {"params", {{"order_id", order_id}}}
        };
        websocket_.sendMessage(cancel_request);
        return websocket_.readMessage();
    }
    catch (const std::exception& e) {
        std::cerr << "Error in cancelOrder: " << e.what() << std::endl;
        throw;
    }
}

// Method to modify an order
json TradeExecution::modifyOrder(const std::string& order_id, double new_price, double new_amount) {
    try {
        json modify_request = {
            {"jsonrpc", "2.0"},
            {"id", getNextRequestId()},
            {"method", "private/edit"},
            {"params", {
                {"order_id", order_id},
                {"new_price", new_price},
                {"new_amount", new_amount},
                {"contracts", new_amount}
            }}
        };
        websocket_.sendMessage(modify_request);
        return websocket_.readMessage();
    }
    catch (const std::exception& e) {
        std::cerr << "Error in modifyOrder: " << e.what() << std::endl;
        throw;
    }
}

// Method to get the order book for a specific instrument
json TradeExecution::getOrderBook(const std::string& instrument_name) {
    try {
        json request = {
            {"jsonrpc", "2.0"},
            {"id", getNextRequestId()},
            {"method", "public/get_order_book"},
            {"params", {{"instrument_name", instrument_name}}}
        };
        websocket_.sendMessage(request);
        return websocket_.readMessage();
    }
    catch (const std::exception& e) {
        std::cerr << "Error in getOrderBook: " << e.what() << std::endl;
        throw;
    }
}

// Method to get current positions
json TradeExecution::getPosition(const std::string& instrument_name) {
    try {
        json request = {
            {"jsonrpc", "2.0"},
            {"id", getNextRequestId()},
            {"method", "private/get_position"},
            {"params", {{"instrument_name", instrument_name}}}
        };
        websocket_.sendMessage(request);
        return websocket_.readMessage();
    }
    catch (const std::exception& e) {
        std::cerr << "Error in getPosition: " << e.what() << std::endl;
        throw;
    }
}

// Add a subscriber for real-time market data updates
void TradeExecution::addMarketDataSubscriber(const std::string& symbol, std::function<void(const json&)> callback) {
    market_data_subscribers_[symbol] = callback;
}

void TradeExecution::subscribeToOrderBook(const std::string& instrument_name, const std::string& interval) {
    try {
        json subscribe_request = {
            {"jsonrpc", "2.0"},
            {"id", getNextRequestId()},
            {"method", "private/subscribe"},
            {"params", {
                {"channels", {
                    "book." + instrument_name + "." + interval
                }}
            }}
        };
        websocket_.sendMessage(subscribe_request);
    }
    catch (const std::exception& e) {
        std::cerr << "Error subscribing to order book: " << e.what() << std::endl;
    }
}

void TradeExecution::unsubscribeFromOrderBook(const std::string& instrument_name) {
    try {
        json unsubscribe_request = {
            {"jsonrpc", "2.0"},
            {"id", getNextRequestId()},
            {"method", "public/unsubscribe_all"},
            {"params", {}}
        };
        websocket_.sendMessage(unsubscribe_request);
    }
    catch (const std::exception& e) {
        std::cerr << "Error unsubscribing: " << e.what() << std::endl;
    }
}

void TradeExecution::handleOrderBookUpdate(const json& update) {
    try {
        if (update.contains("params") && update["params"].contains("data")) {
            const auto& data = update["params"]["data"];
            std::cout << "\nOrder Book Update:" << std::endl;
            if (data.contains("bids")) {
                std::cout << "Bids:" << std::endl;
                for (const auto& bid : data["bids"]) {
                    std::cout << "Price: " << bid[0] << ", Size: " << bid[1] << std::endl;
                }
            }
            if (data.contains("asks")) {
                std::cout << "Asks:" << std::endl;
                for (const auto& ask : data["asks"]) {
                    std::cout << "Price: " << ask[0] << ", Size: " << ask[1] << std::endl;
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error handling order book update: " << e.what() << std::endl;
    }
}

json TradeExecution::getOrderDetails(const std::string& order_id) {
    try {
        json request = {
            {"jsonrpc", "2.0"},
            {"id", getNextRequestId()},
            {"method", "private/get_order_state"},
            {"params", {{"order_id", order_id}}}
        };
        websocket_.sendMessage(request);
        return websocket_.readMessage();
    }
    catch (const std::exception& e) {
        std::cerr << "Error getting order details: " << e.what() << std::endl;
        throw;
    }
}