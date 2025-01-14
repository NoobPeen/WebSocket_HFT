# Trading Application (Deribit_Trading)

This project is a trading application that connects to a cryptocurrency exchange via WebSockets and allows the user to place, 
cancel, modify orders, fetch order books, and view current positions. The application has been optimized for performance, including 
asynchronous network communication, memory management improvements, and threading optimizations.

## Features

- **WebSocket-based Trading Interface**: Establishes a WebSocket connection with the exchange for real-time data and trade execution.
- **Command-line Trading Operations**: Provides a user-friendly menu-driven command-line interface for trading tasks.
- **Support for Various Order Types**: Place market, limit, or other types of orders as supported by the exchange.
- **API Authentication**: Secure authentication using client credentials for accessing trading features.
- **Open Orders Management**: View and manage all active orders.
- **Order Modification and Cancellation**: Modify or cancel existing orders seamlessly.
- **Optimized Performance**: Asynchronous operations, memory management enhancements, and threading optimizations for better responsiveness.

## Project Structure

The project follows a modular structure to ensure readability, maintainability, and scalability. Here's a breakdown of the main files and their purposes:

### Modular Breakdown

1. **Main Entry Point**:
   - **`deribit_trader.cpp`**: 
     - Contains the main trading loop.
     - Implements menu-driven command-line operations.
     - Integrates all modules for a seamless trading experience.

2. **WebSocket Handling**:
   - **`websocket_handler.h`** / **`websocket_handler.cpp`**:
     - Manages WebSocket connections.
     - Sends and receives messages from the exchange in real time.
     - Handles connection errors and reconnections.

3. **Trade Execution**:
   - **`trade_execution.h`** / **`trade_execution.cpp`**:
     - Authenticates with the exchange using API credentials.
     - Places, modifies, and cancels orders.
     - Fetches current positions and order book data.

4. **Latency Measurement**:
   - **`latency_module.h`** / **`latency_module.cpp`**:
     - Provides utilities for measuring latency of critical operations.
     - Logs performance data to help optimize execution times.

5. **API Credentials**:
   - **`api_credentials.h`**:
     - Stores API client ID and secret securely.
     - Ensures credentials are modularized for easy updates without affecting other files.

### Example Workflow

- The **main program** in `deribit_trader.cpp`:
  - Initializes the WebSocket connection using `websocket_handler`.
  - Authenticates using `trade_execution`.
  - Enters a command loop to handle user inputs for placing, modifying, or canceling orders.

- The **WebSocket handler**:
  - Establishes a secure connection to the exchange.
  - Sends and receives JSON-formatted messages.

- The **trade execution module**:
  - Interacts with the exchange API for order-related operations.
  - Utilizes WebSocket messages to fetch data.

- The **latency module**:
  - Logs timestamps to measure the time taken by each operation.
  - Ensures the application meets performance requirements.

This structure ensures that individual modules can be independently developed and tested while maintaining a clear separation of concerns.

## Prerequisites

### System Requirements

- **C++ Compiler**: A compiler with C++17 support, such as:
  - GCC 7+ (Linux)
  - Clang 7+ (Linux/macOS)
  - MSVC 2017+ (Windows)
- **CMake**: Version 3.x or higher.
- **Boost Libraries**: Required for utilities like `boost::asio` for network communication.
- **OpenSSL**: For secure WebSocket and API communication.
- **Git**: For version control and cloning the repository.

### Dependencies

- `nlohmann/json` (for JSON parsing and handling)
- WebSocket library
- Boost Libraries (including `boost::asio`)

## Installation

### 1. Clone the repository:

```bash
git clone https://github.com/automatesolutions/WebSocket_HFT.git
cd Websocket_HFT
```
### 2. Install dependencies:
-Install Boost Library
 Boost is required for this project. If Boost is not already installed, follow these steps:

1. Install Boost using your package manager:

Ubuntu/Debian:
```bash
sudo apt-get install libboost-all-dev
```
macOS (with Homebrew):
```bash
brew install boost
```
Windows: Download and install Boost from the Boost website.
[Link here](https://www.boost.org/)

2. Install OpenSSL
Ubuntu/Debian:
```bash
sudo apt-get install libssl-dev
```
macOS (with Homebrew):
```bash
brew install openssl
```
Windows: Download and install OpenSSL from the OpenSSL website.
[Link here](https://www.openssl.org/)

3. Install Other Dependencies
You may use a package manager like vcpkg to install libraries:
```bash
./vcpkg install nlohmann-json websocketp
```

4. Build the project with CMake:
a) Create a build directory:
```bash
mkdir build
cd build
```
b) Run CMake to configure the project:
```bash
cmake ..
```

c) Compile the project:
```bash
cmake --build . --config debug
```
This will generate the executable file deribit_trader.exe

4. Run the application:
```bash
./deribit_trader.exe
```

## Usage
Once the application is running, you will be prompted with a menu for selecting trading operations:

mathematica
```bash
--- Trading Menu ---
1. Place Order
2. Cancel Order
3. Modify Order
4. Get Order Book
5. View Current Positions
6. Exit
```
Example Workflow
1. Place Order:

Select option 1 and input the instrument name (e.g., BTC-PERPETUAL), amount, and price.
The application will asynchronously place a buy order and show the response.

2. Cancel Order:

Select option 2 and input the order ID to cancel the order.
The application will asynchronously cancel the order and show the response.

3. Modify Order:

Select option 3 and input the order ID, new amount, and new price to modify the existing order.

4. Get Order Book:

Select option 4 and input the instrument name to view the order book.

5. View Current Positions:

Select option 5 to view the current positions in your account.

6. Exit:

Select option 6 to exit the application.

## Optimizations
- Memory Management:
Uses std::make_unique for efficient memory management, preventing memory leaks and reducing overhead.

- Asynchronous Network Communication:
Network requests (placing, canceling, and modifying orders) are handled asynchronously to avoid blocking the main thread, improving responsiveness.

-Thread Management:
The use of std::async allows for concurrent execution of I/O operations, enhancing user experience by not blocking the main application thread.

-SIMD/Parallelization:
Future improvements can include SIMD (Single Instruction, Multiple Data) optimizations for parallelizing large-scale data processing tasks, such as order book analysis.

## Demo
Watch this demo video to see the application in action:
[Crypto Trading System using C++](https://vimeo.com/1041769096)




## Future Improvements
SIMD optimization for large-scale data processing and order book analysis.
Integration with additional exchanges and features.
Advanced error handling and retries for network communication.


## Contributing

Contributions are welcome! If you'd like to improve this project, fix bugs, or add new features, feel free to fork the repository, make your changes, and submit a pull request. Your efforts will help make this trading application even better!

If you found this project helpful or learned something new from it, you can support the development with just a cup of coffee ☕. It's always appreciated and keeps the ideas flowing!

[![Buy Me a Coffee](https://img.shields.io/badge/Buy%20Me%20a%20Coffee-Support-blue?style=for-the-badge&logo=coffee&logoColor=white)](https://buymeacoffee.com/jonelpericon)










