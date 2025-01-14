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

void executeTrades() {
    try {
        // Initialize WebSocket connection
        WebSocketHandler websocket("test.deribit.com", "443", "/ws/api/v2");
        websocket.connect();

        // Initialize trading operations
        // std::unique_ptr has minimal overhead and is generally faster than (optimization)
        // manual memory management with new and delete.
        auto trade = std::make_unique<TradeExecution>(websocket);

        // Authenticate
        json auth_response = trade->authenticate(CLIENT_ID, CLIENT_SECRET);
        std::cout << "Auth Response: " << auth_response.dump(4) << std::endl;

        // std::unordered_map<std::string, json> order_cache;

        while (true) {
            std::string instrument_name, order_id;
            double amount, price;

            // Display menu to the user
            std::cout << "\n--- Trading Menu ---\n";
            std::cout << "1. Place Order\n";
            std::cout << "2. Cancel Order\n";
            std::cout << "3. Modify Order\n";
            std::cout << "4. Get Order Book\n";
            std::cout << "5. View Current Positions\n";
            std::cout << "6. Subscribe to Order Book Updates\n";
            // std::cout << "7. View Active Orders\n";
            std::cout << "7. Exit\n";
            std::cout << "Enter your choice: ";
            int choice;
            std::cin >> choice;
            
            auto loop_start = LatencyModule::start();  // Start the timer
            
            if (choice == 7) {
                std::cout << "Exiting trading application.\n";
                break;
            }

            switch (choice) {
            case 1: {  // Place Order
                std::cout << "Enter instrument name (e.g., BTC-PERPETUAL): ";
                std::cin >> instrument_name;
                std::cout << "Enter amount: ";
                std::cin >> amount;
                std::cout << "Enter price: ";
                std::cin >> price;

                try {
                    auto order_future = std::async(std::launch::async, [&]() {
                        auto order_start = LatencyModule::start();
                        json buy_response = trade->placeBuyOrder(instrument_name, amount, price);
                        LatencyModule::end(order_start, "Order Placement");
                        return buy_response;
                    });
                    
                    json buy_response = order_future.get();
                    std::cout << "Full response received: " << buy_response.dump(2) << std::endl;
                    
                    if (buy_response.contains("result")) {
                        const auto& result = buy_response["result"];
                        std::cout << "Result section: " << result.dump(2) << std::endl;
                        
                    //     if (result.contains("order_id")) {
                    //         std::string order_id = result["order_id"].get<std::string>();
                    //         order_cache[order_id] = result;
                    //         std::cout << "Successfully cached order with ID: " << order_id << std::endl;
                    //         std::cout << "Cache size is now: " << order_cache.size() << std::endl;
                    //     } else {
                    //         std::cout << "No order_id in result" << std::endl;
                    //     }
                    // } else {
                    //     std::cout << "No result section in response" << std::endl;
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "Error placing order: " << e.what() << std::endl;
                }
                break;
            }

            case 2: {  // Cancel Order
                std::cout << "Enter order ID to cancel: ";
                std::cin >> order_id;

                try {
                        // if (order_cache.count(order_id) > 0) {
                        //     std::cout << "Cached Order Details: " << order_cache[order_id].dump(4) << std::endl;
                        // }
                    
                        auto cancel_future = std::async(std::launch::async, [&]() {
                            auto cancel_start = LatencyModule::start();
                            json cancel_response = trade->cancelOrder(order_id);
                            LatencyModule::end(cancel_start, "Cancel Order");
                        
                            // // Remove from cache after successful cancellation
                            // if (cancel_response.contains("result")) {
                            //     order_cache.erase(order_id);
                            // }
                        
                            return cancel_response;
                        });
                }
                catch (const std::exception& e) {
                    std::cerr << "Error cancelling order: " << e.what() << std::endl;
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

                try {
                        auto modify_future = std::async(std::launch::async, [&]() {
                        auto modify_start = LatencyModule::start();
                        json modify_response = trade->modifyOrder(order_id, price, amount);
                        LatencyModule::end(modify_start, "Modify Order");
                        return modify_response;
                        });

                    json modify_response = modify_future.get();
                    std::cout << "Modify Response: " << modify_response.dump(4) << std::endl;
                }
                catch (const std::exception& e) {
                    std::cerr << "Error modifying order: " << e.what() << std::endl;
                }
                break;
            }

            case 4: {  // Get Order Book
                std::cout << "Enter instrument name to view order book (e.g., BTC-PERPETUAL): ";
                std::cin >> instrument_name;

                try {
                    auto order_book_start = LatencyModule::start();
                    json order_book = trade->getOrderBook(instrument_name);
                    LatencyModule::end(order_book_start, "Order Book Fetch");
                    std::cout << "Order Book: " << order_book.dump(4) << std::endl;
                }
                catch (const std::exception& e) {
                    std::cerr << "Error fetching order book: " << e.what() << std::endl;
                }
                break;
            }

            case 5: {  // View Position for Specific Instrument
                std::string instrument_name;
                std::cout << "Enter instrument name (e.g., BTC-PERPETUAL): ";
                std::cin >> instrument_name;
                
                try {
                    auto position_start = LatencyModule::start();
                    json position = trade->getPosition(instrument_name);
                    LatencyModule::end(position_start, "Position Fetch");
                    std::cout << "Current Position: " << position.dump(4) << std::endl;
                }
                catch (const std::exception& e) {
                    std::cerr << "Error fetching position: " << e.what() << std::endl;
                }
                break;
            }

            case 6: {  // Subscribe to Order Book
                std::cout << "Enter instrument name to subscribe (e.g., BTC-PERPETUAL): ";
                std::cin >> instrument_name;
                
                try {
                    // Start a flag for controlling the subscription
                    std::atomic<bool> running{true};
                    
                    // Subscribe to the order book
                    trade->subscribeToOrderBook(instrument_name);
                    std::cout << "Subscribed to order book updates. Press 'q' to unsubscribe." << std::endl;
                    
                    // Create two threads - one for listening to updates and one for handling user input
                    std::thread listen_thread([&]() {
                        while(running) {
                            // Continuously read messages
                            json message = websocket.readMessage();
                            if(!message.empty()) {
                                websocket.onMessage(message.dump());
                            }
                            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Small delay to prevent CPU overuse
                        }
                    });

                    std::thread input_thread([&]() {
                        char input;
                        while ((input = std::cin.get()) != 'q') {
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        }
                        running = false;
                        trade->unsubscribeFromOrderBook(instrument_name);
                        std::cout << "Unsubscribed from order book updates." << std::endl;
                    });

                    // Wait for threads to complete
                    if(listen_thread.joinable()) listen_thread.join();
                    if(input_thread.joinable()) input_thread.join();
                }
                catch (const std::exception& e) {
                    std::cerr << "Error in order book subscription: " << e.what() << std::endl;
                }
                break;
            }

            // case 7: {  // Display Cached Orders
            //     if (order_cache.empty()) {
            //         std::cout << "\nNo active orders in cache.\n";
            //     } else {
            //         std::cout << "\n=== Active Orders ===\n";
            //         for (const auto& [order_id, order_info] : order_cache) {
            //             std::cout << "\nOrder ID: " << order_id;
            //             if (order_info.contains("instrument_name")) {
            //                 std::cout << "\nInstrument: " << order_info["instrument_name"];
            //             }
            //             if (order_info.contains("price")) {
            //                 std::cout << "\nPrice: " << order_info["price"];
            //             }
            //             if (order_info.contains("amount")) {
            //                 std::cout << "\nAmount: " << order_info["amount"];
            //             }
            //             if (order_info.contains("order_state")) {
            //                 std::cout << "\nState: " << order_info["order_state"];
            //             }
            //             std::cout << "\n-------------------\n";
            //         }
            //     }
            //     break;
            // }

            default:
                std::cout << "Invalid choice. Please try again.\n";
                break;
            }
            LatencyModule::end(loop_start, "End-to-End Trading Loop Latency");  // Measure latency for the trading loop
        }

        // Close connection
        websocket.close();

    }
    catch (const std::exception& e) {
        std::cerr << "Error in executeTrades: " << e.what() << std::endl;
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
