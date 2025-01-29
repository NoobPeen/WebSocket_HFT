#pragma once
#ifndef WEBSOCKET_HANDLER_H
#define WEBSOCKET_HANDLER_H

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <nlohmann/json.hpp>
#include <openssl/ssl.h>
#include <boost/beast/websocket.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/core.hpp>
#include <string>
#include "trade_execution.h"  // Include the TradeExecution header for access

namespace beast = boost::beast;
namespace asio = boost::asio;
namespace ssl = asio::ssl;
using tcp = asio::ip::tcp;
using json = nlohmann::json;

class WebSocketHandler : public std::enable_shared_from_this<WebSocketHandler> {
public:
    // Constructor now includes TradeExecution reference
    WebSocketHandler(asio::io_context& ioc, const std::string& host, 
                    const std::string& port, const std::string& endpoint);
    void subscribe(const std::string& channel);
    void unsubscribe(const std::string& channel);
    // Add this to the public section of the WebSocketHandler class
    void handleOrderBookUpdate(const json& data);
    void connect();
    void onMessage(const std::string& message); // Declare the onMessage function
    void sendMessage(const json& message);
    json readMessage();
    void close();

    
    // In websocket_handler.h
    void set_message_handler(std::function<void(const std::string&)> handler);
    void close_connection();  // renamed from close() to avoid confusion
    void async_connect(std::function<void(boost::system::error_code)> callback = nullptr);

private:
    asio::io_context& ioc_;
    ssl::context ctx_;
    tcp::resolver resolver_;
    beast::websocket::stream<ssl::stream<tcp::socket>> websocket_;
    std::string host_;
    std::string endpoint_;
    // TradeExecution& trade_execution_;  // Reference to TradeExecution object
    // In websocket_handler.h
    void start_read();
    std::function<void(const std::string&)> message_handler_;
    beast::flat_buffer buffer_;
    std::function<void(boost::system::error_code)> connect_callback_;
};

#endif // WEBSOCKET_HANDLER_H
