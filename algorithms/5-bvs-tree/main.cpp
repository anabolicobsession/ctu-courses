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

    int get_depth(Node *n) {
        if (n == nullptr) return -1;
        return 1 + max(get_depth(n->left), get_depth(n->right));
    }

public:
    BinarySearchTree() : root(nullptr) {}

    void insert(int key) {
        root = insert(root, key);
    }

    int get_depth() {
        return get_depth(root);
    }

};

int main() {

    return 0;
}
