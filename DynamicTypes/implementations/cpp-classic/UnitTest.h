#ifndef UNITTEST_H
#define UNITTEST_H

// Stub header to avoid compilation errors
// In a real implementation, this would contain unit test infrastructure

// Stub base class for unit tests
class UnitTest {
public:
    UnitTest(const char* name) { (void)name; }
    virtual ~UnitTest() {}
};

// Stub macro for unit test registration
#define RegisterUnitTest(testClass) /* commented out */

// Define Assert macro if not already defined
#ifndef Assert
#define Assert(condition) do { if (!(condition)) { printf("Assert failed: %s at %s:%d\n", #condition, __FILE__, __LINE__); } } while(0)
#endif

#endif /* UNITTEST_H */