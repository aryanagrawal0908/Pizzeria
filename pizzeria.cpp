#include <bits/stdc++.h>
#include "pizzeria.h"
using namespace std;

// Global pizzeria instance
unique_ptr<Pizzeria> g_pizzeria;

// Static member initialization
atomic<int> Order::order_counter{1};

// Ingredient implementation
Ingredient::Ingredient(IngredientType t, const string& n, int q) 
    : type(t), name(n), quantity(q) {}

bool Ingredient::consume(int amount) {
    lock_guard<mutex> lock(ingredient_mutex);
    if (quantity >= amount) {
        quantity -= amount;
        return true;
    }
    return false;
}

void Ingredient::restock(int amount) {
    lock_guard<mutex> lock(ingredient_mutex);
    quantity += amount;
}

int Ingredient::getQuantity() const {
    lock_guard<mutex> lock(ingredient_mutex);
    return quantity;
}

string Ingredient::getName() const {
    return name;
}

IngredientType Ingredient::getType() const {
    return type;
}

// Order implementation
Order::Order(int cust_id, PizzaType type) 
    : order_id(order_counter++), customer_id(cust_id), pizza_type(type), 
      status(OrderStatus::PENDING), order_time(chrono::steady_clock::now()) {}

int Order::getOrderId() const {
    return order_id;
}

int Order::getCustomerId() const {
    return customer_id;
}

PizzaType Order::getPizzaType() const {
    return pizza_type;
}

OrderStatus Order::getStatus() const {
    lock_guard<mutex> lock(order_mutex);
    return status;
}

void Order::setStatus(OrderStatus new_status) {
    lock_guard<mutex> lock(order_mutex);
    status = new_status;
}

string Order::getPizzaName() const {
    return pizzaTypeToString(pizza_type);
}

double Order::getProcessingTime() const {
    if (status == OrderStatus::DELIVERED) {
        auto duration = chrono::duration_cast<chrono::milliseconds>(
            completion_time - order_time);
        return duration.count() / 1000.0;
    }
    return 0.0;
}

void Order::markCompleted() {
    completion_time = chrono::steady_clock::now();
}

// Chef implementation
Chef::Chef(int id, const string& chef_name) 
    : chef_id(id), name(chef_name), is_working(false) {}

Chef::~Chef() {
    stopWorking();
}

void Chef::startWorking() {
    if (!is_working) {
        is_working = true;
        chef_thread = thread(&Chef::work, this);
    }
}

void Chef::stopWorking() {
    if (is_working) {
        is_working = false;
        if (chef_thread.joinable()) {
            chef_thread.join();
        }
    }
}

void Chef::work() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> cooking_time(3000, 8000); // 3-8 seconds
    
    while (is_working && g_pizzeria->isOpen()) {
        // Wait for an order
        auto order = g_pizzeria->getNextOrder();
        if (!order) {
            this_thread::sleep_for(chrono::milliseconds(100));
            continue;
        }
        
        // Check and consume ingredients
        if (!g_pizzeria->checkAndConsumeIngredients(order->getPizzaType())) {
            g_pizzeria->printOrderStatus("Chef " + to_string(chef_id) + 
                " (" + name + ") - Cannot prepare Order #" + 
                to_string(order->getOrderId()) + " - Insufficient ingredients!");
            
            // Put the order back in queue
            g_pizzeria->addOrder(order);
            this_thread::sleep_for(chrono::milliseconds(500));
            continue;
        }
        
        // Start preparing
        order->setStatus(OrderStatus::PREPARING);
        g_pizzeria->printOrderStatus("Chef " + to_string(chef_id) + 
            " (" + name + ") started preparing Order #" + 
            to_string(order->getOrderId()) + " (" + order->getPizzaName() + ")");
        
        // Simulate preparation time
        this_thread::sleep_for(chrono::milliseconds(1000 + cooking_time(gen) / 4));
        
        // Start cooking
        order->setStatus(OrderStatus::COOKING);
        g_pizzeria->printOrderStatus("Chef " + to_string(chef_id) + 
            " (" + name + ") is cooking Order #" + 
            to_string(order->getOrderId()) + " (" + order->getPizzaName() + ")");
        
        // Simulate cooking time
        this_thread::sleep_for(chrono::milliseconds(cooking_time(gen)));
        
        // Mark as ready
        order->setStatus(OrderStatus::READY);
        g_pizzeria->printOrderStatus("Chef " + to_string(chef_id) + 
            " (" + name + ") completed Order #" + 
            to_string(order->getOrderId()) + " (" + order->getPizzaName() + ")");
        
        // Add to ready orders
        g_pizzeria->addReadyOrder(order);
    }
}

int Chef::getChefId() const {
    return chef_id;
}

string Chef::getName() const {
    return name;
}

// Customer implementation
Customer::Customer(int id, const string& customer_name) 
    : customer_id(id), name(customer_name) {}

Customer::~Customer() {
    if (customer_thread.joinable()) {
        customer_thread.join();
    }
}

void Customer::startOrdering() {
    customer_thread = thread(&Customer::placeOrders, this);
}

void Customer::placeOrders() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> pizza_choice(0, 4); // 5 pizza types
    uniform_int_distribution<> order_interval(2000, 10000); // 2-10 seconds
    uniform_int_distribution<> orders_per_customer(1, 3); // 1-3 orders per customer
    
    int num_orders = orders_per_customer(gen);
    
    for (int i = 0; i < num_orders && g_pizzeria->isAcceptingOrders(); ++i) {
        PizzaType pizza_type = static_cast<PizzaType>(pizza_choice(gen));
        auto order = make_shared<Order>(customer_id, pizza_type);
        
        g_pizzeria->addOrder(order);
        g_pizzeria->printOrderStatus("Customer " + to_string(customer_id) + 
            " (" + name + ") placed Order #" + to_string(order->getOrderId()) + 
            " for " + order->getPizzaName());
        
        if (i < num_orders - 1) {
            this_thread::sleep_for(chrono::milliseconds(order_interval(gen)));
        }
    }
}

int Customer::getCustomerId() const {
    return customer_id;
}

string Customer::getName() const {
    return name;
}

// Pizzeria implementation
Pizzeria::Pizzeria(int num_chefs, int num_customers) 
    : chef_semaphore(num_chefs), ingredient_semaphore(1000), 
      gen(rd()), pizza_dist(0, 4), timing_dist(1000, 5000) {
    
    // Initialize ingredients
    ingredients.push_back(make_unique<Ingredient>(IngredientType::DOUGH, "Dough", 50));
    ingredients.push_back(make_unique<Ingredient>(IngredientType::CHEESE, "Cheese", 100));
    ingredients.push_back(make_unique<Ingredient>(IngredientType::TOMATO_SAUCE, "Tomato Sauce", 80));
    ingredients.push_back(make_unique<Ingredient>(IngredientType::PEPPERONI, "Pepperoni", 60));
    ingredients.push_back(make_unique<Ingredient>(IngredientType::MUSHROOMS, "Mushrooms", 40));
    ingredients.push_back(make_unique<Ingredient>(IngredientType::OLIVES, "Olives", 30));
    ingredients.push_back(make_unique<Ingredient>(IngredientType::BELL_PEPPERS, "Bell Peppers", 35));
    
    // Create chefs
    vector<string> chef_names = {"Mario", "Luigi", "Giuseppe", "Antonio", "Francesco", "Giovanni"};
    for (int i = 0; i < num_chefs; ++i) {
        string chef_name = (i < chef_names.size()) ? chef_names[i] : "Chef" + to_string(i + 1);
        chefs.push_back(make_unique<Chef>(i + 1, chef_name));
    }
    
    // Create customers
    vector<string> customer_names = {"Alice", "Bob", "Charlie", "Diana", "Eve", "Frank", "Grace", "Henry", "Ivy", "Jack"};
    for (int i = 0; i < num_customers; ++i) {
        string customer_name = (i < customer_names.size()) ? customer_names[i] : "Customer" + to_string(i + 1);
        customers.push_back(make_unique<Customer>(i + 1, customer_name));
    }
}

Pizzeria::~Pizzeria() {
    stopOperations();
}

void Pizzeria::addOrder(shared_ptr<Order> order) {
    lock_guard<mutex> lock(order_queue_mutex);
    order_queue.push(order);
    total_orders_placed++;
    order_available.notify_one();
}

shared_ptr<Order> Pizzeria::getNextOrder() {
    unique_lock<mutex> lock(order_queue_mutex);
    order_available.wait(lock, [this] { return !order_queue.empty() || !is_open; });
    
    if (!order_queue.empty()) {
        auto order = order_queue.front();
        order_queue.pop();
        return order;
    }
    return nullptr;
}

void Pizzeria::addReadyOrder(shared_ptr<Order> order) {
    lock_guard<mutex> lock(ready_orders_mutex);
    ready_orders.push(order);
    total_orders_completed++;
    ready_order_available.notify_one();
}

shared_ptr<Order> Pizzeria::getReadyOrder() {
    unique_lock<mutex> lock(ready_orders_mutex);
    ready_order_available.wait_for(lock, chrono::milliseconds(1000), 
        [this] { return !ready_orders.empty() || !is_open; });
    
    if (!ready_orders.empty()) {
        auto order = ready_orders.front();
        ready_orders.pop();
        return order;
    }
    return nullptr;
}

bool Pizzeria::checkAndConsumeIngredients(PizzaType pizza_type) {
    auto required_ingredients = getRequiredIngredients(pizza_type);
    
    // Check if all ingredients are available
    for (auto ingredient_type : required_ingredients) {
        auto it = find_if(ingredients.begin(), ingredients.end(),
            [ingredient_type](const unique_ptr<Ingredient>& ing) {
                return ing->getType() == ingredient_type;
            });
        
        if (it == ingredients.end() || (*it)->getQuantity() < 1) {
            return false;
        }
    }
    
    // Consume ingredients
    for (auto ingredient_type : required_ingredients) {
        auto it = find_if(ingredients.begin(), ingredients.end(),
            [ingredient_type](const unique_ptr<Ingredient>& ing) {
                return ing->getType() == ingredient_type;
            });
        
        if (it != ingredients.end()) {
            (*it)->consume(1);
        }
    }
    
    return true;
}

void Pizzeria::restockIngredients() {
    uniform_int_distribution<> restock_amount(5, 20);
    for (auto& ingredient : ingredients) {
        ingredient->restock(restock_amount(gen));
    }
}

void Pizzeria::startOperations() {
    printOrderStatus("🍕 Welcome to Concurrent Pizzeria! 🍕");
    printOrderStatus("Opening for business...");
    
    // Start chefs
    for (auto& chef : chefs) {
        chef->startWorking();
    }
    
    // Start customers
    for (auto& customer : customers) {
        customer->startOrdering();
    }
    
    // Start delivery service
    thread delivery_thread(&Pizzeria::deliveryService, this);
    
    // Start ingredient manager
    thread ingredient_thread(&Pizzeria::ingredientManager, this);
    
    // Start statistics reporter
    thread stats_thread(&Pizzeria::statisticsReporter, this);
    
    // Run for a specified time
    this_thread::sleep_for(chrono::seconds(30));
    
    // Stop accepting new orders
    accepting_orders = false;
    printOrderStatus("⚠️  Pizzeria is now closed for new orders. Finishing existing orders...");
    
    // Wait for all orders to be processed
    this_thread::sleep_for(chrono::seconds(15));
    
    // Close pizzeria
    is_open = false;
    order_available.notify_all();
    ready_order_available.notify_all();
    
    // Wait for threads to finish
    if (delivery_thread.joinable()) delivery_thread.join();
    if (ingredient_thread.joinable()) ingredient_thread.join();
    if (stats_thread.joinable()) stats_thread.join();
    
    printOrderStatus("🏁 Pizzeria has closed. Final statistics:");
    printStatistics();
}

void Pizzeria::stopOperations() {
    is_open = false;
    accepting_orders = false;
    order_available.notify_all();
    ready_order_available.notify_all();
}

void Pizzeria::printOrderStatus(const string& message) {
    lock_guard<mutex> lock(cout_mutex);
    auto now = chrono::system_clock::now();
    auto time_t = chrono::system_clock::to_time_t(now);
    auto ms = chrono::duration_cast<chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    cout << put_time(localtime(&time_t), "[%H:%M:%S");
    cout << "." << setfill('0') << setw(3) << ms.count() << "] ";
    cout << message << endl;
}

void Pizzeria::printStatistics() {
    lock_guard<mutex> lock(cout_mutex);
    cout << "\n" << string(50, '=') << endl;
    cout << "📊 PIZZERIA STATISTICS 📊" << endl;
    cout << string(50, '=') << endl;
    cout << "Total Orders Placed: " << total_orders_placed << endl;
    cout << "Total Orders Completed: " << total_orders_completed << endl;
    cout << "Total Orders Delivered: " << total_orders_delivered << endl;
    cout << "Orders in Queue: " << order_queue.size() << endl;
    cout << "Ready Orders: " << ready_orders.size() << endl;
    cout << "\n📦 INGREDIENT LEVELS:" << endl;
    for (const auto& ingredient : ingredients) {
        cout << "  " << ingredient->getName() << ": " << ingredient->getQuantity() << endl;
    }
    cout << string(50, '=') << endl;
}

bool Pizzeria::isOpen() const {
    return is_open;
}

bool Pizzeria::isAcceptingOrders() const {
    return accepting_orders;
}

void Pizzeria::deliveryService() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> delivery_time(1000, 3000);  
    
    while (is_open) {
        auto order = getReadyOrder();
        if (!order) {
            this_thread::sleep_for(chrono::milliseconds(100));
            continue;
        }
        
        // Simulate delivery time
        this_thread::sleep_for(chrono::milliseconds(delivery_time(gen)));
        
        order->setStatus(OrderStatus::DELIVERED);
        order->markCompleted();
        total_orders_delivered++;
        
        printOrderStatus("🚚 Order #" + to_string(order->getOrderId()) + 
            " delivered to Customer " + to_string(order->getCustomerId()) + 
            " (Processing time: " + to_string(order->getProcessingTime()) + "s)");
    }
}

void Pizzeria::ingredientManager() {
    while (is_open) {
        this_thread::sleep_for(chrono::seconds(8));
        
        // Check if any ingredient is running low
        bool need_restock = false;
        for (const auto& ingredient : ingredients) {
            if (ingredient->getQuantity() < 10) {
                need_restock = true;
                break;
            }
        }
        
        if (need_restock) {
            restockIngredients();
            printOrderStatus("📦 Ingredients restocked!");
        }
    }
}

void Pizzeria::statisticsReporter() {
    while (is_open) {
        this_thread::sleep_for(chrono::seconds(10));
        printStatistics();
    }
}

// Utility functions
string pizzaTypeToString(PizzaType type) {
    switch (type) {
        case PizzaType::MARGHERITA: return "Margherita";
        case PizzaType::PEPPERONI: return "Pepperoni";
        case PizzaType::MUSHROOM: return "Mushroom";
        case PizzaType::VEGGIE: return "Veggie";
        case PizzaType::SUPREME: return "Supreme";
        default: return "Unknown";
    }
}

string orderStatusToString(OrderStatus status) {
    switch (status) {
        case OrderStatus::PENDING: return "Pending";
        case OrderStatus::PREPARING: return "Preparing";
        case OrderStatus::COOKING: return "Cooking";
        case OrderStatus::READY: return "Ready";
        case OrderStatus::DELIVERED: return "Delivered";
        default: return "Unknown";
    }
}

string ingredientTypeToString(IngredientType type) {
    switch (type) {
        case IngredientType::DOUGH: return "Dough";
        case IngredientType::CHEESE: return "Cheese";
        case IngredientType::TOMATO_SAUCE: return "Tomato Sauce";
        case IngredientType::PEPPERONI: return "Pepperoni";
        case IngredientType::MUSHROOMS: return "Mushrooms";
        case IngredientType::OLIVES: return "Olives";
        case IngredientType::BELL_PEPPERS: return "Bell Peppers";
        default: return "Unknown";
    }
}

// ingredients each pizza needs:
vector<IngredientType> getRequiredIngredients(PizzaType pizza_type) {
    vector<IngredientType> ingredients;
    
    // All pizzas need dough, cheese, and tomato sauce
    ingredients.push_back(IngredientType::DOUGH);
    ingredients.push_back(IngredientType::CHEESE);
    ingredients.push_back(IngredientType::TOMATO_SAUCE);
    
    switch (pizza_type) {
        case PizzaType::MARGHERITA:
            // Only basic ingredients
            break;
        case PizzaType::PEPPERONI:
            ingredients.push_back(IngredientType::PEPPERONI);
            break;
        case PizzaType::MUSHROOM:
            ingredients.push_back(IngredientType::MUSHROOMS);
            break;
        case PizzaType::VEGGIE:
            ingredients.push_back(IngredientType::MUSHROOMS);
            ingredients.push_back(IngredientType::OLIVES);
            ingredients.push_back(IngredientType::BELL_PEPPERS);
            break;
        case PizzaType::SUPREME:
            ingredients.push_back(IngredientType::PEPPERONI);
            ingredients.push_back(IngredientType::MUSHROOMS);
            ingredients.push_back(IngredientType::OLIVES);
            ingredients.push_back(IngredientType::BELL_PEPPERS);
            break;
    }
    
    return ingredients;
}
