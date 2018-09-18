#pragma once

#include <string>
#include <map>
#include <cassert>

namespace VAPoR {

template<typename K, typename T> class IResourceManager {
protected:
    std::map<K, T *> _map;
    std::string      _resourceDirectory;

    T *GetResource(const K &key);

public:
    virtual ~IResourceManager();
    bool         HasResource(const K &key) const;
    bool         HasResource(const T *resource) const;
    bool         SetResourceDirectory(const std::string &path);
    virtual bool LoadResourceByKey(const K &key) = 0;
    bool         AddResource(const K &key, T *resource);
    void         DeleteResource(const K &key);
};

template<typename K, typename T> IResourceManager<K, T>::~IResourceManager() {}

template<typename K, typename T> T *IResourceManager<K, T>::GetResource(const K &key)
{
    auto it = _map.find(key);
    if (it == _map.end()) {
        if (!LoadResourceByKey(key)) {
            assert(!"Resource does not exist and unable to load by name");
            return nullptr;
        }
        it = _map.find(key);
    }
    return it->second;
}

template<typename K, typename T> bool IResourceManager<K, T>::HasResource(const K &key) const { return _map.find(key) != _map.end(); }

template<typename K, typename T> bool IResourceManager<K, T>::HasResource(const T *resource) const
{
    for (auto it = _map.begin(); it != _map.end(); ++it)
        if (it->second == resource) return true;
    return false;
}

template<typename K, typename T> bool IResourceManager<K, T>::SetResourceDirectory(const std::string &path)
{
    _resourceDirectory = path;
    return true;
}

template<typename K, typename T> bool IResourceManager<K, T>::AddResource(const K &key, T *resource)
{
    if (HasResource(key) || HasResource(resource)) {
        assert(!"Resource already exists");
        return false;
    }
    _map.insert(std::pair<K, T *>(key, resource));
    return true;
}

template<typename K, typename T> void IResourceManager<K, T>::DeleteResource(const K &key)
{
    assert(HasResource(key));
    delete _map[key];
    _map.erase(key);
}

}    // namespace VAPoR
