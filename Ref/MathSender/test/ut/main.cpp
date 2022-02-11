#include "Tester.hpp"
#include "STest/Random/Random.hpp"

TEST(Nominal, AddCommand) {
    Ref::Tester tester;
    tester.testAddCommand();
}

TEST(Nominal, SubCommand) {
    Ref::Tester tester;
    tester.testSubCommand();
}

TEST(Nominal, MulCommand) {
    Ref::Tester tester;
    tester.testMulCommand();
}

TEST(Nominal, DivCommand) {
    Ref::Tester tester;
    tester.testDivCommand();
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    STest::Random::seed();
    return RUN_ALL_TESTS();
}