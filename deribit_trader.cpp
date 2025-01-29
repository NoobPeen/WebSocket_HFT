#include "api_credentials.h"
#include "websocket_handler.h"
#include "trade_execution.h"
#include "latency_module.h"
#include <iostream>
#include <string>
#include <exception>
#include <memory>
#include <unordered_map>
#include <future>
#include <vector>
#include <thread>
#include <immintrin.h>

void handleMenuChoice(int choice, TradeExecution* trade, std::shared_ptr<WebSocketHandler> websocket) {
    std::string instrument_name, order_id;
    double amount, price;

    try {
        switch (choice) {
            case 1: {  // Place Order
                std::cout << "Enter instrument name (e.g., BTC-PERPETUAL): ";
                std::cin >> instrument_name;
                std::cout << "Enter amount: ";
                std::cin >> amount;
                std::cout << "Enter price: ";
                std::cin >> price;

                auto order_future = std::async(std::launch::async, 
                    [trade, instrument_name, amount, price]() {
                        std::cout << "Order thread ID: " << std::this_thread::get_id() << std::endl;
                        auto order_start = LatencyModule::start();
                        auto result = trade->placeBuyOrder(instrument_name, amount, price);
                        LatencyModule::end(order_start, "Order Placement");
                        return result;
                    });
                std::cout << "Main thread continues immediately..." << std::endl;
                auto status = order_future.wait_for(std::chrono::seconds(5));
                if (status == std::future_status::ready) {
                    auto response = order_future.get();
                    std::cout << "Order Response: " << response.dump(2) << std::endl;
                } else {
                    throw std::runtime_error("Order placement timed out");
                }
                break;
            }

            case 2: {  // Cancel Order
                std::cout << "Enter order ID to cancel: ";
                std::cin >> order_id;

                auto cancel_future = std::async(std::launch::async, 
                    [trade, order_id]() {
                        std::cout << "Cancel Order thread ID: " << std::this_thread::get_id() << std::endl;
                        auto cancel_start = LatencyModule::start();
                        auto result = trade->cancelOrder(order_id);
                        LatencyModule::end(cancel_start, "Cancel Order");
                        return result;
                    });

                auto status = cancel_future.wait_for(std::chrono::seconds(5));
                if (status == std::future_status::ready) {
                    auto response = cancel_future.get();
                    std::cout << "Cancel Response: " << response.dump(2) << std::endl;
                } else {
                    throw std::runtime_error("Order cancellation timed out");
                }
                break;
            }

            case 3: {  // Modify Order
                std::cout << "Enter order ID to modify: ";
                std::cin >> order_id;
                std::cout << "Enter new price: ";
                std::cin >> price;
                std::cout << "Enter new amount: ";
                std::cin >> amount;

                auto modify_future = std::async(std::launch::async, 
                    [trade, order_id, price, amount]() {
                        std::cout << "Modify Order thread ID: " << std::this_thread::get_id() << std::endl;
                        auto modify_start = LatencyModule::start();
                        auto result = trade->modifyOrder(order_id, price, amount);
                        LatencyModule::end(modify_start, "Modify Order");
                        return result;
                    });

                auto status = modify_future.wait_for(std::chrono::seconds(5));
                if (status == std::future_status::ready) {
                    auto response = modify_future.get();
                    std::cout << "Modify Response: " << response.dump(2) << std::endl;
                } else {
                    throw std::runtime_error("Order modification timed out");
                }
                break;
            }

            case 4: {  // Get Order Book
                std::cout << "Enter instrument name to view order book (e.g., BTC-PERPETUAL): ";
                std::cin >> instrument_name;

                auto orderbook_future = std::async(std::launch::async, 
                    [trade, instrument_name]() {
                        std::cout << "order book thread ID: " << std::this_thread::get_id() << std::endl;
                        auto orderbook_start = LatencyModule::start();
                        std::cout << "Order book thread ID: " << std::this_thread::get_id() << std::endl;
                        auto result = trade->getOrderBook(instrument_name);
                        LatencyModule::end(orderbook_start, "Order Book Fetch");
                        return result;
                    });

                auto status = orderbook_future.wait_for(std::chrono::seconds(5));
                if (status == std::future_status::ready) {
                    auto order_book = orderbook_future.get();
                    if (order_book.contains("result")) {
                        const auto& result = order_book["result"];
                        std::cout << "\nOrder Book for " << instrument_name << ":\n";
                        
                        if (result.contains("bids")) {
                            std::cout << "\nTop 5 Bids:\n";
                            int count = 0;
                            for (const auto& bid : result["bids"]) {
                                if (count++ >= 5) break;
                                std::cout << "Price: " << bid[0] << ", Size: " << bid[1] << "\n";
                            }
                        }
                        
                        if (result.contains("asks")) {
                            std::cout << "\nTop 5 Asks:\n";
                            int count = 0;
                            for (const auto& ask : result["asks"]) {
                                if (count++ >= 5) break;
                                std::cout << "Price: " << ask[0] << ", Size: " << ask[1] << "\n";
                            }
                        }
                    }
                } else {
                    throw std::runtime_error("Order book fetch timed out");
                }
                break;
            }

            case 5: {  // View Position
                std::cout << "Enter instrument name (e.g., BTC-PERPETUAL): ";
                std::cin >> instrument_name;

                auto position_future = std::async(std::launch::async, 
                    [trade, instrument_name]() {
                        std::cout << "position thread ID: " << std::this_thread::get_id() << std::endl;
                        auto position_start = LatencyModule::start();
                        std::cout << "Position thread ID: " << std::this_thread::get_id() << std::endl;
                        auto result = trade->getPosition(instrument_name);
                        LatencyModule::end(position_start, "Position Fetch");
                        return result;
                    });

                auto status = position_future.wait_for(std::chrono::seconds(5));
                if (status == std::future_status::ready) {
                    auto position = position_future.get();
                    if (position.contains("result")) {
                        const auto& result = position["result"];
                        std::cout << "\nPosition Details:\n";
                        std::cout << "Size: " << result.value("size", 0.0) << "\n";
                        std::cout << "Entry Price: " << result.value("average_price", 0.0) << "\n";
                        std::cout << "Liquidation Price: " << result.value("liquidation_price", 0.0) << "\n";
                        std::cout << "Unrealized PNL: " << result.value("total_profit_loss", 0.0) << "\n";
                    }
                } else {
                    throw std::runtime_error("Position fetch timed out");
                }
                break;
            }

            case 6: {  // Subscribe to Order Book
                std::cout << "Enter instrument name to subscribe (e.g., BTC-PERPETUAL): ";
                std::cin >> instrument_name;

                std::atomic<bool> running{true};
                trade->subscribeToOrderBook(instrument_name);
                std::cout << "Subscribed to order book updates. Press 'q' to unsubscribe.\n";

                std::thread listen_thread([&running, websocket]() {
                    while(running) {
                        try {
                            json message = websocket->readMessage();
                            if(!message.empty()) {
                                websocket->handleOrderBookUpdate(message);
                            }
                        } catch (const std::exception& e) {
                            if (running) {
                                std::cerr << "Error reading message: " << e.what() << std::endl;
                            }
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                });

                std::thread input_thread([&running, trade, instrument_name]() {
                    char input;
                    while (running && (input = std::cin.get()) != 'q') {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                    running = false;
                    trade->unsubscribeFromOrderBook(instrument_name);
                });

                if(listen_thread.joinable()) listen_thread.join();
                if(input_thread.joinable()) input_thread.join();
                break;
            }

            default:
                std::cout << "Invalid choice. Please try again.\n";
                break;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error processing command: " << e.what() << std::endl;
    }
}


void executeTrades() {
    try {
        // Create io_context with work guard
        asio::io_context ioc;
        auto work = std::make_unique<asio::io_context::work>(ioc);
        
        // Flags for connection and authentication status
        std::atomic<bool> is_connected{false};
        std::atomic<bool> is_authenticated{false};
        std::atomic<bool> should_exit{false};
        
        // Create the WebSocket handler and trade execution objects
        auto websocket = std::make_shared<WebSocketHandler>(ioc, "test.deribit.com", "443", "/ws/api/v2");
        auto trade = std::make_unique<TradeExecution>(*websocket);

        // Start the IO context in a separate thread
        std::thread ioc_thread([&ioc]() {
            try {
                std::cout << "Starting IO context thread...\n";
                ioc.run();
                std::cout << "IO context thread stopped.\n";
            } catch (const std::exception& e) {
                std::cerr << "IO Context error: " << e.what() << std::endl;
            }
        });

        // Connect asynchronously with timeout
        auto connection_start = std::chrono::steady_clock::now();
        websocket->async_connect([&trade, &is_connected, &is_authenticated]
            (boost::system::error_code ec) {
            if (!ec) {
                is_connected = true;
                std::cout << "Connected successfully, attempting authentication...\n";
                
                try {
                    json auth_response = trade->authenticate(CLIENT_ID, CLIENT_SECRET);
                    if (auth_response.contains("result")) {
                        is_authenticated = true;
                        std::cout << "Authentication successful!\n";
                    } else {
                        std::cerr << "Authentication failed!\n";
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Authentication error: " << e.what() << std::endl;
                }
            } else {
                std::cerr << "Connection failed: " << ec.message() << std::endl;
            }
        });

        // Wait for connection and authentication with timeout
        const int timeout_seconds = 10;
        while (!is_authenticated) {
            auto current_time = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(
                current_time - connection_start).count() > timeout_seconds) {
                std::cerr << "Connection/Authentication timeout after " << timeout_seconds << " seconds\n";
                should_exit = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        if (!should_exit) {
            std::cout << "\nConnected and authenticated successfully!\n";
            
            while (!should_exit) {
                std::cout << "\n--- Trading Menu ---\n";
                std::cout << "Main thread ID: " << std::this_thread::get_id() << std::endl;
                std::cout << "1. Place Order\n";
                std::cout << "2. Cancel Order\n";
                std::cout << "3. Modify Order\n";
                std::cout << "4. Get Order Book\n";
                std::cout << "5. View Current Positions\n";
                std::cout << "6. Subscribe to Order Book Updates\n";
                std::cout << "7. Exit\n";
                std::cout << "Enter your choice: ";
                
                int choice;
                if (!(std::cin >> choice)) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "Invalid input. Please enter a number.\n";
                    continue;
                }

                if (choice == 7) {
                    should_exit = true;
                    break;
                }

                // Handle menu choices with proper error handling
                try {
                    handleMenuChoice(choice, trade.get(), websocket);
                } catch (const std::exception& e) {
                    std::cerr << "Error processing menu choice: " << e.what() << std::endl;
                }
            }
        }

        // Cleanup
        std::cout << "Cleaning up...\n";
        work.reset(); // Allow io_context to stop
        websocket->close();
        ioc.stop();
        
        if (ioc_thread.joinable()) {
            ioc_thread.join();
        }
        
        std::cout << "Cleanup complete. Exiting...\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error in executeTrades: " << e.what() << std::endl;
    }
}



int main() {
    try {
        executeTrades();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
