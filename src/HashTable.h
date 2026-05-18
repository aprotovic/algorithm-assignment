#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "Models.h"
#include <cstddef>

class HashTable {
private:
    static const int TABLE_SIZE = 100;
    struct HashNode {
        int key;
        Listing* value;
        HashNode* next;
        HashNode(int k, Listing* v) : key(k), value(v), next(NULL) {}
    };
    HashNode* table[TABLE_SIZE];

    int hashFunction(int key) {
        return key % TABLE_SIZE;
    }

public:
    HashTable() {
        for (int i = 0; i < TABLE_SIZE; i++) table[i] = NULL;
    }

    ~HashTable() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            HashNode* entry = table[i];
            while (entry != NULL) {
                HashNode* prev = entry;
                entry = entry->next;
                delete prev;
            }
        }
    }

    void insert(int key, Listing* value) {
        int hashValue = hashFunction(key);
        HashNode* prev = NULL;
        HashNode* entry = table[hashValue];

        while (entry != NULL && entry->key != key) {
            prev = entry;
            entry = entry->next;
        }

        if (entry == NULL) {
            entry = new HashNode(key, value);
            if (prev == NULL) {
                table[hashValue] = entry;
            } else {
                prev->next = entry;
            }
        } else {
            entry->value = value;
        }
    }

    Listing* search(int key) {
        int hashValue = hashFunction(key);
        HashNode* entry = table[hashValue];
        while (entry != NULL) {
            if (entry->key == key) return entry->value;
            entry = entry->next;
        }
        return NULL;
    }

    void remove(int key) {
        int hashValue = hashFunction(key);
        HashNode* entry = table[hashValue];
        HashNode* prev = NULL;

        while (entry != NULL && entry->key != key) {
            prev = entry;
            entry = entry->next;
        }

        if (entry == NULL) return;

        if (prev == NULL) {
            table[hashValue] = entry->next;
        } else {
            prev->next = entry->next;
        }
        delete entry;
    }
};

#endif
