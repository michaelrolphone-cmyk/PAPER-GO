#include <unity.h>
#include "ChessLogic.h"

void test_pawn_move_and_turns(){ ChessGame g; g.reset(); TEST_ASSERT_TRUE(g.playMove(52,36)); TEST_ASSERT_EQUAL(ChessGame::Color::Black,g.turn()); TEST_ASSERT_FALSE(g.playMove(51,35)); }
void test_scholars_mate_checkmate(){ ChessGame g; g.reset(); g.playMove(52,36); g.playMove(12,28); g.playMove(59,31); g.playMove(1,18); g.playMove(61,34); g.playMove(6,21); TEST_ASSERT_TRUE(g.playMove(31,13)); TEST_ASSERT_TRUE(g.inCheck(ChessGame::Color::Black)); TEST_ASSERT_TRUE(g.isCheckmate(ChessGame::Color::Black)); }
void test_serialize_roundtrip(){ ChessGame g; g.reset(); g.playMove(52,36); String s=g.serialize(); ChessGame g2; TEST_ASSERT_TRUE(g2.deserialize(s)); TEST_ASSERT_EQUAL((int)g.turn(),(int)g2.turn()); TEST_ASSERT_EQUAL((int)g.at(36).piece,(int)g2.at(36).piece); }
int main(){ UNITY_BEGIN(); RUN_TEST(test_pawn_move_and_turns); RUN_TEST(test_scholars_mate_checkmate); RUN_TEST(test_serialize_roundtrip); return UNITY_END(); }
