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
// This implementation follows std container naming conventions.
//
// Author: Samuel Li
// Date  : 9/26/2019
//-----------------------------------------------------------------------------

#ifndef UNIQUE_PTR_CACHE_H
#define UNIQUE_PTR_CACHE_H

#include <cstddef>
#include <list>
#include <utility>
#include <memory>

namespace VAPoR {

// Note : Key must support == operator
template<typename Key, typename BigObj> class unique_ptr_cache final {
public:
    // Constructor with the max size specified
    unique_ptr_cache(size_t size) : m_max_size(size > 1 ? size : 1)    // cache max size has to be at least one
    {
    }

    // Given that this cache is intended to be used to keep unique pointers,
    // we don't want to allow any type of copy constructors, so delete them.
    unique_ptr_cache(const unique_ptr_cache &) = delete;
    unique_ptr_cache(const unique_ptr_cache &&) = delete;
    unique_ptr_cache &operator=(const unique_ptr_cache &) = delete;
    unique_ptr_cache &operator=(const unique_ptr_cache &&) = delete;

    size_t max_size() const { return m_max_size; }

    size_t size() const { return m_list.size(); }

    void clear() { m_list.clear(); }

    bool empty() const { return m_list.empty(); }

    // Key must support == operator.
    bool contains(const Key &key) const
    {
        // (Should use const iterator here, but gcc-4.8 on CentOS7 doesn't support...)
        for (auto it = m_list.begin(); it != m_list.end(); ++it) {
            if (it->first == key) {
                // Move the current element to the list head if it's not already there
                if (it != m_list.begin()) m_list.splice(m_list.begin(), m_list, it);

                return true;
            }
        }
        return false;
    }

    // Upon the existance of Key, returns a const pointer pointing to BigObj
    // Upon non-existance of Key, return a nullptr
    const BigObj *find(const Key &key) const
    {
        // We found the key, and now the pair is at the beginning of the list
        if (this->contains(key)) {
            // Get a raw pointer from the unique_ptr
            return (m_list.cbegin()->second).get();
        } else
            return nullptr;
    }

    //
    // Inserts to the head of the cache a key and a raw pointer pointing to a BigObj.
    // Upon success, this class takes ownership of the BigObj and the old raw pointer
    // shall not be used anymore.
    // In the case of the key already exists, the new BigObj takes place of the old
    // BigObj, while the old BigObj is properly destroyed.
    // (Pass by value and move idiom on Key)
    //
    void insert(Key key, const BigObj *ptr)
    {
        // Remove the old pair if the same key already exists.
        // (Should use const iterator here, but gcc-4.8 on CentOS7 doesn't support...)
        for (auto it = m_list.begin(); it != m_list.end(); ++it) {
            if (it->first == key) {
                m_list.erase(it);
                break;    // There's at most one existing key, so OK to break here.
            }
        }

        // Should have used make_unique<> in C++14. GCC-4.8 in CentOS7 prevents it as of 2019.
        std::unique_ptr<const BigObj> tmp(ptr);

        // Create a new pair at the front of the list
        m_list.emplace_front(std::move(key), std::move(tmp));

        if (m_list.size() > m_max_size) m_list.pop_back();
    }

private:
    using list_type = std::list<std::pair<Key, std::unique_ptr<const BigObj>>>;
    mutable list_type m_list;
    const size_t      m_max_size;

};    // end of class unique_ptr_cache

}    // end of namespace VAPoR

#endif
