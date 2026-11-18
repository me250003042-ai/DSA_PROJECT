
#pragma once

#include <vector>

struct HashEntry {
    long long id;
    int heapSide;
    bool active;
    bool occupied;
    bool deleted;

    HashEntry() {
        id = -1;
        heapSide = 0;
        active = false;
        occupied = false;
        deleted = false;
    }
};

class HashTable {
private:
    std::vector<HashEntry> table;
    int capacity;
    int count;
    int used;

    int hashFunction(long long id) const {
        return static_cast<int>(
            static_cast<unsigned long long>(id) % capacity
        );
    }

    void rehash(int newCapacity) {
        std::vector<HashEntry> oldTable = table;

        table.clear();
        table.resize(newCapacity);

        capacity = newCapacity;
        count = 0;
        used = 0;

        for (int i = 0; i < (int)oldTable.size(); i++) {
            if (oldTable[i].occupied && !oldTable[i].deleted) {
                insert(oldTable[i].id, oldTable[i].heapSide);

                HashEntry* entry = find(oldTable[i].id);

                if (entry != nullptr) {
                    entry->active = oldTable[i].active;
                }
            }
        }
    }

public:
    HashTable(int size = 200003) {
        capacity = (size < 8) ? 8 : size;
        count = 0;
        used = 0;
        table.resize(capacity);
    }

    bool insert(long long id, int heapSide) {
        if (id <= 0) {
            return false;
        }

        if (find(id) != nullptr) {
            return false;
        }

        if ((used + 1) * 10 >= capacity * 7) {
            rehash(capacity * 2);
        }

        int index = hashFunction(id);
        int firstDeleted = -1;

        for (int probes = 0; probes < capacity; probes++) {
            if (!table[index].occupied) {
                if (firstDeleted != -1) {
                    index = firstDeleted;
                } else {
                    used++;
                }

                table[index].id = id;
                table[index].heapSide = heapSide;
                table[index].active = true;
                table[index].occupied = true;
                table[index].deleted = false;

                count++;
                return true;
            }

            if (table[index].deleted && firstDeleted == -1) {
                firstDeleted = index;
            }

            index = (index + 1) % capacity;
        }

        if (firstDeleted != -1) {
            index = firstDeleted;

            table[index].id = id;
            table[index].heapSide = heapSide;
            table[index].active = true;
            table[index].occupied = true;
            table[index].deleted = false;

            count++;
            return true;
        }

        rehash(capacity * 2);
        return insert(id, heapSide);
    }

    HashEntry* find(long long id) {
        if (id <= 0) {
            return nullptr;
        }

        int index = hashFunction(id);

        for (int probes = 0; probes < capacity; probes++) {
            if (!table[index].occupied) {
                return nullptr;
            }

            if (!table[index].deleted && table[index].id == id) {
                return &table[index];
            }

            index = (index + 1) % capacity;
        }

        return nullptr;
    }

    bool erase(long long id) {
        HashEntry* entry = find(id);

        if (entry == nullptr || !entry->active) {
            return false;
        }

        entry->active = false;
        return true;
    }

    bool remove(long long id) {
        HashEntry* entry = find(id);

        if (entry == nullptr) {
            return false;
        }

        entry->deleted = true;
        entry->active = false;
        count--;

        if (used - count > capacity / 4) {
            rehash(capacity);
        }

        return true;
    }

    int size() const {
        return count;
    }

    int getCapacity() const {
        return capacity;
    }
};
