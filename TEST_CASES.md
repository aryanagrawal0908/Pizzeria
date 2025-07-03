# Test Cases for Concurrent Pizzeria Simulation

## Test Case 1: Basic Functionality
**Objective**: Verify basic pizzeria operations
**Steps**:
1. Run with 1 chef and 1 customer
2. Observe order placement, preparation, and delivery
3. Check that all steps complete successfully

**Expected Result**: Single order should be processed from start to finish

## Test Case 2: Concurrency Stress Test
**Objective**: Test system under high load
**Steps**:
1. Run with 6 chefs and 10 customers
2. Monitor for race conditions or deadlocks
3. Verify all orders are processed correctly

**Expected Result**: No crashes, all orders processed, statistics accurate

## Test Case 3: Resource Contention
**Objective**: Test ingredient management under pressure
**Steps**:
1. Run with 3 chefs and 8 customers
2. Monitor ingredient levels during operation
3. Verify ingredients are restocked appropriately

**Expected Result**: Ingredients should be consumed and restocked without issues

## Test Case 4: Thread Safety
**Objective**: Verify thread-safe operations
**Steps**:
1. Run multiple instances simultaneously
2. Monitor console output for interleaved messages
3. Check for consistent statistics

**Expected Result**: Clean console output, no data corruption

## Test Case 5: Graceful Shutdown
**Objective**: Test proper cleanup
**Steps**:
1. Run simulation to completion
2. Verify all threads terminate properly
3. Check final statistics accuracy

**Expected Result**: Clean shutdown with accurate final report

## Performance Benchmarks

### Typical Performance Metrics:
- **Order Processing Time**: 4-12 seconds per order
- **Ingredient Consumption**: 3-7 ingredients per pizza
- **Throughput**: 5-15 orders per minute (depends on chef count)
- **Memory Usage**: < 50MB for normal operation

### Scalability Tests:
- **1 Chef, 1 Customer**: ~2-3 orders/minute
- **3 Chefs, 5 Customers**: ~8-12 orders/minute  
- **6 Chefs, 10 Customers**: ~15-20 orders/minute

## Memory and Thread Analysis

### Thread Count:
- Main thread: 1
- Chef threads: N (user specified)
- Customer threads: N (user specified)
- Delivery thread: 1
- Ingredient manager: 1
- Statistics reporter: 1
- **Total**: 3 + 2N threads

### Memory Usage:
- Base memory: ~10MB
- Per chef: ~1MB
- Per customer: ~0.5MB
- Per order: ~1KB

## Debugging Commands

### GCC/Linux:
```bash
# Debug build
make debug

# Memory leak detection
make valgrind

# Static analysis
make analyze
```

### MSVC/Windows:
```batch
# Debug build
cl /EHsc /std:c++20 /DEBUG main.cpp pizzeria.cpp /Fe:pizzeria_debug.exe

# Run with debug output
pizzeria_debug.exe
```

## Common Issues and Solutions

### Issue 1: Compiler Errors
**Problem**: "semaphore not found" or C++20 features not supported
**Solution**: 
- Use GCC 10+ or Clang 12+ or MSVC 2019+
- Ensure `-std=c++20` flag is used
- Link with `-pthread` on Linux/macOS

### Issue 2: Deadlocks
**Problem**: Simulation hangs or stops responding
**Solution**:
- Check for circular lock dependencies
- Verify timeout values in condition variables
- Ensure proper lock ordering

### Issue 3: Race Conditions
**Problem**: Inconsistent output or statistics
**Solution**:
- Verify all shared data is protected by mutexes
- Check atomic operations for counters
- Ensure proper synchronization in critical sections

### Issue 4: Resource Leaks
**Problem**: High memory usage or resource exhaustion
**Solution**:
- Use RAII for automatic resource management
- Ensure all threads are properly joined
- Check for memory leaks with valgrind

## Expected Output Analysis

### Normal Operation Indicators:
- ✅ Orders are placed and processed in reasonable time
- ✅ Multiple chefs work simultaneously
- ✅ Ingredients are consumed and restocked
- ✅ Statistics are updated consistently
- ✅ Clean shutdown with final report

### Warning Signs:
- ⚠️ Orders stuck in queue for long periods
- ⚠️ Ingredients never restocked
- ⚠️ Inconsistent statistics
- ⚠️ Threads not terminating on shutdown
- ⚠️ Garbled console output
