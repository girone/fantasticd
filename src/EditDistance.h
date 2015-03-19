/*
 * EditDistance.h
 *
 *  Created on: 19.03.2015
 *      Author: jonas
 */

#ifndef EDITDISTANCE_H_
#define EDITDISTANCE_H_

#include <string>


class EditDistance
{
public:
    // Returns the edit distance ED between a and b.
    static unsigned int ed(const std::string& a, const std::string& b, bool ped=false);
    // Returns the prefix edit distance PED between prefix and b.
    static unsigned int ped(const std::string& prefix, const std::string& b);
};


#endif  // EDITDISTANCE_H_
