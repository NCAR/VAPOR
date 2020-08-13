//-----------------------------------------------------------------------------
// This is an implementation of a LRU cache that keeps unique pointers 
// pointing to big structures (e.g., grids, quadtrees).
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
//         are not altered, and are properly destroyed when evicted.
//         The cache guarantees no more than that.
//
// Tip:    This cache should be initialized sufficiently big so that a returned 
//         pointer won't be evicted while that pointer is in use.
//         To use an example, a `unique_ptr_cache` is initialized to hold N objects.
//         A queried pointer `ptr` is valid at the time of query, and will remain valid until
//         another (N-1) different individual objects being queried/inserted. 
//         At that point, the immediate next query/insertion of another individual object (the Nth)
//         will evict `ptr`, and the object it points to is destroyed, and `prt` is no longer valid.
//
// Revision: (8/13/2020) it uses std::array<> to achieve the highest performance with
//                       small to medium cache sizes.
// Revision: (8/13/2020) it uses mutexes to achieve thread safety.
//
// Author   : Samuel Li
// Date     : 9/26/2019
// Revision : 8/13/2020
//-----------------------------------------------------------------------------

#ifndef UNIQUE_PTR_CACHE_H
#define UNIQUE_PTR_CACHE_H

#include <cstddef>  // size_t
#include <utility>  // std::pair<>
#include <memory>   // std::unique_ptr<>
#include <array>
#include <mutex>

namespace VAPoR {

//
// Note : Key must support == operator
//
template <typename Key, typename BigObj, size_t CacheCapacity >
class unique_ptr_cache final
{
public:

    // Constructors
    // Note: because this cache is intended to be used to keep unique pointers,
    // we don't want to allow any type of copy constructors, so delete them.
    unique_ptr_cache()                                      = default;
    unique_ptr_cache( const unique_ptr_cache& )             = delete;
    unique_ptr_cache( const unique_ptr_cache&& )            = delete;
    unique_ptr_cache& operator=( const unique_ptr_cache& )  = delete;
    unique_ptr_cache& operator=( const unique_ptr_cache&& ) = delete;

    auto capacity() const -> size_t
    {
        return CacheCapacity;
    }

    auto size() const -> size_t
    {
        return _current_size;
    }

    void clear() 
    {
        _current_size = 0;
    }

    auto empty() const -> bool
    {
        return (_current_size == 0 );
    }

    auto full() const -> bool
    {
        return (_current_size == CacheCapacity );
    }

    //
    // Major action function.
    // If the key exists, it returns the unique pointer associated with the key. 
    // If the key does not exist, it returns the unique_ptr version of a nullptr.
    // 
    auto query( const Key& key ) -> const std::unique_ptr<const BigObj>&
    {
        const std::lock_guard<std::mutex> lock( __element_array_mutex );

        auto it = std::find_if( _element_array.begin(), _element_array.end(), 
                                [&key](element_type& e){return e.first == key;} );
        if( it == _element_array.end() ) {  // This key does not exist
            return _local_nullptr;
        } 
        else {                              // This key does exist
            auto tmp = std::move(*it);
            std::move_backward( _element_array.begin(), it, it + 1 );
            _element_array.front() = std::move(tmp);
            
            return _element_array.front().second;
        }
    }

    void insert( Key key, const BigObj* ptr )
    {
        const std::lock_guard<std::mutex> lock( __element_array_mutex );

        auto it = std::find_if( _element_array.begin(), _element_array.end(), 
                                [&key](element_type& e){return e.first == key;} );

        if( it == _element_array.end() ) {          // This key does not exist
            if( _current_size < CacheCapacity ) {   // This cache is not full
                _element_array[_current_size].first = std::move(key);
                _element_array[_current_size].second.reset(ptr);
                std::rotate( _element_array.begin(), _element_array.begin() + _current_size,
                             _element_array.begin() + _current_size + 1 );
                _current_size++;
            }
            else {                                  // The cache is full!
                _element_array.back().first = std::move(key);
                _element_array.back().second.reset(ptr);
                std::rotate( _element_array.begin(), _element_array.end() - 1, _element_array.end() );
            }
        }
        else {                                      // This key does exist. 
            it->second.reset(ptr);
            auto tmp = std::move(*it);
            std::move_backward( _element_array.begin(), it, it + 1 );
            _element_array.front() = std::move(tmp);
        }
    }

private:
    using element_type = std::pair<Key, std::unique_ptr<const BigObj>>;

    std::array< element_type, CacheCapacity >   _element_array;
    size_t                                      _current_size = 0;
    const std::unique_ptr<const BigObj>         _local_nullptr = {nullptr};
    std::mutex                                  __element_array_mutex;
};

}   // End of VAPoR namespace

#endif
