
#pragma once
#include <vector>

struct HashEntry {
    long long id;
    int heapSide;
    bool active;
    bool occupied;

    HashEntry() {
        id = -1;
        heapSide = 0;
        active = false;
        occupied = false;
    }
};

class HashTable {
private:
    std::vector<HashEntry> table;
    int capacity;
    int count;

    int hashFunction(long long id) const {
        return id % capacity;
    }

public:
    HashTable(int size = 200003) {
        capacity = size;
        count = 0;
        table.resize(capacity);
    }

    void insert(long long id, int heapSide) {
        int index = hashFunction(id);

        while (table[index].occupied && table[index].id != id) {
            index = (index + 1) % capacity;
        }

        if (!table[index].occupied)
            count++;

        table[index].id = id;
        table[index].heapSide = heapSide;
        table[index].active = true;
        table[index].occupied = true;
    }

    HashEntry* find(long long id) {
        int index = hashFunction(id);
        int start = index;

        while (table[index].occupied) {
            if (table[index].id == id)
                return &table[index];

            index = (index + 1) % capacity;

            if (index == start)
                break;
        }

        return nullptr;
    }

    bool erase(long long id) {
        HashEntry* entry = find(id);

        if (entry == nullptr)
            return false;

        entry->active = false;
        return true;
    }

    int size() const {
        return count;
    }
};
