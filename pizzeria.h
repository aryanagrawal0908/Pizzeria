#pragma once
#include <bits/stdc++.h>
#include <semaphore>

using namespace std;

// Forward declarations
class Order;
class Chef;
class Customer;
class Ingredient;

// Ingredient types
enum class IngredientType {
    DOUGH,
    CHEESE,
    TOMATO_SAUCE,
    PEPPERONI,
    MUSHROOMS,
    OLIVES,
    BELL_PEPPERS
};

// Pizza types
enum class PizzaType {
    MARGHERITA,
    PEPPERONI,
    MUSHROOM,
    VEGGIE,
    SUPREME
};

// Order status
enum class OrderStatus {
    PENDING,
    PREPARING,
    COOKING,
    READY,
    DELIVERED
};

// Ingredient class
class Ingredient {
private:
    IngredientType type;
    string name;
    int quantity;
    mutable mutex ingredient_mutex;

public:
    Ingredient(IngredientType t, const string& n, int q);
    bool consume(int amount);
    void restock(int amount);
    int getQuantity() const;
    string getName() const;
    IngredientType getType() const;
};

// Order class
class Order {
private:
    static atomic<int> order_counter;
    int order_id;
    int customer_id;
    PizzaType pizza_type;
    OrderStatus status;
    chrono::steady_clock::time_point order_time;
    chrono::steady_clock::time_point completion_time;
    mutable mutex order_mutex;

public:
    Order(int cust_id, PizzaType type);
    int getOrderId() const;
    int getCustomerId() const;
    PizzaType getPizzaType() const;
    OrderStatus getStatus() const;
    void setStatus(OrderStatus new_status);
    string getPizzaName() const;
    double getProcessingTime() const;
    void markCompleted();
};

// Chef class
class Chef {
private:
    int chef_id;
    string name;
    bool is_working;
    thread chef_thread;
    
public:
    Chef(int id, const string& chef_name);
    ~Chef();
    void startWorking();
    void stopWorking();
    void work();
    int getChefId() const;
    string getName() const;
};

// Customer class
class Customer {
private:
    int customer_id;
    string name;
    thread customer_thread;
    
public:
    Customer(int id, const string& customer_name);
    ~Customer();
    void startOrdering();
    void placeOrders();
    int getCustomerId() const;
    string getName() const;
};

// Pizzeria class - main orchestrator
class Pizzeria {
private:
    // Concurrency controls
    mutex order_queue_mutex;
    mutex ready_orders_mutex;
    mutex cout_mutex;
    condition_variable order_available;
    condition_variable ready_order_available;
    
    // Semaphores for resource management
    counting_semaphore<> chef_semaphore;
    counting_semaphore<> ingredient_semaphore;
    
    // Collections
    queue<shared_ptr<Order>> order_queue;
    queue<shared_ptr<Order>> ready_orders;
    vector<unique_ptr<Chef>> chefs;
    vector<unique_ptr<Customer>> customers;
    vector<unique_ptr<Ingredient>> ingredients;
    
    // Statistics
    atomic<int> total_orders_placed{0};
    atomic<int> total_orders_completed{0};
    atomic<int> total_orders_delivered{0};
    
    // Control flags
    atomic<bool> is_open{true};
    atomic<bool> accepting_orders{true};
    
    // Random number generation
    random_device rd;
    mt19937 gen;
    uniform_int_distribution<> pizza_dist;
    uniform_int_distribution<> timing_dist;
    
public:
    Pizzeria(int num_chefs, int num_customers);
    ~Pizzeria();
    
    // Order management
    void addOrder(shared_ptr<Order> order);
    shared_ptr<Order> getNextOrder();
    void addReadyOrder(shared_ptr<Order> order);
    shared_ptr<Order> getReadyOrder();
    
    // Ingredient management
    bool checkAndConsumeIngredients(PizzaType pizza_type);
    void restockIngredients();
    
    // Threading methods
    void startOperations();
    void stopOperations();
    
    // Utility methods
    void printOrderStatus(const string& message);
    void printStatistics();
    bool isOpen() const;
    bool isAcceptingOrders() const;
    
    // Delivery thread
    void deliveryService();
    
    // Ingredient restocking thread
    void ingredientManager();
    
    // Statistics thread
    void statisticsReporter();
};

// Global pizzeria instance
extern unique_ptr<Pizzeria> g_pizzeria;

// Utility functions
string pizzaTypeToString(PizzaType type);
string orderStatusToString(OrderStatus status);
string ingredientTypeToString(IngredientType type);
vector<IngredientType> getRequiredIngredients(PizzaType pizza_type);
