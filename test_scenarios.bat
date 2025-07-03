@echo off
echo ========================================
echo 🧪 PIZZERIA SIMULATION TEST SCENARIOS 🧪
echo ========================================
echo.

echo 📋 Test Case 1: Minimal Load (1 chef, 1 customer)
echo 1 | echo 1 | .\pizzeria.exe
echo.
echo Press any key to continue to next test...
pause >nul

echo 📋 Test Case 2: Balanced Load (3 chefs, 5 customers)
echo 3 | echo 5 | .\pizzeria.exe
echo.
echo Press any key to continue to next test...
pause >nul

echo 📋 Test Case 3: High Load (6 chefs, 10 customers)
echo 6 | echo 10 | .\pizzeria.exe
echo.
echo Press any key to continue to next test...
pause >nul

echo 📋 Test Case 4: Bottleneck Test (1 chef, 8 customers)
echo 1 | echo 8 | .\pizzeria.exe
echo.

echo ✅ All test scenarios completed!
echo.
echo 📊 Key things to observe:
echo • Thread synchronization under different loads
echo • Resource contention with varying chef/customer ratios
echo • Order processing times and queue management
echo • Ingredient consumption and restocking
echo • Clean shutdown with accurate statistics
echo.
pause
