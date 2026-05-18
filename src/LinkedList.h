#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <cstddef>

template <typename T>
class Node {
public:
    T data;
    Node* next;

    Node(T value) : data(value), next(NULL) {}
};

template <typename T>
class LinkedList {
private:
    Node<T>* head;
    Node<T>* tail;
    int size;

public:
    LinkedList() : head(NULL), tail(NULL), size(0) {}

    LinkedList(const LinkedList<T>& other) : head(NULL), tail(NULL), size(0) {
        Node<T>* current = other.head;
        while (current != NULL) {
            add(current->data);
            current = current->next;
        }
    }

    LinkedList<T>& operator=(const LinkedList<T>& other) {
        if (this != &other) {
            clear();
            Node<T>* current = other.head;
            while (current != NULL) {
                add(current->data);
                current = current->next;
            }
        }
        return *this;
    }

    ~LinkedList() {
        clear();
    }

    void add(T item) {
        Node<T>* newNode = new Node<T>(item);
        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        size++;
    }

    bool remove(int index) {
        if (index < 0 || index >= size) return false;
        
        Node<T>* temp = head;
        if (index == 0) {
            head = head->next;
            delete temp;
            if (!head) tail = NULL;
        } else {
            Node<T>* prev = NULL;
            for (int i = 0; i < index; i++) {
                prev = temp;
                temp = temp->next;
            }
            prev->next = temp->next;
            if (temp == tail) tail = prev;
            delete temp;
        }
        size--;
        return true;
    }

    T* get(int index) {
        if (index < 0 || index >= size) return NULL;
        Node<T>* temp = head;
        for (int i = 0; i < index; i++) {
            temp = temp->next;
        }
        return &(temp->data);
    }

    int getSize() const {
        return size;
    }

    void clear() {
        Node<T>* current = head;
        while (current != NULL) {
            Node<T>* next = current->next;
            delete current;
            current = next;
        }
        head = tail = NULL;
        size = 0;
    }
};

#endif
