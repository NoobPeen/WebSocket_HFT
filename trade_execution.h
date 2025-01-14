#ifndef TRADE_EXECUTION_H
#define TRADE_EXECUTION_H

#include "websocket_handler.h"
#include <nlohmann/json.hpp>
#include <string>
#include <functional>
#include <map>
#include <atomic>

// Forward declaration to avoid circular dependency
class WebSocketHandler;

using json = nlohmann::json;

class TradeExecution {
public:
   explicit TradeExecution(WebSocketHandler& websocket); 
   ~TradeExecution();

   
    json getOrderDetails(const std::string& order_id);
    json authenticate(const std::string& client_id, const std::string& client_secret);
    json getInstruments(const std::string& currency, const std::string& kind, bool expired);
    json placeBuyOrder(const std::string& instrument_name, double amount, double price);
    json cancelOrder(const std::string& order_id);
    json modifyOrder(const std::string& order_id, double new_price, double new_amount);
    json getOrderBook(const std::string& instrument_name);
    json getPosition(const std::string& instrument_name);
    void subscribeToOrderBook(const std::string& instrument_name, const std::string& interval = "agg2");
    void unsubscribeFromOrderBook(const std::string& instrument_name);
    void handleOrderBookUpdate(const json& update);

    // Market Data Handling
    void handleMarketData(const json& data);
    void onMarketDataReceived(const json& market_data);

    // Subscriber Management
    void addMarketDataSubscriber(const std::string& symbol, std::function<void(const json&)> callback);

private:
   WebSocketHandler& websocket_;

    std::map<std::string, std::function<void(const json&)>> market_data_subscribers_;
    static std::atomic<int> request_id;
    int getNextRequestId();
};

#endif // TRADE_EXECUTION_H


