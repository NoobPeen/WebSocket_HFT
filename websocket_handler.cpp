#include "websocket_handler.h"
#include "latency_module.h"
#include <iostream>

WebSocketHandler::WebSocketHandler(asio::io_context& ioc, const std::string& host, 
                                 const std::string& port, const std::string& endpoint)
    : ioc_(ioc),
      ctx_(ssl::context::tlsv12_client),
      resolver_(ioc_),
      websocket_(ioc_, ctx_),
      host_(host),
      endpoint_(endpoint) {
    
    ctx_.set_verify_mode(ssl::verify_peer);
    ctx_.set_default_verify_paths();
}
void WebSocketHandler::onMessage(const std::string& message) {
    try {
        json data = json::parse(message);
        
        // Handle subscription messages
        if (data.contains("method") && data["method"] == "subscription") {
            if (data.contains("params") && data["params"].contains("channel") 
                && data["params"]["channel"].get<std::string>().substr(0, 4) == "book") {
                handleOrderBookUpdate(data);
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in onMessage: " << e.what() << std::endl;
    }
}

void WebSocketHandler::connect() {
    try {
        // Resolve the host and port
        auto const results = resolver_.resolve(host_, "443");

        // Connect to the server
        asio::connect(websocket_.next_layer().next_layer(), results.begin(), results.end());

        // Perform the SSL handshake
        websocket_.next_layer().handshake(ssl::stream_base::client);

        // Perform the WebSocket handshake
        websocket_.handshake(host_, endpoint_);

        std::cout << "WebSocket connected successfully!" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error during WebSocket connection: " << e.what() << std::endl;
    }
}

// void WebSocketHandler::onMessage(const std::string& message) {
//     // Parse the incoming message into JSON
//     json data = json::parse(message);


// }

void WebSocketHandler::sendMessage(const json& message) {
    try {
        // Serialize the JSON message and send it
        std::string message_str = message.dump();
        websocket_.write(asio::buffer(message_str));

        // std::cout << "Sent message: " << message_str << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error sending message: " << e.what() << std::endl;
    }
}

json WebSocketHandler::readMessage() {
    try {
        auto read_start = LatencyModule::start();  // Start timer for WebSocket message read

        beast::flat_buffer buffer;
        websocket_.read(buffer);

        // Parse the received message as JSON
        std::string message_str = beast::buffers_to_string(buffer.data());
        // std::cout << "Received message: " << message_str << std::endl;

        // End the timer and log the latency
        LatencyModule::end(read_start, "WebSocket Read Latency");

        return json::parse(message_str);
    }
    catch (const std::exception& e) {
        std::cerr << "Error reading message: " << e.what() << std::endl;
        return json();  // Return an empty JSON object in case of error
    }
}

void WebSocketHandler::close() {
    try {
        websocket_.close(beast::websocket::close_code::normal);
        std::cout << "WebSocket connection closed." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error closing WebSocket: " << e.what() << std::endl;
    }
}

void WebSocketHandler::handleOrderBookUpdate(const json& data) {
    try {
        if (data.contains("params") && data["params"].contains("data")) {
            const auto& orderBook = data["params"]["data"];
            
            // Print timestamp and instrument name if available
            if (orderBook.contains("timestamp")) {
                std::cout << "\nTimestamp: " << orderBook["timestamp"] << std::endl;
            }
            if (orderBook.contains("instrument_name")) {
                std::cout << "Instrument: " << orderBook["instrument_name"] << std::endl;
            }

            // Print bids
            if (orderBook.contains("bids") && !orderBook["bids"].empty()) {
                std::cout << "Bids Updates:" << std::endl;
                for (const auto& bid : orderBook["bids"]) {
                    if (bid.size() >= 2) {
                        std::cout << "Price: " << bid[0] << ", Size: " << bid[1];
                        if (bid[0] == "delete") {
                            std::cout << " (Deleted)";
                        }
                        std::cout << std::endl;
                    }
                }
            }

            // Print asks
            if (orderBook.contains("asks") && !orderBook["asks"].empty()) {
                std::cout << "Asks Updates:" << std::endl;
                for (const auto& ask : orderBook["asks"]) {
                    if (ask.size() >= 2) {
                        std::cout << "Price: " << ask[0] << ", Size: " << ask[1];
                        if (ask[0] == "delete") {
                            std::cout << " (Deleted)";
                        }
                        std::cout << std::endl;
                    }
                }
            }
            std::cout << "----------------------------------------" << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error handling order book update: " << e.what() << std::endl;
    }
}

void WebSocketHandler::subscribe(const std::string& channel) {
    json sub_message = {
        {"jsonrpc", "2.0"},
        {"method", "private/subscribe"},
        {"params", {
            {"channels", {channel}}
        }},
        {"id", 42}
    };
    sendMessage(sub_message);
}

void WebSocketHandler::unsubscribe(const std::string& channel) {
    json unsub_message = {
        {"jsonrpc", "2.0"},
        {"method", "private/unsubscribe"},
        {"params", {
            {"channels", {channel}}
        }},
        {"id", 42}
    };
    sendMessage(unsub_message);
}


void WebSocketHandler::async_connect(std::function<void(boost::system::error_code)> callback) {
    try {
        std::cout << "Starting async connection to: " << host_ << std::endl;
        
        // Start the resolver
        resolver_.async_resolve(
            host_,
            "443",
            [this, callback](boost::system::error_code ec, tcp::resolver::results_type results) {
                if(ec) {
                    std::cerr << "Resolution failed: " << ec.message() << std::endl;
                    if(callback) callback(ec);
                    return;
                }

                // Once resolved, initiate async connect
                asio::async_connect(
                    websocket_.next_layer().next_layer(),
                    results,
                    [this, callback](boost::system::error_code ec, const tcp::endpoint&) {
                        if(ec) {
                            std::cerr << "Connect failed: " << ec.message() << std::endl;
                            if(callback) callback(ec);
                            return;
                        }

                        // After connection, perform SSL handshake
                        websocket_.next_layer().async_handshake(
                            ssl::stream_base::client,
                            [this, callback](boost::system::error_code ec) {
                                if(ec) {
                                    std::cerr << "SSL handshake failed: " << ec.message() << std::endl;
                                    if(callback) callback(ec);
                                    return;
                                }

                                // Finally, perform WebSocket handshake
                                websocket_.async_handshake(
                                    host_,
                                    endpoint_,
                                    [this, callback](boost::system::error_code ec) {
                                        if(ec) {
                                            std::cerr << "WebSocket handshake failed: " << ec.message() << std::endl;
                                        } else {
                                            std::cout << "WebSocket connected successfully!" << std::endl;
                                        }
                                        if(callback) callback(ec);
                                    });
                            });
                    });
            });
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in async_connect: " << e.what() << std::endl;
        if(callback) {
            callback(boost::system::errc::make_error_code(
                boost::system::errc::operation_canceled));
        }
    }
}

void WebSocketHandler::start_read() {
    auto self = shared_from_this();
    websocket_.async_read(
        buffer_,
        [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
            if (!ec) {
                std::string msg = beast::buffers_to_string(buffer_.data());
                buffer_.consume(buffer_.size());
                onMessage(msg);
                start_read();  // Continue reading
            }
        });
}

void WebSocketHandler::set_message_handler(std::function<void(const std::string&)> handler) {
    message_handler_ = handler;
}

void WebSocketHandler::close_connection() {
    try {
        websocket_.close(beast::websocket::close_code::normal);
    } catch (const std::exception& e) {
        std::cerr << "Error closing WebSocket: " << e.what() << std::endl;
    }
}

// void WebSocketHandler::start_ping_timer() {
//     auto timer = std::make_shared<boost::asio::steady_timer>(ioc_);
//     timer->expires_after(std::chrono::seconds(30));
//     timer->async_wait([this, timer](boost::system::error_code ec) {
//         if (!ec) {
//             try {
//                 websocket_.ping();
//                 start_ping_timer();  // Schedule next ping
//             } catch (const std::exception& e) {
//                 std::cerr << "Ping error: " << e.what() << std::endl;
//             }
//         }
//     });
// }
