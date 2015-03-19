/*
 * EditDistance.cpp
 *
 *  Created on: 19.03.2015
 *      Author: jonas
 */

#include "./EditDistance.h"
#include <algorithm>
#include <cassert>
#include <string>

unsigned int EditDistance::ed(const std::string& a, const std::string& b, bool ped)
{
    if (a.size() == 0)
    {
        return b.size();
    }
    else if (b.size() == 0)
    {
        return a.size();
    }
    // Initialize the array.
    // TODO(Jonas): If necessary, speed this up by maintaining just two arrays
    // or allocating just once and reuse this buffer.
    unsigned int array[a.size() + 1][b.size() + 1];
    for (size_t i = 0; i < a.size() + 1; ++i)
    {
        array[i][0] = i;
    }
    for (size_t j = 0; j < b.size() + 1; ++j)
    {
        array[0][j] = j;
    }

    // Compute EDs.
    for (size_t i = 1; i < a.size() + 1; ++i)
    {
        for (size_t j = 1; j < b.size() + 1; ++j)
        {
            bool equal = (tolower(a[i - 1]) == tolower(b[j - 1]));
            unsigned int x = array[i - 1][j - 1] + (equal ? 0 : 1);
            unsigned int y = array[i - 1][j] + 1;
            unsigned int z = array[i][j - 1] + 1;
            array[i][j] = std::min(x, std::min(y, z));
        }
    }

    if (ped)
    {
        // The prefix edit distance is the minimum of the last row.
        unsigned int min = -1;
        for (size_t j = 0; j < b.size() + 1; ++j)
        {
            min = std::min(min, array[a.size()][j]);
        }
        return min;
    }
    else
    {
        return array[a.size()][b.size()];
    }
}

unsigned int EditDistance::ped(const std::string& prefix, const std::string& b)
{
    // TODO(Jonas): If necessary, implement the optimization the lecture.
    // (see lecture 5, WS1314, slide 18).
    return ed(prefix, b, true);
}
