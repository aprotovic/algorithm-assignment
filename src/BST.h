#ifndef BST_H
#define BST_H

#include "Models.h"
#include "LinkedList.h"
#include <cstddef>

class BSTNode {
public:
    Listing* data;
    BSTNode* left;
    BSTNode* right;

    BSTNode(Listing* val) : data(val), left(NULL), right(NULL) {}
};

class BST {
private:
    BSTNode* root;

    void insertRec(BSTNode*& node, Listing* data) {
        if (node == NULL) {
            node = new BSTNode(data);
        } else if (data->price < node->data->price) {
            insertRec(node->left, data);
        } else {
            insertRec(node->right, data);
        }
    }

    void inorderRec(BSTNode* node, LinkedList<Listing*>& list) {
        if (node != NULL) {
            inorderRec(node->left, list);
            list.add(node->data);
            inorderRec(node->right, list);
        }
    }

    void clearRec(BSTNode* node) {
        if (node != NULL) {
            clearRec(node->left);
            clearRec(node->right);
            delete node;
        }
    }

public:
    BST() : root(NULL) {}
    ~BST() { clear(); }

    void insert(Listing* data) {
        insertRec(root, data);
    }

    void getSortedByPrice(LinkedList<Listing*>& list) {
        inorderRec(root, list);
    }

    void clear() {
        clearRec(root);
        root = NULL;
    }
};

#endif
