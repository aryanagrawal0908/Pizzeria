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
      status(OrderStatus::PENDING), order_time(chrono::steady_clock::now()),
      price(0.0), is_paid(false), is_refunded(false) {
    
    // Set price based on pizza type
    switch (type) {
        case PizzaType::MARGHERITA:
            price = 12.99;
            break;
        case PizzaType::PEPPERONI:
            price = 15.99;
            break;
        case PizzaType::MUSHROOM:
            price = 14.99;
            break;
        case PizzaType::VEGGIE:
            price = 16.99;
            break;
        case PizzaType::SUPREME:
            price = 19.99;
            break;
        default:
            price = 12.99;
            break;
    }
}

double Order::getPrice() const {
    return price;
}

void Order::setPaid(bool paid) {
    lock_guard<mutex> lock(order_mutex);
    is_paid = paid;
}

bool Order::isPaid() const {
    lock_guard<mutex> lock(order_mutex);
    return is_paid;
}

void Order::setRefunded(bool refunded) {
    lock_guard<mutex> lock(order_mutex);
    is_refunded = refunded;
}

bool Order::isRefunded() const {
    lock_guard<mutex> lock(order_mutex);
    return is_refunded;
}

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

// Replace the Customer::placeOrders method:

void Customer::placeOrders() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> pizza_choice(0, 4);
    uniform_int_distribution<> order_interval(2000, 8000);
    uniform_int_distribution<> orders_per_customer(1, 3);
    
    int num_orders = orders_per_customer(gen);
    
    for (int i = 0; i < num_orders && g_pizzeria->isAcceptingOrders(); ++i) {
        PizzaType pizza_type = static_cast<PizzaType>(pizza_choice(gen));
        auto order = make_shared<Order>(customer_id, pizza_type);
        
        // Customer pays for the order
        double price = order->getPrice();
        order->setPaid(true);
        
        g_pizzeria->addOrder(order);
        g_pizzeria->printOrderStatus("PAYMENT: Customer " + to_string(customer_id) + 
            " (" + name + ") placed Order #" + to_string(order->getOrderId()) + 
            " for " + order->getPizzaName() + " ($" + 
            to_string(price).substr(0, to_string(price).find('.') + 3) + ") - PAID");
        
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
    ingredients.push_back(make_unique<Ingredient>(IngredientType::DOUGH, "Dough", 5));
    ingredients.push_back(make_unique<Ingredient>(IngredientType::CHEESE, "Cheese", 10));
    ingredients.push_back(make_unique<Ingredient>(IngredientType::TOMATO_SAUCE, "Tomato Sauce", 8));
    ingredients.push_back(make_unique<Ingredient>(IngredientType::PEPPERONI, "Pepperoni", 6));
    ingredients.push_back(make_unique<Ingredient>(IngredientType::MUSHROOMS, "Mushrooms", 4));
    ingredients.push_back(make_unique<Ingredient>(IngredientType::OLIVES, "Olives", 3));
    ingredients.push_back(make_unique<Ingredient>(IngredientType::BELL_PEPPERS, "Bell Peppers", 3));
    

// Create chefs
vector<string> chef_names = {"Mario", "Luigi", "Giuseppe", "Antonio", "Francesco", "Giovanni"};
for (int i = 0; i < num_chefs; ++i) {
    string chef_name = (static_cast<size_t>(i) < chef_names.size()) ? chef_names[i] : "Chef" + to_string(i + 1);
    chefs.push_back(make_unique<Chef>(i + 1, chef_name));
}

// Create customers
vector<string> customer_names = {"Alice", "Bob", "Charlie", "Diana", "Eve", "Frank", "Grace", "Henry", "Ivy", "Jack"};
for (int i = 0; i < num_customers; ++i) {
    string customer_name = (static_cast<size_t>(i) < customer_names.size()) ? customer_names[i] : "Customer" + to_string(i + 1);
    customers.push_back(make_unique<Customer>(i + 1, customer_name));
}
}

Pizzeria::~Pizzeria() {
    stopOperations();
}

void Pizzeria::addOrder(shared_ptr<Order> order) {
    // order_queue is buffer queue b/w customer placing order and chef processing it
    lock_guard<mutex> lock(order_queue_mutex);
    order_queue.push(order);
    total_orders_placed++;
    order_available.notify_one();
}

shared_ptr<Order> Pizzeria::getNextOrder() {
    unique_lock<mutex> lock(order_queue_mutex);// this ensures only one thread accesses the queue at a time
    // Wait for an order to be available or pizzeria to close
    order_available.wait(lock, [this] { return !order_queue.empty() || !is_open; });
    
    if (!order_queue.empty()) {
        auto order = order_queue.front();
        order_queue.pop();
        return order;
    }
    return nullptr;
}

void Pizzeria::addReadyOrder(shared_ptr<Order> order) {
    lock_guard<mutex> lock(ready_orders_mutex);// releases the mutex when it goes out of scope
    ready_orders.push(order);
    total_orders_completed++;
    ready_order_available.notify_one();// notify waiting threads that a new order is ready
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
            (*it)->consume(1);// decrease by 1 unit
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
    printOrderStatus("*** Welcome to Concurrent Pizzeria! ***");
    printOrderStatus("PIZZA PRICES: Margherita $12.99 | Pepperoni $15.99 | Mushroom $14.99 | Veggie $16.99 | Supreme $19.99");
    printOrderStatus("Opening for business...");
    
    // Start chefs
    for (auto& chef : chefs) {
        chef->startWorking();
    }
    
    // Start customers
    for (auto& customer : customers) {
        customer->startOrdering();
    }
    
    // Start service threads
    thread delivery_thread(&Pizzeria::deliveryService, this);
    thread ingredient_thread(&Pizzeria::ingredientManager, this);
    thread stats_thread(&Pizzeria::statisticsReporter, this);
    
    // Run for reduced time: 25 seconds instead of 30
    this_thread::sleep_for(chrono::seconds(25));
    
    // Stop accepting new orders
    accepting_orders = false;
    printOrderStatus("WARNING: Pizzeria closed for new orders. Processing remaining orders...");
    
    // Reduced waiting time: 35 seconds instead of 65 (total: 60s instead of 90s)
    int wait_cycles = 0;
    const int max_wait_cycles = 50; // Changed from 65 to 35 seconds
    
    while (wait_cycles < max_wait_cycles) {
        this_thread::sleep_for(chrono::seconds(1));
        wait_cycles++;
        
        // Check if all orders are processed
        bool orders_remaining = false;
        {
            lock_guard<mutex> order_lock(order_queue_mutex);
            lock_guard<mutex> ready_lock(ready_orders_mutex);
            orders_remaining = !order_queue.empty() || !ready_orders.empty();
        }
        
        if (!orders_remaining && total_orders_placed == total_orders_delivered) {
            printOrderStatus("SUCCESS: All orders completed and delivered!");
            break;
        }
        
        // Progress updates
        if (wait_cycles % 5 == 0) {
            printOrderStatus("PROCESSING: " + to_string(total_orders_delivered) + "/" + 
                           to_string(total_orders_placed) + " delivered");
        }

        if (wait_cycles % 10 == 0) {
            order_available.notify_all();
            ready_order_available.notify_all();
        }
    }
    
    if (wait_cycles >= max_wait_cycles) {
        printOrderStatus("TIMEOUT: 75-second time limit reached!");
    }
    
    // Process refunds for undelivered orders
    processRefunds();
    
    // Close pizzeria
    is_open = false;
    order_available.notify_all();
    ready_order_available.notify_all();
    
    // Wait for threads
    if (delivery_thread.joinable()) delivery_thread.join();
    if (ingredient_thread.joinable()) ingredient_thread.join();
    if (stats_thread.joinable()) stats_thread.join();
    
    printOrderStatus("FINAL: Pizzeria closed. Final reports:");
    printStatistics();
    printCompletionAnalysis();
    printEarningsReport();
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

// Replace the printStatistics method:

void Pizzeria::printStatistics() {
    lock_guard<mutex> lock(cout_mutex);
    cout << "\n" << string(50, '=') << endl;
    cout << "PIZZERIA STATISTICS" << endl;
    cout << string(50, '=') << endl;
    cout << "Total Orders Placed: " << total_orders_placed << endl;
    cout << "Total Orders Completed: " << total_orders_completed << endl;
    cout << "Total Orders Delivered: " << total_orders_delivered << endl;
    cout << "Orders in Queue: " << order_queue.size() << endl;
    cout << "Ready Orders: " << ready_orders.size() << endl;
    cout << "\nINGREDIENT LEVELS:" << endl;
    for (const auto& ingredient : ingredients) {
        cout << "  " << ingredient->getName() << ": " << ingredient->getQuantity() << endl;
    }
    cout << string(50, '=') << endl;
}

// Replace the printCompletionAnalysis method:

void Pizzeria::printCompletionAnalysis() {
    lock_guard<mutex> lock(cout_mutex);
    cout << "\n" << string(50, '=') << endl;
    cout << "COMPLETION ANALYSIS" << endl;
    cout << string(50, '=') << endl;
    
    int unprocessed_orders = total_orders_placed - total_orders_delivered;
    double completion_rate = (total_orders_delivered * 100.0) / total_orders_placed;
    
    cout << "Orders Placed: " << total_orders_placed << endl;
    cout << "Orders Delivered: " << total_orders_delivered << endl;
    cout << "Unprocessed Orders: " << unprocessed_orders << endl;
    cout << "Completion Rate: " << fixed << setprecision(1) << completion_rate << "%" << endl;
    
    {
        lock_guard<mutex> order_lock(order_queue_mutex);
        lock_guard<mutex> ready_lock(ready_orders_mutex);
        
        if (!order_queue.empty()) {
            cout << "WARNING: Orders still in queue: " << order_queue.size() << endl;
        }
        
        if (!ready_orders.empty()) {
            cout << "WARNING: Orders ready but not delivered: " << ready_orders.size() << endl;
        }
    }
    
    if (completion_rate == 100.0) {
        cout << "PERFECT! All orders were successfully completed!" << endl;
    } else if (completion_rate >= 90.0) {
        cout << "GOOD: Most orders were completed." << endl;  // Fixed: Removed Unicode thumbs up
    } else {
        cout << "SUGGESTION: Consider increasing processing time or chef count for better completion rates." << endl;  // Fixed: Removed Unicode notebook
    }
    
    cout << string(50, '=') << endl;
}

double Pizzeria::calculateRefund(double original_price) {
    return original_price * 1.10; // 110% refund (original + 10% apology)
}

// Replace the processRefunds method:

void Pizzeria::processRefunds() {
    vector<shared_ptr<Order>> undelivered_orders;
    
    // Collect undelivered orders from queue
    {
        lock_guard<mutex> order_lock(order_queue_mutex);
        while (!order_queue.empty()) {
            undelivered_orders.push_back(order_queue.front());
            order_queue.pop();
        }
    }
    
    // Collect undelivered ready orders
    {
        lock_guard<mutex> ready_lock(ready_orders_mutex);
        while (!ready_orders.empty()) {
            undelivered_orders.push_back(ready_orders.front());
            ready_orders.pop();
        }
    }
    
    // Process refunds
    if (!undelivered_orders.empty()) {
        printOrderStatus("REFUNDS: Processing refunds with 10% apology bonus...");
        
        for (auto& order : undelivered_orders) {
            if (order->isPaid() && !order->isRefunded()) {
                double refund_amount = calculateRefund(order->getPrice());
                total_refunds.store(total_refunds.load() + refund_amount);
                order->setRefunded(true);
                
                printOrderStatus("REFUND: Issued to Customer " + 
                    to_string(order->getCustomerId()) + " for Order #" + 
                    to_string(order->getOrderId()) + ": $" + 
                    to_string(refund_amount).substr(0, to_string(refund_amount).find('.') + 3) + 
                    " (Original: $" + to_string(order->getPrice()).substr(0, to_string(order->getPrice()).find('.') + 3) + 
                    " + 10% apology)");
            }
        }
    }
}

void Pizzeria::printEarningsReport() {
    lock_guard<mutex> lock(cout_mutex);
    cout << "\n" << string(60, '=') << endl;
    cout << "FINAL EARNINGS REPORT" << endl;  // Fixed: Removed Unicode money symbol
    cout << string(60, '=') << endl;
    
    double gross_earnings = total_earnings.load();
    double total_refunds_paid = total_refunds.load();
    double net_earnings = gross_earnings - total_refunds_paid;
    
    cout << "Gross Earnings (Delivered Orders): $" << fixed << setprecision(2) << gross_earnings << endl;
    cout << "Total Refunds Paid (with 10% apology): $" << fixed << setprecision(2) << total_refunds_paid << endl;
    cout << "Net Earnings: $" << fixed << setprecision(2) << net_earnings << endl;
    
    cout << "\nBREAKDOWN:" << endl;  // Fixed: Removed Unicode chart symbol
    cout << "  Orders Delivered: " << total_orders_delivered << endl;
    cout << "  Orders Refunded: " << (total_orders_placed - total_orders_delivered) << endl;
    cout << "  Average Order Value: $" << fixed << setprecision(2) 
         << (total_orders_delivered > 0 ? gross_earnings / total_orders_delivered : 0.0) << endl;
    
    if (net_earnings > 0) {
        cout << "\nSUCCESS: Profitable day! Net profit: $" << fixed << setprecision(2) << net_earnings << endl;  // Fixed: Removed Unicode party symbol
    } else if (net_earnings < 0) {
        cout << "\nLOSS: Loss incurred due to refunds: $" << fixed << setprecision(2) << abs(net_earnings) << endl;  // Fixed: Removed Unicode sad face
    } else {
        cout << "\nBREAK-EVEN: Break-even day!" << endl;  // Fixed: Removed Unicode neutral face
    }
    
    cout << string(60, '=') << endl;
}

// Replace the deliveryService method:

void Pizzeria::deliveryService() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> delivery_time(800, 2500);
    
    while (is_open || !ready_orders.empty()) {
        auto order = getReadyOrder();
        if (!order) {
            if (!is_open) {
                lock_guard<mutex> lock(ready_orders_mutex);
                if (ready_orders.empty()) break;
            }
            this_thread::sleep_for(chrono::milliseconds(50));
            continue;
        }
        
        // Simulate delivery time
        this_thread::sleep_for(chrono::milliseconds(delivery_time(gen)));
        
        order->setStatus(OrderStatus::DELIVERED);
        order->markCompleted();
        total_orders_delivered++;
        
        // Add to earnings when delivered
        if (order->isPaid()) {
            total_earnings.store(total_earnings.load() + order->getPrice());
        }
        
        printOrderStatus("DELIVERY: Order #" + to_string(order->getOrderId()) + 
            " delivered to Customer " + to_string(order->getCustomerId()) + 
            " ($" + to_string(order->getPrice()).substr(0, to_string(order->getPrice()).find('.') + 3) + 
            ") - Processing time: " + to_string(order->getProcessingTime()) + "s");
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

bool Pizzeria::isOpen() const {
    return is_open.load();
}

bool Pizzeria::isAcceptingOrders() const {
    return accepting_orders.load();
}

void Pizzeria::ingredientManager() {
    uniform_int_distribution<> restock_amount(5, 20);
    
    while (is_open) {
        this_thread::sleep_for(chrono::seconds(8)); // Check every 8 seconds
        
        // Check if any ingredient is running low
        bool need_restock = false;
        for (const auto& ingredient : ingredients) {
            if (ingredient->getQuantity() < 10) { // Restock when below 10 units
                need_restock = true;
                break;
            }
        }
        
        if (need_restock) {
            for (auto& ingredient : ingredients) {
                ingredient->restock(restock_amount(gen));
            }
            printOrderStatus("RESTOCK: Ingredients restocked!"); // Fixed: Removed Unicode box symbol
        }
    }
}

void Pizzeria::statisticsReporter() {
    while (is_open) {
        this_thread::sleep_for(chrono::seconds(15)); // Report every 15 seconds
        if (is_open) {
            printStatistics();
        }
    }
}
