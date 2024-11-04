#include "../lib/ArduinoMock.h"
#include "../lib/unity/unity.h"
#include "../ESPDigitalFilter/hysteresis.h"
#include "ESPTests.h"

// Test hysteresis function with various initial conditions
void testHysteresis() {
    float arr1[] = {100.0};
    TEST_ASSERT_EQUAL_FLOAT(100.0, hysteresis(arr1, 1));
    float arr2[] = {50.0, 150.0};

    TEST_ASSERT_EQUAL_FLOAT(100.0, hysteresis(arr2, 2));

    float arr3[] = {10.0, 20.0, 30.0, 40.0, 50.0};
    TEST_ASSERT_EQUAL_FLOAT(30.0, hysteresis(arr3, 5));

    float arr4[] = {100.0, 200.0, 300.0, 400.0, 500.0};
    TEST_ASSERT_EQUAL_FLOAT(300.0, hysteresis(arr4, 5)); 

    float arr5[] = {-10.0, -20.0, -30.0, -40.0, -50.0};
    TEST_ASSERT_EQUAL_FLOAT(-30.0, hysteresis(arr5, 5)); 
}

void setUp(void) {}

void tearDown(void) {}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(testHysteresis);
    return UNITY_END();
}
