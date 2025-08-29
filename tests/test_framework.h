#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <chrono>

// Simple test framework for Browserwind validation
class TestFramework {
public:
    struct TestResult {
        std::string testName;
        bool passed;
        std::string message;
        double duration; // in milliseconds
    };

    static TestFramework& getInstance() {
        static TestFramework instance;
        return instance;
    }

    void addTest(const std::string& name, std::function<bool()> testFunc) {
        tests.push_back({name, testFunc});
    }

    bool runAllTests() {
        std::cout << "🧪 Starting Browserwind Validation Tests\n";
        std::cout << "=======================================\n\n";

        int passed = 0;
        int failed = 0;
        std::vector<TestResult> results;

        for (const auto& test : tests) {
            TestResult result;
            result.testName = test.name;

            auto start = std::chrono::high_resolution_clock::now();

            try {
                result.passed = test.func();
                auto end = std::chrono::high_resolution_clock::now();
                result.duration = std::chrono::duration<double, std::milli>(end - start).count();

                if (result.passed) {
                    result.message = "✅ PASSED";
                    passed++;
                } else {
                    result.message = "❌ FAILED";
                    failed++;
                }
            } catch (const std::exception& e) {
                result.passed = false;
                result.message = "💥 EXCEPTION: " + std::string(e.what());
                failed++;
            }

            results.push_back(result);
            printTestResult(result);
        }

        printSummary(passed, failed, results);
        return failed == 0;
    }

private:
    struct Test {
        std::string name;
        std::function<bool()> func;
    };

    std::vector<Test> tests;

    void printTestResult(const TestResult& result) {
        std::cout << "[" << result.message << "] " << result.testName;
        std::cout << " (" << result.duration << "ms)\n";
    }

    void printSummary(int passed, int failed, const std::vector<TestResult>& results) {
        std::cout << "\n=======================================\n";
        std::cout << "📊 Test Summary:\n";
        std::cout << "   Total Tests: " << (passed + failed) << "\n";
        std::cout << "   ✅ Passed: " << passed << "\n";
        std::cout << "   ❌ Failed: " << failed << "\n";

        if (failed > 0) {
            std::cout << "\n❌ Failed Tests:\n";
            for (const auto& result : results) {
                if (!result.passed) {
                    std::cout << "   - " << result.testName << ": " << result.message << "\n";
                }
            }
        }

        std::cout << "\n" << (failed == 0 ? "🎉 All tests passed!" : "⚠️  Some tests failed.") << "\n";
    }
};

// Test assertion macros
#define TEST_ASSERT(condition, message) \
    if (!(condition)) { \
        std::cout << "   Assertion failed: " << message << std::endl; \
        return false; \
    }

#define TEST_ASSERT_TRUE(condition) TEST_ASSERT(condition, #condition " should be true")
#define TEST_ASSERT_FALSE(condition) TEST_ASSERT(!(condition), #condition " should be false")
#define TEST_ASSERT_EQUAL(a, b) TEST_ASSERT(a == b, #a " should equal " #b)

// Test registration macro
#define REGISTER_TEST(name, func) \
    TestFramework::getInstance().addTest(name, func)

#endif // TEST_FRAMEWORK_H
