
#pragma once

#include <vector>
#include <stdexcept>

class ExpiryQueue {
private:
    std::vector<long long> data;
    int capacity;
    int front;
    int rear;
    int count;

public:
    ExpiryQueue(int size) {
        if (size <= 0)
            throw std::invalid_argument("Invalid queue capacity");

        capacity = size;
        front = 0;
        rear = 0;
        count = 0;

        data.resize(capacity);
    }

    bool empty() const {
        return count == 0;
    }

    bool full() const {
        return count == capacity;
    }

    int size() const {
        return count;
    }

    void push(long long id) {
        if (full())
            throw std::runtime_error("Queue is full");

        data[rear] = id;
        rear = (rear + 1) % capacity;
        count++;
    }

    long long pop() {
        if (empty())
            throw std::runtime_error("Queue is empty");

        long long id = data[front];
        front = (front + 1) % capacity;
        count--;

        return id;
    }

    long long getFront() const {
        if (empty())
            throw std::runtime_error("Queue is empty");

        return data[front];
    }
};
