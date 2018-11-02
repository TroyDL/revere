
#ifndef _r_utils_r_blob_tree_h
#define _r_utils_r_blob_tree_h

#include "r_utils/r_exception.h"

#include <map>
#include <string>
#include <vector>
#include <stdexcept>

class test_r_utils_r_blob_tree;

namespace r_utils
{

class r_blob_tree
{
    friend class ::test_r_utils_r_blob_tree;

public:
    enum node_type
    {
        NT_OBJECT,
        NT_ARRAY,
        NT_LEAF
    };

    static std::vector<uint8_t> serialize(const r_blob_tree& rt, uint32_t version);

    static r_blob_tree deserialize(const uint8_t* p, size_t size, uint32_t& version);

    r_blob_tree& operator[](const std::string& key)
    {
        if(!_childrenByIndex.empty())
            R_STHROW(r_internal_exception, ("r_blob_tree node cannot be both an array and an object."));
        return _children[key];
    }

    r_blob_tree& operator[](size_t index)
    {
        if(!_children.empty())
            R_STHROW(r_internal_exception, ("r_blob_tree node cannot be both an object and an array."));
        if(_childrenByIndex.size() < (index+1))
            _childrenByIndex.resize(index+1);
        return _childrenByIndex[index];
    }

    size_t size()
    {
        if(!_children.empty())
            R_STHROW(r_internal_exception, ("r_blob_tree node cannot be both an object and an array."));
        return _childrenByIndex.size();
    }

    r_blob_tree& operator=(const std::pair<size_t, const uint8_t*>& payload)
    {
        _payload = payload;
        return *this;
    }

    inline std::pair<size_t, const uint8_t*> get() const
    {
        return _payload;
    }

private:
    static size_t _sizeof_treeb(const r_blob_tree& rt);
    static size_t _write_treeb(const r_blob_tree& rt, uint8_t* p, uint8_t* end);
    static size_t _read_treeb(const uint8_t* p, const uint8_t* end, r_blob_tree& rt);
    inline static size_t _bytes_left(const uint8_t* p, const uint8_t* end) { return end - p; }

    std::map<std::string, r_blob_tree> _children;
    std::vector<r_blob_tree> _childrenByIndex;
    std::pair<size_t, const uint8_t*> _payload;
};

}

#endif