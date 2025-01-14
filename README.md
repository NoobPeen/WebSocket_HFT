# Deribit Trading CLI Application

A high-performance, low-latency command-line trading interface for Deribit cryptocurrency exchange using C++17 and Boost.

## Features

- Real-time WebSocket connection to Deribit API v2
- Low-latency order execution and market data streaming
- Comprehensive order management (place, cancel, modify)
- Real-time order book monitoring
- Position tracking
- Market data subscription system
- Built with modern C++17 features
- Optimized for performance with minimal latency

## Prerequisites

Install required dependencies on Ubuntu:

```bash
# Update package list
sudo apt update

# Install build essentials and cmake
sudo apt install -y build-essential cmake

# Install Boost libraries
sudo apt install -y libboost-all-dev

# Install OpenSSL
sudo apt install -y libssl-dev

# Install nlohmann-json
sudo apt install -y nlohmann-json3-dev
```

## Building the Project

1. Clone the repository:
```bash
git clone <repository-url>
cd deribit-trading-cli
```

2. Create build directory:
```bash
mkdir build
cd build
```

3. Build the project:
```bash
cmake ..
make
```

## Running the Application

Execute the built binary:
```bash
./deribit_trader
```

## Usage

The application provides a command-line interface with the following options:

1. Place Order - Create new market or limit orders
2. Cancel Order - Cancel existing orders by ID
3. Modify Order - Update price/quantity of existing orders
4. Get Order Book - View current market depth
5. View Current Positions - Check open positions
6. Subscribe to Order Book Updates - Real-time market data
7. Exit

## Performance Features

- Asynchronous WebSocket communication
- Memory-optimized data structures
- Low-latency market data processing
- Real-time latency monitoring

## Error Handling

The application includes comprehensive error handling for:
- Network connectivity issues
- API authentication failures
- Invalid order parameters
- Market data processing errors

## Configuration

API credentials are stored in `api_credentials.h`. Update with your own credentials:
```cpp
const std::string CLIENT_ID = "your_client_id";
const std::string CLIENT_SECRET = "your_client_secret";
```
