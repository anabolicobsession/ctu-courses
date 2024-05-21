#include <functional>
#include "bst_tree.h"

void bst_tree::insert(long long data) {
    node *new_node = new node(data);
    node *null_node = nullptr;

    if (root.compare_exchange_strong(null_node, new_node)) {
        return;
    }

    node *current = root;

    while (true) {
        null_node = nullptr;

        if (new_node->data < current->data) {
            if (current->left != nullptr) {
                current = current->left;
            } else if (current->left.compare_exchange_strong(null_node, new_node)) {
                return;
            }
        } else {
            if (current->right != nullptr) {
                current = current->right;
            } else if (current->right.compare_exchange_strong(null_node, new_node)) {
                return;
            }
        }
    }
}

bst_tree::~bst_tree() {
    std::function<void(node*)> cleanup = [&](node * n) {
        if(n != nullptr) {
            cleanup(n->left);
            cleanup(n->right);

            delete n;
        }
    };
    cleanup(root);
}
