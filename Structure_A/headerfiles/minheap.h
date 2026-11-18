
#pragma once
#include "Item.h"
#include "Counters.h"
#include <vector>
#include <stdexcept>
#include <utility>

class MinHeap {
private:
    std::vector<Item> data;

    bool smallerItem(const Item& a, const Item& b) const {
        COUNT(g_heap_compares);

        if (a.value != b.value)
            return a.value < b.value;

        return a.id < b.id;
    }

    void siftUp(int i) {
        while (i > 0) {
            int parent = (i - 1) / 2;

            if (!smallerItem(data[i], data[parent]))
                break;

            COUNT(g_heap_swaps);
            std::swap(data[i], data[parent]);
            i = parent;
        }
    }

    void siftDown(int i) {
        int n = data.size();

        while (true) {
            int smallest = i;
            int left = 2 * i + 1;
            int right = 2 * i + 2;

            if (left < n && smallerItem(data[left], data[smallest]))
                smallest = left;

            if (right < n && smallerItem(data[right], data[smallest]))
                smallest = right;

            if (smallest == i)
                break;

            COUNT(g_heap_swaps);
            std::swap(data[i], data[smallest]);
            i = smallest;
        }
    }

public:
    bool empty() const {
        return data.empty();
    }

    int size() const {
        return data.size();
    }

    const Item& top() const {
        if (empty())
            throw std::runtime_error("Heap is empty");

        return data[0];
    }

    void push(const Item& x) {
        data.push_back(x);
        siftUp(data.size() - 1);
    }

    Item pop() {
        if (empty())
            throw std::runtime_error("Heap is empty");

        Item result = data[0];

        data[0] = data.back();
        data.pop_back();

        if (!empty())
            siftDown(0);

        return result;
    }

    void build(const std::vector<Item>& items) {
        data = items;

        for (int i = (int)data.size() / 2 - 1; i >= 0; i--)
            siftDown(i);
    }

    const std::vector<Item>& items() const {
        return data;
    }
};
