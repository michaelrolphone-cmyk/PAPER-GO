#include <unity.h>
#include "GoLogic.h"
void test_place_and_turn(){ GoGame g; TEST_ASSERT_TRUE(g.begin(9)); TEST_ASSERT_TRUE(g.place(4,4)); TEST_ASSERT_EQUAL((int)GoGame::Stone::White,(int)g.turn()); TEST_ASSERT_FALSE(g.place(4,4)); }
void test_capture(){ GoGame g; g.begin(9); g.place(1,1); g.place(0,1); g.place(4,4); g.place(1,0); g.place(5,5); g.place(2,1); g.place(6,6); g.place(1,2); TEST_ASSERT_EQUAL((int)GoGame::Stone::Empty,(int)g.at(1,1)); }
int main(){ UNITY_BEGIN(); RUN_TEST(test_place_and_turn); RUN_TEST(test_capture); return UNITY_END(); }
