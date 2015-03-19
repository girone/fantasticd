#include "EditDistance.h"
#include <gmock/gmock.h>

TEST(EditDistanceTest, edit_distance)
{
    EXPECT_EQ(5, EditDistance::ed("", "hallo"));
    EXPECT_EQ(5, EditDistance::ed("hello", ""));
    EXPECT_EQ(0, EditDistance::ed("", ""));

    EXPECT_EQ(4, EditDistance::ed("doof", "bloed"));
    EXPECT_EQ(4, EditDistance::ed("bloed", "doof"));

    EXPECT_EQ(7, EditDistance::ed("uni", "university"));
    EXPECT_EQ(5, EditDistance::ed("uniwer", "university"));
}

TEST(EditDistanceTest, prefix_edit_distance)
{
    EXPECT_EQ(3, EditDistance::ped("abc", "uvwxyz"));
    EXPECT_EQ(6, EditDistance::ped("uvwxyz", "abc"));

    EXPECT_EQ(0, EditDistance::ped("uni", "university"));
    EXPECT_EQ(1, EditDistance::ped("uniwer", "university"));
}
