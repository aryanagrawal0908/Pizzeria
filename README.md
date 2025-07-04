
## 🎯 Project Overview

This project simulates a busy pizzeria with multiple chefs, customers, orders, and ingredients. It showcases various concurrency mechanisms working together to create a realistic multi-threaded application.
# 🍕 Concurrent Pizzeria Simulation

A multi-threaded C++ simulation of a pizzeria with real-time order processing, ingredient management, and financial tracking.


### Pizza Menu & Pricing
```cpp
PizzaType::MARGHERITA  - $12.99
PizzaType::PEPPERONI   - $15.99
PizzaType::MUSHROOM    - $14.99
PizzaType::VEGGIE      - $16.99
PizzaType::SUPREME     - $19.99
```

### Timing Configuration
```cpp
Active Operations:     25 seconds   // New orders accepted
Shutdown Processing:   50 seconds   // Complete existing orders
Total Runtime:         75 seconds   // Maximum total time
```

### Default Setup
- **Chefs**: 3 (Mario, Luigi, Giuseppe)
- **Customers**: 5 (Alice, Bob, Charlie, Diana, Eve)
- **Initial Ingredients**: Limited quantities to simulate real-world constraints

## 💳 Financial System

### Payment Flow
1. **Order Placement**: Customer pays immediately upon ordering
2. **Order Processing**: Chefs prepare and cook pizzas
3. **Delivery**: Payment confirmed when order is delivered
4. **Refunds**: Undelivered orders receive 110% refund

### Earnings Calculation
```cpp
Gross Earnings = Sum of all delivered orders
Total Refunds = Sum of (undelivered orders × 1.10)
Net Earnings = Gross Earnings - Total Refunds
```

## 🧵 Threading Model

### Thread Types
1. **Customer Threads** (5-default): Place orders independently
2. **Chef Threads** (3-default): Process orders from queue
3. **Delivery Thread** (1): Delivers completed orders
4. **Ingredient Manager** (1): Monitors and restocks ingredients
5. **Statistics Reporter** (1): Periodic status updates

### Synchronization Mechanisms
- **Mutexes**: Protect shared data structures
- **Condition Variables**: Thread communication for order availability
- **Atomic Variables**: Thread-safe counters for statistics
- **Semaphores**: Resource management for chefs and ingredients

### Deadlock Prevention
- **Consistent Lock Ordering**: Prevents circular dependencies
- **Timeout Mechanisms**: Prevents infinite waiting
- **Periodic Notifications**: Ensures thread responsiveness

## 📈 Performance Metrics

### Key Metrics Tracked
- **Order Statistics**: Placed, completed, delivered counts
- **Completion Rate**: Percentage of successfully delivered orders
- **Processing Time**: Average time from order to delivery
- **Financial Performance**: Revenue, refunds, net profit
- **Resource Utilization**: Ingredient consumption and restocking
 

### Manual Compilation
```bash
# GCC/Clang
g++ -std=c++20 -pthread -Wall -Wextra -O2 main.cpp pizzeria.cpp -o pizzeria

# Run
./pizzeria
```

### Windows (MinGW)
```bash
g++ -std=c++20 -pthread main.cpp pizzeria.cpp -o pizzeria.exe
pizzeria.exe
```

## 📊 Sample Output

```
🍕 Welcome to Concurrent Pizzeria! 🍕
💰 PIZZA PRICES: Margherita $12.99 | Pepperoni $15.99 | Mushroom $14.99 | Veggie $16.99 | Supreme $19.99

[14:30:15.123] 💰 Customer 1 (Alice) placed Order #1 for Margherita ($12.99) - PAID
[14:30:16.456] Chef 1 (Mario) started preparing Order #1 (Margherita)
[14:30:22.789] 🚚 Order #1 delivered to Customer 1 ($12.99) - Processing time: 7.6s

==================================================
📋 COMPLETION ANALYSIS
==================================================
Orders Placed: 15
Orders Delivered: 14
Unprocessed Orders: 1
Completion Rate: 93.3%
👍 Good job! Most orders were completed.

💰 FINAL EARNINGS REPORT 💰
============================================================
Gross Earnings (Delivered Orders): $198.86
Total Refunds Paid (with 10% apology): $17.59
Net Earnings: $181.27

🎉 Profitable day! Net profit: $181.27
============================================================
```

