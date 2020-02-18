#include <cassert>
#include <initializer_list>
#include <iostream>
#include <list>
#include <stdexcept>
#include <vector>

template<class KeyType, class ValueType>
class Item {
    public:
        size_t hash;
        KeyType first;
        ValueType second;
        typename std::list<Item<const KeyType, ValueType>>::iterator nextItWithEqualHash;

        Item(size_t hash, const std::pair<KeyType, ValueType>& p, typename std::list<Item<const KeyType, ValueType>>::iterator nextItWithEqualHash) :
                hash(hash), first(p.first), second(p.second), nextItWithEqualHash(nextItWithEqualHash) {}
};

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
    public:
        typedef typename std::list<Item<const KeyType, ValueType>>::iterator iterator;
        typedef typename std::list<Item<const KeyType, ValueType>>::const_iterator const_iterator;

        const iterator UNDEF = iterator();

        HashMap() : TABLE_SIZE(START_SIZE), hashFirstIt(TABLE_SIZE, UNDEF), sz(0) {}
        HashMap(const Hash& hasher) : TABLE_SIZE(START_SIZE), hashFirstIt(TABLE_SIZE, UNDEF), hasher(hasher), sz(0) {}
        
        template<class Iterator>
        HashMap(Iterator begin, Iterator end) : TABLE_SIZE(START_SIZE), hashFirstIt(TABLE_SIZE, UNDEF), sz(0) {
            while (begin != end) {
                insert(*begin);
                ++begin;
            }
        }
        template<class Iterator>
        HashMap(Iterator begin, Iterator end, const Hash& hasher) {
            this->hasher = hasher;
            *this = HashMap(begin, end);
        }

        HashMap(const std::initializer_list<std::pair<const KeyType, ValueType>>& il) : HashMap(il.begin(), il.end()) {}
        HashMap(const std::initializer_list<std::pair<const KeyType, ValueType>>& il, const Hash& hasher) : HashMap(il.begin(), il.end(), hasher) {}

        size_t size() const {
            return sz;
        }

        bool empty() const {
            return items.empty();
        }

        Hash hash_function() const {
            return hasher;
        }

        HashMap operator=(const HashMap& second) {
            items.clear();
            TABLE_SIZE = 2 * second.size();
            hashFirstIt.assign(TABLE_SIZE, UNDEF);
            hasher = second.hasher;
            for (const auto& x : second) {
                insert(x);
            }
            return *this;
        }
/*
        HashMap(const HashMap& second) {
            items.clear();
            TABLE_SIZE = 2 * second.size();
            hashFirstIt.assign(TABLE_SIZE, UNDEF);
            hasher = second.hasher;
            for (const auto& x : second) {
                insert(x);
            }
        }
*/
        void rebuild() {
            for (size_t i = 0; i < TABLE_SIZE; ++i) {
                hashFirstIt[i] = UNDEF;
            }
            TABLE_SIZE *= 2;
            while (hashFirstIt.size() < TABLE_SIZE) {
                hashFirstIt.push_back(UNDEF);
            }
            for (auto iter = items.begin(); iter != items.end(); ++iter) {
                auto& item = *iter;
                size_t hash = item.hash;
                size_t startingId = hash % TABLE_SIZE;
                item.nextItWithEqualHash = hashFirstIt[startingId];
                hashFirstIt[startingId] = iter;
            }
        }

        void clear() {
            TABLE_SIZE = START_SIZE;
            hashFirstIt.assign(TABLE_SIZE, UNDEF);
            items.clear();
            sz = 0;
        }

        void insert(const std::pair<const KeyType, ValueType>& val) {
            size_t hash = hasher(val.first);
            size_t startingId = hash % TABLE_SIZE;
            //size_t cnt = 1;
            for (iterator it = hashFirstIt[startingId]; it != UNDEF; it = it->nextItWithEqualHash) {
                if (it->hash == hash && it->first == val.first) {
                    return;
                }
                //++cnt;
            }
            items.push_back(Item<const KeyType, ValueType>(hash, val, hashFirstIt[startingId]));
            hashFirstIt[startingId] = (--items.end());
            ++sz;
            if (size() > 2 * TABLE_SIZE) {
                rebuild();
            }
        }
        void insert(const Item<const KeyType, ValueType>& item) {
            insert(std::make_pair(item.first, item.second));
        }

        void erase(const KeyType& key) {
            size_t hash = hasher(key);
            size_t startingId = hash % TABLE_SIZE;
            iterator prevIt = UNDEF;
            for (iterator it = hashFirstIt[startingId]; it != UNDEF; it = it->nextItWithEqualHash) {
                if (it->hash == hash && it->first == key) {
                    if (prevIt == UNDEF) {
                        hashFirstIt[startingId] = it->nextItWithEqualHash;
                    } else {
                        prevIt->nextItWithEqualHash = it->nextItWithEqualHash;
                    }
                    items.erase(it);
                    --sz;
                    break;
                }
                prevIt = it;
            }
        }

        iterator begin() {
            return items.begin();
        }
        const_iterator begin() const {
            return items.cbegin();
        }
        iterator end() {
            return items.end();
        }
        const_iterator end() const {
            return items.cend();
        }

        iterator find(const KeyType& key) {
            size_t hash = hasher(key);
            size_t startingId = hash % TABLE_SIZE;
            for (iterator it = hashFirstIt[startingId]; it != UNDEF; it = it->nextItWithEqualHash) {
                if (it->hash == hash && it->first == key) {
                    return it;
                }
            }
            return end();
        }
        const_iterator find(const KeyType& key) const {
            size_t hash = hasher(key);
            size_t startingId = hash % TABLE_SIZE;
            for (const_iterator it = hashFirstIt[startingId]; it != UNDEF; it = it->nextItWithEqualHash) {
                if (it->hash == hash && it->first == key) {
                    return it;
                }
            }
            return end();
        }

        ValueType& operator[](const KeyType& key) {
            size_t hash = hasher(key);
            size_t startingId = hash % TABLE_SIZE;
            //size_t cnt = 1;
            for (iterator it = hashFirstIt[startingId]; it != UNDEF; it = it->nextItWithEqualHash) {
                if (it->hash == hash && it->first == key) {
                    return it->second;
                }
                //++cnt;
            }
            items.push_back(Item<const KeyType, ValueType>(hash, std::make_pair(key, ValueType()), hashFirstIt[startingId]));
            hashFirstIt[startingId] = (--items.end());
            ++sz;
            if (size() > 2 * TABLE_SIZE) {
                rebuild();
            }
            return (--items.end())->second;
        }

        const ValueType& at(const KeyType& key) const {
            const_iterator it = find(key);
            if (it != end()) {
                return it->second;
            }
            throw std::out_of_range("");
        }
    private:
        const size_t START_SIZE = 1;
        const size_t MAX_EQUAL = 3;
        size_t TABLE_SIZE; 
        std::vector<iterator> hashFirstIt;
        std::list<Item<const KeyType, ValueType>> items;
        Hash hasher;
        size_t sz;
};
