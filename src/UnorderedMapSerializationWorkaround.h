#ifndef SRC_UNORDEREDMAPSERIALIZATIONWORKAROUND_H_
#define SRC_UNORDEREDMAPSERIALIZATIONWORKAROUND_H_

#include <boost/serialization/map.hpp>
#include <map>
#include <unordered_map>

// A workaround for the yet missing boost::serialization::unordered_map.
// TODO(Jonas): Remove this workaround once serialization for unordered_map is available.
template<class Archive, class U, class V>
void serializeUnorderedMap(Archive& ar, std::unordered_map<U, V>& unorderedMap)
{
    std::map<U, V> tmp;
    tmp.insert(unorderedMap.begin(), unorderedMap.end());
    ar & tmp;

    unorderedMap.clear();
    unorderedMap.insert(tmp.begin(), tmp.end());
}


#endif  // SRC_UNORDEREDMAPSERIALIZATIONWORKAROUND_H_
