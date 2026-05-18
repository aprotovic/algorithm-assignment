#ifndef QUEUE_H
#define QUEUE_H

#include <cstddef>

template <typename T>
class QueueNode {
public:
    T data;
    QueueNode* next;
    QueueNode(T val) : data(val), next(NULL) {}
};

template <typename T>
class Queue {
private:
    QueueNode<T>* front;
    QueueNode<T>* rear;
    int size;

public:
    Queue() : front(NULL), rear(NULL), size(0) {}

    ~Queue() {
        while (!isEmpty()) {
            dequeue();
        }
    }

    void enqueue(T item) {
        QueueNode<T>* newNode = new QueueNode<T>(item);
        if (rear == NULL) {
            front = rear = newNode;
        } else {
            rear->next = newNode;
            rear = newNode;
        }
        size++;
    }

    T dequeue() {
        if (isEmpty()) return T();
        QueueNode<T>* temp = front;
        T data = temp->data;
        front = front->next;
        if (front == NULL) rear = NULL;
        delete temp;
        size--;
        return data;
    }

    T peek() const {
        return isEmpty() ? T() : front->data;
    }

    bool isEmpty() const {
        return front == NULL;
    }

    int getSize() const {
        return size;
    }
};

#endif
