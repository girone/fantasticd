#include "UnorderedMapSerializationWorkaround.h"

#include <gmock/gmock.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <fstream>
#include <string>
#include <unordered_map>


const std::string TESTFILE = "serialization.test.TMP";

class SomeContainer
{
 public:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, unsigned int version)
    {
        serializeUnorderedMap(ar, content_);
    }

    std::unordered_map<std::string, int> content_;
};

TEST(UnorderedMapSerializationWorkaroundTest, self)
{
    std::unordered_map<std::string, int> input, compare, output;
    input["hello"] = 7;
    input["hello"] = 7;
    input["now"] = 7;
    input["never"] = 99;
    input["some"] = 42;
    input["know"] = 8;

    compare = input;

    {
        std::ofstream ofs(TESTFILE);
        boost::archive::binary_oarchive oa(ofs);
        serializeUnorderedMap(oa, input);
        EXPECT_EQ(compare, input) << "Serialization should not change the original object.";
    }
    {
        std::ifstream ifs(TESTFILE);
        boost::archive::binary_iarchive ia(ifs);
        serializeUnorderedMap(ia, output);
    }

    EXPECT_EQ(input, output);
}
