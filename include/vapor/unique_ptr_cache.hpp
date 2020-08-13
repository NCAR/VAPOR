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
// Caveat: This cache guarantees that a returned non-null pointer is valid at the
//         query time, but cannot guarantee that the pointer keeps being valid after
//         handing it over (i.e., not being evicted). Thus, this cache should be initialized 
//         sufficiently big so that a returned pointer won't be evicted while that pointer
//         is in use.
//
// Revision: (8/13/2020) it uses std::array<> to achieve the highest performance with
//                       small to medium cache sizes.
// Revision: (8/13/2020) it uses mutexes to achieve thread safety.
//
// Author: Samuel Li
// Date  : 9/26/2019
// Revision: 8/13/2020
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
        return current_size;
    }

    void clear() 
    {
        current_size = 0;
    }

    auto empty() const -> bool
    {
        return (current_size == 0 );
    }

    auto full() const -> bool
    {
        return (current_size == CacheCapacity );
    }

    //
    // Major action function.
    // If the key exists, it returns the unique pointer associated with the key. 
    // If the key does not exist, it returns nullptr.
    // 
    auto query( const Key& key ) -> const std::unique_ptr<const BigObj>&
    {
        const std::lock_guard<std::mutex> lock( element_array_mutex );

        auto it = std::find_if( element_array.begin(), element_array.end(), 
                                [&key](element_type& e){return e.first == key;} );
        if( it == element_array.end() ) {   // This key does not exist
            return local_nullptr;
        } 
        else {                              // This key does exist
            auto tmp = std::move(*it);
            std::move_backward( element_array.begin(), it, it + 1 );
            element_array.front() = std::move(tmp);
            
            return element_array.front().second;
        }
    }

    void insert( Key key, const BigObj* ptr )
    {
        const std::lock_guard<std::mutex> lock( element_array_mutex );

        auto it = std::find_if( element_array.begin(), element_array.end(), 
                                [&key](element_type& e){return e.first == key;} );

        if( it == element_array.end() ) {           // This key does not exist
            if( current_size < CacheCapacity ) {    // This cache is not full
                element_array[current_size].first = std::move(key);
                element_array[current_size].second.reset(ptr);
                std::rotate( element_array.begin(), element_array.begin() + current_size,
                             element_array.begin() + current_size + 1 );
                current_size++;
            }
            else {                                  // The cache is full!
                element_array.back().first = std::move(key);
                element_array.back().second.reset(ptr);
                std::rotate( element_array.begin(), element_array.end() - 1, element_array.end() );
            }
        }
        else {                                      // This key does exist. 
            it->second.reset(ptr);
            auto tmp = std::move(*it);
            std::move_backward( element_array.begin(), it, it + 1 );
            element_array.front() = std::move(tmp);
        }
    }

private:
    using element_type = std::pair<Key, std::unique_ptr<const BigObj>> ;
    std::array< element_type, CacheCapacity > element_array;
    size_t current_size = 0;
    std::unique_ptr<const BigObj> local_nullptr;
    std::mutex element_array_mutex;
};

}   // End of VAPoR namespace

#endif
