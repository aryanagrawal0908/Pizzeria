#include <bits/stdc++.h>
#include "pizzeria.h"
using namespace std;

int main() {
    cout << "🍕 Concurrent Pizzeria Simulation 🍕" << endl;
    cout << "=====================================" << endl;
    cout << "This simulation demonstrates:" << endl;
    cout << "• Multi-threading with chefs, customers, and delivery service" << endl;
    cout << "• Mutex locks for thread-safe operations" << endl;
    cout << "• Semaphores for resource management" << endl;
    cout << "• Condition variables for thread synchronization" << endl;
    cout << "• Atomic operations for statistics" << endl;
    cout << "=====================================" << endl;
    cout << endl;
    
    try {
        // Get user input for simulation parameters
        int num_chefs = 3;
        int num_customers = 5;
        
        cout << "Enter number of chefs (1-6, default 3): ";
        cin >> num_chefs;
        if (num_chefs < 1 || num_chefs > 6) {
            num_chefs = 3;
            cout << "Using default: 3 chefs" << endl;
        }
        
        cout << "Enter number of customers (1-10, default 5): ";
        cin >> num_customers;
        if (num_customers < 1 || num_customers > 10) {
            num_customers = 5;
            cout << "Using default: 5 customers" << endl;
        }
        
        cout << endl;
        cout << "🚀 Starting pizzeria with " << num_chefs << " chefs and " 
                  << num_customers << " customers..." << endl;
        cout << "📝 The simulation will run for 30 seconds, then close to new orders." << endl;
        cout << "⏱️  Total simulation time: ~45 seconds" << endl;
        cout << endl;
        
        // Create and start pizzeria
        g_pizzeria = make_unique<Pizzeria>(num_chefs, num_customers);
        g_pizzeria->startOperations();
        
        cout << endl;
        cout << "✅ Simulation completed successfully!" << endl;
        cout << "Thank you for running the Concurrent Pizzeria Simulation!" << endl;
        
    } catch (const exception& e) {
        cerr << "❌ Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
