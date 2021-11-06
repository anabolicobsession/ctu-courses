#include <iostream>

#include "utils.h"

using namespace std;

class BinarySearchTree {

private:
    typedef int key_t;

    struct Node {
        key_t key;
        Node *left;
        Node *right;
    };

    Node *root;

    Node* insert(Node *n, key_t k) {
        if (n == nullptr) {
            n = new Node;
            n->key = k;
            n->left = n->right = nullptr;
        } else if (k < n->key) {
            n->left = insert(n->left, k);
        } else {
            n->right = insert(n->right, k);
        }

        return n;
    }

public:
    BinarySearchTree() : root(nullptr) {}

    void insert(int key) {
        root = insert(root, key);
    }
};

int main() {
    BinarySearchTree bst;
    bst.insert(3);
    bst.insert(2);
    bst.insert(1);

    return 0;
}
