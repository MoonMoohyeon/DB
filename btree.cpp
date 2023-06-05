#include "btree.hpp"
#include <iostream> 

btree::btree(){
	root = new page(LEAF);
	height = 1;
};

void btree::insert(char *key, uint64_t val){
	// Please implement this function in project 2.
	page* leaf = root;
    int leaf_height = 1;
    while (leaf->get_type() != LEAF) {
        int i;
        uint32_t num_data = leaf->hdr.get_num_data();
        uint16_t* offset_array = (uint16_t*)leaf->hdr.get_offset_array();
        for (i = 0; i < num_data; i++) {
            uint16_t off = offset_array[i];
            void* data_region = (void*)((uint64_t)leaf + (uint64_t)off);
            char* stored_key = leaf->get_key(data_region);
            if (strcmp(key, stored_key) < 0) {
                break;
            }
        }
        void* child_ptr = (void*)((uint64_t)leaf + offset_array[i - 1]);
        leaf = (page*)child_ptr;
        leaf_height++;
    }

    uint32_t num_data = leaf->hdr.get_num_data();
    uint16_t* offset_array = (uint16_t*)leaf->hdr.get_offset_array();
    uint16_t record_size = sizeof(uint16_t) + strlen(key) + 1 + sizeof(uint64_t);
    if (leaf->is_full(record_size)) {
        char* parent_key = nullptr;
        page* new_page = leaf->split(key, val, &parent_key);
        if (leaf == root) {
            root = new page(INTERNAL);
            root->set_leftmost_ptr(leaf);
            root->insert(parent_key, 0, nullptr);
            root->insert(parent_key, 0, nullptr);
            root->insert(parent_key, 0, new_page);
            height++;
        } else {
            leaf = leaf->get_parent();
            while (leaf != nullptr) {
                if (leaf->is_full(record_size)) {
                    char* parent_key = nullptr;
                    new_page = leaf->split(key, val, &parent_key);
                    leaf = leaf->get_parent();
                } else {
                    leaf->insert(parent_key, 0, new_page);
                    break;
                }
            }
        }
    } else {
        leaf->insert(key, val, nullptr);
    }
}

uint64_t btree::lookup(char *key){
	// Please implement this function in project 2.
	page* node = root;
    while (node != nullptr) {
        if (node->get_type() == LEAF) {
            uint64_t result = node->find(key);
            if (result != 0) {
                return result;
            }
            break;
        }
        int i;
        uint32_t num_data = node->hdr.get_num_data();
        uint16_t* offset_array = (uint16_t*)node->hdr.get_offset_array();
        for (i = 0; i < num_data; i++) {
            uint16_t off = offset_array[i];
            void* data_region = (void*)((uint64_t)node + (uint64_t)off);
            char* stored_key = node->get_key(data_region);
            if (strcmp(key, stored_key) < 0) {
                break;
            }
        }
        void* child_ptr = (void*)((uint64_t)node + offset_array[i - 1]);
        node = (page*)child_ptr;
    }
    return 0;
}
