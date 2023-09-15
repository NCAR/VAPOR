//-----------------------------------------------------------------------------
// This is an implementation of a least-recently-used (LRU) cache that keeps
// unique pointers pointing to big structures (e.g., grids, quadtrees).
//
// This cache has two execution policies:
// 1) only insertion counts as `recently used`, and
// 2) both insertion and query count as `recently used`.
//
// Given this design, this cache is expected to keep the ownership of these
// structures once they're put in the cache, and all other codes will not
// need to manage these structures.
//
// All structures stored in this cache are const qualified, so once a
// structure is put in this cache, there is no more modification to this structure.
//
// Caveat: A cache keeps things that it is asked to keep, which in this case are pointers.
//         This implementation guarantees that pointers and the objects that they point to
//         are not altered while in the cache, and are properly destroyed when evicted.
//         The cache guarantees no more than that.
//
// Tip:    This cache should be initialized sufficiently big so that a returned
//         pointer won't be evicted while that pointer is in use.
//         In other words, users need to know how many new insersions are going to happen
//         while a queried pointer is in use, and initialize the cache to be at least
//         that big.
//
//         To use an example, a `unique_ptr_cache` is initialized to hold N objects.
//         A queried pointer `ptr` is valid at the time of query, and will remain valid until
//         another (N-1) unique individual objects being inserted/queried.
//         At that point, the immediate next insertion of another unique object (the N-th)
//         will evict `ptr`, and the object it points to is destroyed, and `prt` is no longer valid.
//
// Revision: (8/13/2020) it uses std::array<> instead of std::list<> to achieve
//                       the highest performance with small to medium cache sizes.
// Revision: (8/13/2020) it uses mutexes to achieve thread safety.
// Revision: (9/29/2020) it uses std::vector<> instead of std::array<> so that the cache size
//                       can be set dynamically at construction time.
//
// Author   : Samuel Li
// Date     : 9/26/2019
// Revision : 8/13/2020, 9/29/2020
//-----------------------------------------------------------------------------

#ifndef UNIQUE_PTR_CACHE_H
#define UNIQUE_PTR_CACHE_H

#include <cstddef>    // size_t
#include <utility>    // std::pair<>
#include <memory>     // std::unique_ptr<>
#include <mutex>
#include <algorithm>
#include <vector>

namespace VAPoR {

//
// Note : Key must support == operator
//
template<typename Key, typename BigObj> class unique_ptr_cache final {
public:
    // Constructor
    // A user needs to specify if a query is counted as `recently used`
    // by passing a boolean to the constructor.
    unique_ptr_cache(size_t capacity, bool query) : _capacity(capacity), _query_shuffle(query) { _element_vector.reserve(_capacity); }
    // Note: because this cache is intended to be used to keep unique pointers,
    // we don't want to allow any type of copy constructors, so delete them.
    unique_ptr_cache(const unique_ptr_cache &) = delete;
    unique_ptr_cache(const unique_ptr_cache &&) = delete;
    unique_ptr_cache &operator=(const unique_ptr_cache &) = delete;
    unique_ptr_cache &operator=(const unique_ptr_cache &&) = delete;

    auto capacity() const -> size_t { return _capacity; }

    auto size() const -> size_t { return _element_vector.size(); }

    void clear() { _element_vector.clear(); }

    auto empty() const -> bool { return _element_vector.empty(); }

    auto full() const -> bool { return (_element_vector.size() >= _capacity); }

    //
    // Major action function.
    // If the key exists, it returns the unique pointer associated with the key.
    // If the key does not exist, it returns the unique_ptr version of a nullptr.
    //
    auto query(const Key &key) -> const std::unique_ptr<const BigObj> &
    {
        // Only need to apply the mutex if `_query_shuffle` is enabled.
        if (_query_shuffle) const std::lock_guard<std::mutex> lock_gd(_element_vector_mutex);

        auto it = std::find_if(_element_vector.begin(), _element_vector.end(), [&key](element_type &e) { return e.first == key; });
        if (it == _element_vector.end()) {    // This key does not exist
            return _local_nullptr;
        } else {    // This key does exist
            if (_query_shuffle) {
                std::rotate(_element_vector.begin(), it, it + 1);
                return _element_vector.front().second;
            } else
                return it->second;
        }
    }

    void insert(Key key, const BigObj *ptr)
    {
        const std::lock_guard<std::mutex> lock_gd(_element_vector_mutex);

        auto it = std::find_if(_element_vector.begin(), _element_vector.end(), [&key](element_type &e) { return e.first == key; });

        if (it == _element_vector.end()) {                                          // This key does not exist
            if (_element_vector.size() >= _capacity) _element_vector.pop_back();    // Evict the last element
            std::unique_ptr<const BigObj> tmp(ptr);
            _element_vector.emplace(_element_vector.begin(), std::move(key), std::move(tmp));
        } else {    // This key does exist.
            it->second.reset(ptr);
            std::rotate(_element_vector.begin(), it, it + 1);
        }
    }

private:
    using element_type = std::pair<Key, std::unique_ptr<const BigObj>>;

    const size_t                        _capacity;
    const bool                          _query_shuffle;
    std::vector<element_type>           _element_vector;
    const std::unique_ptr<const BigObj> _local_nullptr = {nullptr};
    std::mutex                          _element_vector_mutex;
};
}    // namespace VAPoR

#endif
