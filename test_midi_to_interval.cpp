

#include "midi_to_interval.h"
#include <gtest/gtest.h>



TEST(resultsTest, CountingTwoTriads) {
    EXPECT_EQ(6, run("testData/2Triads.mid"));
}


TEST(resultsTest, CountingEightNotes) {
    EXPECT_EQ(8, run("testData/8Notes.mid"));
}


TEST(resultsTest, FullPieceTest) {
    ASSERT_NEAR(297, run("testData/testMidi.mid"), 10); 
}


TEST(resultsTest, FullPieceTest2) {
    ASSERT_NEAR(33835, run("testData/Beethoven.mid"), 10); 
}


TEST(resultsTest, FullPieceTest3) {
    ASSERT_NEAR(8993, run("testData/Kyrie.mid"), 10); 
}


TEST(resultsTest, FullPieceTest4) {
    ASSERT_NEAR(5930, run("testData/Symphony.mid"), 10);                                        
}



int main(int argc, char **argv) {
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}

      
