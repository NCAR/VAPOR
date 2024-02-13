//-----------------------------------------------------------------------------
// This is an implementation of a least-recently-used (LRU) cache that keeps
// raw pointers pointing to big structures (e.g., grids, quadtrees).
//
// This cache has two execution policies:
// 1) only insertion counts as `recently used`, and
// 2) both insertion and query count as `recently used`.
//
// Given this design, this cache is expected to keep the ownership of these
// structures once they're put in the cache, and all other codes will not
// need to manage these structures. In other words, once an object is evected,
// its destructor will be called.
//
// All structures stored in this cache are const qualified, so once a
// structure is put in this cache, there is no more modification to this structure.
//
// Caveat: A cache keeps things that it is asked to keep, which in this case are pointers.
//         This implementation guarantees that pointers and the objects that they point to
//         are not altered while in the cache, and are properly destroyed when evicted.
//         The cache guarantees no more than that.
//-----------------------------------------------------------------------------

#ifndef PTR_CACHE_H
#define PTR_CACHE_H

#include <cstddef>    // size_t
#include <utility>    // std::pair<>
#include <mutex>
#include <algorithm>
#include <array>

namespace VAPoR {

//
// Note : Key must support == operator and = operator.
//
template<typename Key, typename BigObj, unsigned int Size, bool Query>
class ptr_cache {
public:

    //
    // Destructor
    //
    ~ptr_cache()
    {
      for (auto& p : _element_vector) {
        if (p.second)
          delete p.second;
      }
    }

    auto size() const -> size_t { return _element_vector.size(); }

    //
    // Major action function.
    // If the key exists, it returns the pointer associated with the key.
    // If the key does not exist, it returns a nullptr.
    //
    auto query(const Key &key) -> const BigObj *
    {
        // Only need to apply the mutex if `Query` is true.
        if (Query) const std::lock_guard<std::mutex> lock_gd(_element_vector_mutex);

        auto it = std::find_if(_element_vector.begin(), _element_vector.end(), [&key](element_type &e) { return e.first == key; });
        if (it == _element_vector.end()) {    // This key does not exist
            return nullptr;
        } else {    // This key does exist
            if (!Query) {
                return it->second;
            } else {
                std::rotate(_element_vector.begin(), it, it + 1);
                return _element_vector.front().second;
            }
        }
    }

    void insert(const Key &key, const BigObj *ptr)
    {
        const std::lock_guard<std::mutex> lock_gd(_element_vector_mutex);

        auto it = std::find_if(_element_vector.begin(), _element_vector.end(), [&key](element_type &e) { return e.first == key; });

        if (it == _element_vector.end()) {  // This key does not exist
            --it;                           // `it` points to the last element now.
            it->first = key;
        }
        if (it->second)   // Destroy the old object held here.
          delete it->second;
        it->second = ptr;
        std::rotate(_element_vector.begin(), it, it + 1);
    }

private:
    using element_type = std::pair<Key, const BigObj*>;

    std::mutex                          _element_vector_mutex;
    std::array<element_type, Size>      _element_vector;
};
}    // namespace VAPoR

#endif
