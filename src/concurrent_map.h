#pragma once

#include <mutex>
#include <map>
#include <vector>

using namespace std::string_literals;

template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);
    
    explicit ConcurrentMap(size_t bucket_count) : n_ranges_(bucket_count),
                                                  raw_vault_(bucket_count),
                                                  mutexes_(bucket_count) {
    }
    
    struct Access {
        Access() = delete;
        explicit Access(const Key& key, 
                        std::vector<std::mutex>& vm,
                        std::vector<std::map<Key, Value>>& data,
                        size_t n) : access_key(static_cast<uint64_t>(key) % n),
                                    guard(vm[access_key]),
                                    ref_to_value((data[access_key])[key]) {
        }
        uint64_t access_key;
        std::unique_lock<std::mutex> guard;
        Value& ref_to_value;
    };

    Access operator[](const Key& key) {
        return Access(key, mutexes_, raw_vault_, n_ranges_);
    }
    
    void Erase(Key key) {
        Access tmp_access_obj(key, mutexes_, raw_vault_, n_ranges_);
        raw_vault_[tmp_access_obj.access_key].erase(key);
    }

    std::map<Key, Value> BuildOrdinaryMap() {
       std::map<Key, Value> result;
       for(size_t i = 0; i < n_ranges_; ++i) {
           std::lock_guard guard(mutexes_[i]);
           result.merge(raw_vault_[i]);
       }
       return result;
    }

private:
    size_t n_ranges_;
    std::vector<typename std::map<Key, Value>> raw_vault_;
    std::vector<std::mutex> mutexes_;
};
