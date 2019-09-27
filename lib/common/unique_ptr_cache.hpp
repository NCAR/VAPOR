//-----------------------------------------------------------------------------
// This is an implementation of a LRU cache that keeps unique pointers 
// pointing to big structures (e.g., grids, quadtrees).
//
// Given this design, this cache is expected to keep the ownership of these 
// structures once they're put in the cache, and all other codes will only
// have read-only access to the structures in the cache.
//
// This implementation follows std container conventions in terms of behaviors,
// API, and names.
//
// Author: Samuel Li
// Date  : 9/26/2019
//-----------------------------------------------------------------------------

#ifndef VAPOR_CACHE_H
#define VAPOR_CACHE_H

#include <cstddef>
#include <list>
#include <utility>

//namespace VAPoR
//{

// Note : Key must support == operator
// Note2: Don't instantiate this class with reference typenames. 
template <typename Key, typename Value>
class vapor_cache
{
public:
    using const_iterator = typename std::list<std::pair<Key, Value>>::const_iterator;

    // Constructor with the size limit specified
    vapor_cache( size_t size )
      : m_size_limit( size > 1 ? size : 1 )   // cache size limit has to be at least one 
    { }
    
    // Given that this cache is intended to be used to keep unique pointers,
    // we don't want to allow any type of copy constructors, so delete them.
    vapor_cache( const vapor_cache& )             = delete;
    vapor_cache( const vapor_cache&& )            = delete;
    vapor_cache& operator=( const vapor_cache& )  = delete;
    vapor_cache& operator=( const vapor_cache&& ) = delete;

    size_t size() const
    {
        return m_size_limit;
    }

    bool empty() const
    {
        return m_list.empty();
    }

    // Key must support == operator.
    bool contains( const Key& key ) const
    {
        for( auto it = m_list.cbegin(); it != m_list.cend(); ++it )
        {
            if( it->first == key )
                return true;
        }
        return false;
    }

    // Upon the existance of Key, returns a const iterator pointing to that pair. 
    // Upon non-existance of Key, returns a const iterator pointing to m_list.cend();
    const_iterator find( const Key& key ) const
    {
        for( auto it = m_list.cbegin(); it != m_list.cend(); ++it )
        { 
            if( it->first == key )
                return it;
        }
        return m_list.cend();
    }

    const_iterator cend() const
    {
        return m_list.cend();
    }


private:
    const size_t m_size_limit;
    std::list< std::pair<Key, Value> > m_list;

};  // end of class VaporCache

//}   // end of namespace VAPoR

#endif
