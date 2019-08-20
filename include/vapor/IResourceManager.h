#pragma once

#include <string>
#include <map>
#include "vapor/VAssert.h"
#include "vapor/MyBase.h"

#ifdef WIN32
#ifdef RENDER_EXPORTS
#define IRESOURCEMANAGER_IMPLEMENT
#endif
#else
#define IRESOURCEMANAGER_IMPLEMENT
#endif

namespace VAPoR {

template <typename K, typename T>
class RENDER_API IResourceManager  : public Wasp::MyBase {
protected:
    std::map<K, T*> _map;
    
    T *GetResource(const K &key);
    
public:
    virtual ~IResourceManager();
    bool HasResource(const K &key) const;
    bool HasResource(const T *resource) const;
    virtual int LoadResourceByKey(const K &key) = 0;
    bool AddResource(const K &key, T *resource);
    void DeleteResource(const K &key);
};

#ifdef IRESOURCEMANAGER_IMPLEMENT
template <typename K, typename T>
IResourceManager<K, T>::~IResourceManager() {
    for (auto it = _map.begin(); it != _map.end(); ++it)
        delete it->second;
}
    
template <typename K, typename T>
T* IResourceManager<K, T>::GetResource(const K &key)
{
    auto it = _map.find(key);
    if (it == _map.end()) {
        if (LoadResourceByKey(key) < 0) {
            SetErrMsg("Resource does not exist and unable to load by name");
            return nullptr;
        }
        it = _map.find(key);
    }
    return it->second;
}

template <typename K, typename T>
bool IResourceManager<K, T>::HasResource(const K &key) const
{
    return _map.find(key) != _map.end();
}

template <typename K, typename T>
bool IResourceManager<K, T>::HasResource(const T *resource) const
{
    for (auto it = _map.begin(); it != _map.end(); ++it)
        if (it->second == resource)
            return true;
    return false;
}

template <typename K, typename T>
bool IResourceManager<K, T>::AddResource(const K &key, T *resource)
{
    if (HasResource(key) || HasResource(resource)) {
        VAssert(!"Resource already exists");
        return false;
    }
    _map.insert(std::pair<K, T*>(key, resource));
    return true;
}

template <typename K, typename T>
void IResourceManager<K, T>::DeleteResource(const K &key)
{
    VAssert(HasResource(key));
    delete _map[key];
    _map.erase(key);
}
#endif

}
