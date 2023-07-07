//
// Created by giacomo on 07/06/23.
//

#include"BPTree.h"
using namespace NCL::CSC8503;

#ifndef cut
#define cut(length) (((length) % 2 == 0) ? ((length)/2) : ((length)/2+1))
#endif 
/// Non static-inline

node* NCL::CSC8503::delete_entry(node* root, node* n, long long key, void* pointer) {
    int min_keys;
    node* neighbor;
    int neighbor_index;
    int k_prime_index, k_prime;
    int capacity;

    n = remove_entry_from_node(n, key, (node*)pointer);

    if (n == root)
        return adjust_root(root);

    min_keys = n->is_leaf ? cut(n->order - 1) : cut(n->order) - 1;

    if (n->num_keys >= min_keys)
        return root;

    neighbor_index = get_neighbor_index(n);
    k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
    k_prime = n->parent->keys[k_prime_index];
    neighbor = static_cast<node*>(neighbor_index == -1 ? n->parent->pointers[1] : n->parent->pointers[neighbor_index]);

    capacity = n->is_leaf ? n->order : n->order - 1;

    if (neighbor->num_keys + n->num_keys < capacity)
        return NCL::CSC8503::coalesce_nodes(root, n, neighbor, neighbor_index, k_prime);
    else
        return redistribute_nodes(root, n, neighbor, neighbor_index, k_prime_index, k_prime);
}

//static inline
node* NCL::CSC8503::coalesce_nodes(node* root, node* n, node* neighbor, int neighbor_index, long long k_prime) {
    int i, j, neighbor_insertion_index, n_end;
    node* tmp;

    if (neighbor_index == -1) {
        tmp = n;
        n = neighbor;
        neighbor = tmp;
    }

    neighbor_insertion_index = neighbor->num_keys;

    if (!n->is_leaf) {
        neighbor->keys[neighbor_insertion_index] = k_prime;
        neighbor->num_keys++;

        n_end = n->num_keys;

        for (i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++) {
            neighbor->keys[i] = n->keys[j];
            neighbor->pointers[i] = n->pointers[j];
            neighbor->num_keys++;
            n->num_keys--;
        }

        neighbor->pointers[i] = n->pointers[j];

        for (i = 0; i < neighbor->num_keys + 1; i++) {
            tmp = (node*)neighbor->pointers[i];
            tmp->parent = neighbor;
        }
    }

    else {
        for (i = neighbor_insertion_index, j = 0; j < n->num_keys; i++, j++) {
            neighbor->keys[i] = n->keys[j];
            neighbor->pointers[i] = n->pointers[j];
            neighbor->num_keys++;
        }
        neighbor->pointers[neighbor->order - 1] = n->pointers[n->order - 1];
    }

    root = NCL::CSC8503::delete_entry(root, n->parent, k_prime, n);
    free(n->keys);
    free(n->pointers);
    free(n);
    return root;
}

node* NCL::CSC8503::insert_into_parent(node* root, node* left, long long key, node* right, int ord) {
    int left_index;
    node* parent;

    parent = left->parent;

    if (parent == nullptr)
        return insert_into_new_root(left, key, right, ord);

    left_index = get_left_index(parent, left);

    if (parent->num_keys < parent->order - 1)
        return insert_into_node(root, parent, left_index, key, right);

    return NCL::CSC8503::insert_into_node_after_splitting(root, parent, left_index, key, right, ord);
}

node* NCL::CSC8503::insert_into_node_after_splitting(node* root, node* old_node, int left_index, long long key, node* right, int ord) {
    int i, j, split, k_prime;
    node* new_node, * child;
    int* temp_keys;
    node** temp_pointers;

    temp_pointers = static_cast<node**>(malloc((old_node->order + 1) * sizeof(node*)));
    if (temp_pointers == nullptr) {
        perror("Temporary pointers array for splitting nodes.");
        exit(EXIT_FAILURE);
    }
    temp_keys = static_cast<int*>(malloc(old_node->order * sizeof(int)));
    if (temp_keys == nullptr) {
        perror("Temporary keys array for splitting nodes.");
        exit(EXIT_FAILURE);
    }

    for (i = 0, j = 0; i < old_node->num_keys + 1; i++, j++) {
        if (j == left_index + 1)
            j++;
        temp_pointers[j] = static_cast<node*>(old_node->pointers[i]);
    }

    for (i = 0, j = 0; i < old_node->num_keys; i++, j++) {
        if (j == left_index)
            j++;
        temp_keys[j] = old_node->keys[i];
    }

    temp_pointers[left_index + 1] = right;
    temp_keys[left_index] = key;

    split = cut(old_node->order);
    new_node = make_node(old_node->order);
    old_node->num_keys = 0;
    for (i = 0; i < split - 1; i++) {
        old_node->pointers[i] = temp_pointers[i];
        old_node->keys[i] = temp_keys[i];
        old_node->num_keys++;
    }
    old_node->pointers[i] = temp_pointers[i];
    k_prime = temp_keys[split - 1];
    for (++i, j = 0; i < new_node->order; i++, j++) {
        new_node->pointers[j] = temp_pointers[i];
        new_node->keys[j] = temp_keys[i];
        new_node->num_keys++;
    }
    new_node->pointers[j] = temp_pointers[i];
    free(temp_pointers);
    free(temp_keys);
    new_node->parent = old_node->parent;
    for (i = 0; i <= new_node->num_keys; i++) {
        child = static_cast<node*>(new_node->pointers[i]);
        child->parent = new_node;
    }

    return NCL::CSC8503::insert_into_parent(root, old_node, k_prime, new_node, ord);
}
