
#pragma once
#include "Item.h"
#include <vector>
#include <stdexcept>
#include <utility>

class MaxHeap {
private:
    std::vector<Item> data;

    bool greaterItem(const Item& a, const Item& b) const {
        if (a.value != b.value)
            return a.value > b.value;

        return a.id > b.id;
    }

    void siftUp(int i) {
        while (i > 0) {
            int parent = (i - 1) / 2;

            if (!greaterItem(data[i], data[parent]))
                break;

            std::swap(data[i], data[parent]);
            i = parent;
        }
    }

    void siftDown(int i) {
        int n = data.size();

        while (true) {
            int largest = i;
            int left = 2 * i + 1;
            int right = 2 * i + 2;

            if (left < n && greaterItem(data[left], data[largest]))
                largest = left;

            if (right < n && greaterItem(data[right], data[largest]))
                largest = right;

            if (largest == i)
                break;

            std::swap(data[i], data[largest]);
            i = largest;
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
