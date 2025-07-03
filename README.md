# Concurrent Pizzeria Simulation

A comprehensive C++ simulation that demonstrates advanced concurrency concepts including multi-threading, semaphores, mutex locks, and condition variables in a real-world pizzeria scenario.

## 🎯 Project Overview

This project simulates a busy pizzeria with multiple chefs, customers, orders, and ingredients. It showcases various concurrency mechanisms working together to create a realistic multi-threaded application.

## 🔧 Concurrency Concepts Demonstrated

### 1. **Multi-threading**
- **Chef threads**: Each chef works independently, taking orders from a shared queue
- **Customer threads**: Customers place orders concurrently 
- **Delivery service thread**: Handles order delivery in the background
- **Ingredient manager thread**: Monitors and restocks ingredients
- **Statistics reporter thread**: Periodically reports system status

### 2. **Mutex Locks**
- **Order queue mutex**: Protects the shared order queue from race conditions
- **Ready orders mutex**: Ensures thread-safe access to completed orders
- **Console output mutex**: Prevents interleaved console output
- **Order status mutex**: Protects individual order status updates
- **Ingredient mutex**: Ensures thread-safe ingredient consumption and restocking

### 3. **Semaphores**
- **Chef semaphore**: Limits the number of concurrent chefs (resource management)
- **Ingredient semaphore**: Controls access to ingredient inventory

### 4. **Condition Variables**
- **Order available**: Signals chefs when new orders arrive
- **Ready order available**: Notifies delivery service when orders are ready

### 5. **Atomic Operations**
- **Order counters**: Thread-safe counting of placed, completed, and delivered orders
- **Control flags**: Safe management of pizzeria open/closed state

## 🏗️ Architecture

### Core Classes

1. **Pizzeria** - Main orchestrator class that manages all operations
2. **Chef** - Represents individual chefs who prepare orders
3. **Customer** - Represents customers who place orders
4. **Order** - Represents individual pizza orders with status tracking
5. **Ingredient** - Manages ingredient inventory with thread-safe operations

### Key Features

- **Dynamic ingredient management**: Ingredients are consumed during cooking and restocked automatically
- **Realistic timing**: Random delays simulate real-world preparation and cooking times
- **Order tracking**: Complete order lifecycle from placement to delivery
- **Statistics reporting**: Real-time monitoring of pizzeria performance
- **Graceful shutdown**: Proper cleanup when pizzeria closes

## 🔄 Process Flow

1. **Initialization**: Pizzeria starts with specified number of chefs and customers
2. **Order placement**: Customers place orders concurrently at random intervals
3. **Order processing**: Chefs take orders from queue and check ingredient availability
4. **Preparation**: Chefs prepare orders (ingredient consumption, cooking simulation)
5. **Completion**: Finished orders are moved to ready queue
6. **Delivery**: Delivery service picks up ready orders and delivers them
7. **Monitoring**: Background threads monitor ingredients and report statistics

## 🚀 Compilation and Execution

### Requirements
- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- Standard library with semaphore support

### Windows (MSVC)
```bash
cl /EHsc /std:c++20 main.cpp pizzeria.cpp -o pizzeria.exe
```

### Linux/macOS (GCC)
```bash
g++ -std=c++20 -pthread main.cpp pizzeria.cpp -o pizzeria
```

### Run the simulation
```bash
./pizzeria
```

## 📊 Sample Output

```
🍕 Concurrent Pizzeria Simulation 🍕
=====================================
Enter number of chefs (1-6, default 3): 3
Enter number of customers (1-10, default 5): 5

🚀 Starting pizzeria with 3 chefs and 5 customers...

[10:30:15.123] 🍕 Welcome to Concurrent Pizzeria! 🍕
[10:30:15.124] Opening for business...
[10:30:16.234] Customer 1 (Alice) placed Order #1 for Margherita
[10:30:16.445] Chef 1 (Mario) started preparing Order #1 (Margherita)
[10:30:17.123] Customer 2 (Bob) placed Order #2 for Pepperoni
[10:30:18.567] Chef 1 (Mario) is cooking Order #1 (Margherita)
[10:30:19.234] Chef 2 (Luigi) started preparing Order #2 (Pepperoni)
[10:30:22.789] Chef 1 (Mario) completed Order #1 (Margherita)
[10:30:23.456] 🚚 Order #1 delivered to Customer 1 (Processing time: 7.3s)
...
```

## 🎓 Learning Outcomes

This simulation demonstrates:

1. **Thread synchronization**: How multiple threads can work together safely
2. **Resource management**: Using semaphores to limit concurrent access
3. **Producer-consumer pattern**: Order queue management between customers and chefs
4. **Deadlock prevention**: Proper lock ordering and timeout mechanisms
5. **Performance monitoring**: Real-time statistics in multi-threaded environment

## 🔧 Customization

You can modify the simulation by:

- Adjusting timing parameters (cooking time, order intervals)
- Adding new pizza types and ingredients
- Implementing priority ordering systems
- Adding more complex chef specializations
- Extending statistics and reporting features

## 📝 Technical Notes

- Uses C++20 features including `std::counting_semaphore`
- Implements RAII for automatic resource management
- Thread-safe singleton pattern for global pizzeria access
- Proper exception handling for robustness
- Clean shutdown procedures for all threads
