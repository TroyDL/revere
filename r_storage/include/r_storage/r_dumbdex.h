
#ifndef r_storage_r_dumbdex_h_
#define r_storage_r_dumbdex_h_

#include <string>
#include <cstdint>
#include <map>
#include <vector>

class test_r_storage;

namespace r_storage
{

const size_t INDEX_ELEMENT_SIZE = 10;
const size_t FREEDEX_ELEMENT_SIZE = 2;

class r_dumbdex final
{
    friend class ::test_r_storage;
public:
    class iterator
    {
    public:
        iterator(
            const r_dumbdex& dex,
            const uint8_t* iter
        ) :
            _dex(dex),
            _iter(iter)
        {
        }

        std::pair<uint64_t, uint16_t> operator*() const
        {
            return _dex._read_index(_iter);
        }

        bool operator==(const iterator& other) const
        {
            return _iter == other._iter;
        }

        bool next()
        {
            auto e = _dex._index + (*_dex._n_indexes * INDEX_ELEMENT_SIZE);
            if(_iter != e)
            {
                _iter += INDEX_ELEMENT_SIZE;
                return true;
            }
            return false;
        }

        bool prev()
        {
            if(_iter != _dex._index)
            {
                _iter -= INDEX_ELEMENT_SIZE;
                return true;
            }

            return false;
        }

        bool valid() const
        {
            auto e = _dex._index + (*_dex._n_indexes * INDEX_ELEMENT_SIZE);
            return _iter != e;
        }

    private:
        const r_dumbdex& _dex;
        const uint8_t* _iter;
    };

    r_dumbdex();
    r_dumbdex(uint8_t* p, size_t max_indexes);
    r_dumbdex(const r_dumbdex&) = delete;
    r_dumbdex(r_dumbdex&& other) noexcept;

    ~r_dumbdex();

    r_dumbdex& operator=(const r_dumbdex&) = delete;
    r_dumbdex& operator=(r_dumbdex&& other) noexcept;

    iterator begin() const {return iterator(*this, _index);}
    iterator end() const {return iterator(*this, _index + (*_n_indexes * INDEX_ELEMENT_SIZE));}

    iterator find_lower_bound(int64_t ts) const;

    uint16_t insert(uint64_t ts);
    void remove(uint64_t ts);
    void print();

    static void allocate(uint8_t* p, size_t block_size, size_t num_blocks);

    static uint32_t max_indexes_within(size_t block_size);

private:
    static uint32_t _index_entry_size() { return sizeof(uint64_t) + sizeof(uint16_t); }
    static uint32_t _freedex_entry_size() { return _index_entry_size(); }
    static uint32_t _index_size(uint32_t max_indexes) { return sizeof(uint32_t) + (max_indexes * _index_entry_size()); }
    static uint32_t _freedex_size(uint32_t max_indexes) { return _index_size(max_indexes); }
    static uint32_t _dumbdex_size(uint32_t max_indexes) { return _index_size(max_indexes) + _freedex_size(max_indexes); }
    void _push_free_block(uint16_t blk);
    bool _verify_index_state(std::vector<std::pair<uint64_t, uint16_t>> cmp);
    void _write_index(uint8_t* p, uint64_t ts, uint16_t blk);
    std::pair<uint64_t, uint16_t> _read_index(const uint8_t* p) const;
    uint32_t _num_indexes() const;

    FILE* _open_journal();
    void _close_journal(FILE* f);
    void _remove_journal();
    void _append_change(FILE* f, const uint8_t* base, const uint8_t* p, size_t size);
    void _rollback_changes(uint8_t* base);
    bool _needs_rollback();

    uint8_t* _block;
    size_t _max_indexes;
    uint32_t* _n_indexes;
    uint8_t* _index;
    uint32_t* _n_free;
    uint16_t* _freedex;
};

}

#endif
