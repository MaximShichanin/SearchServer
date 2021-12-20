#pragma once

#include <mutex>
#include <map>
#include <vector>

using namespace std::string_literals;

template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);
    struct Bucket {
        std::mutex m;
        std::map<Key, Value> bucket_vault;
    };
    explicit ConcurrentMap(size_t bucket_count) : buckets_(bucket_count) {
    }
    
    struct Access {
        Access() = delete;
        explicit Access(const Key& key, 
                        std::vector<Bucket>& buckets) : access_key(static_cast<uint64_t>(key) % buckets.size()),
                                                        guard(buckets[access_key].m),
                                                        ref_to_value(buckets[access_key].bucket_vault[key]) {
        }
        uint64_t access_key;
        std::unique_lock<std::mutex> guard;
        Value& ref_to_value;
    };

    Access operator[](const Key& key) {
        return Access(key, buckets_);
    }
    
    void Erase(Key key) {
        Access tmp_access_obj(key, buckets_);
        buckets_[tmp_access_obj.access_key].bucket_vault.erase(key);
    }

    std::map<Key, Value> BuildOrdinaryMap() {
       std::map<Key, Value> result;
       for(size_t i = 0; i < buckets_.size(); ++i) {
           std::lock_guard guard(buckets_[i].m);
           result.merge(buckets_[i].bucket_vault);
       }
       return result;
    }

private:
    std::vector<Bucket> buckets_;
};
