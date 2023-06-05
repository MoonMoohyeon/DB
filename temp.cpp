uint64_t page::find(char *key) {
    uint32_t num_data = hdr.get_num_data();
    uint16_t* offset_array = (uint16_t*)hdr.get_offset_array();

    if (get_type() == LEAF) {
        for (int i = 0; i < num_data; i++) {
            uint16_t off = offset_array[i];
            void* data_region = (void*)((uint64_t)this + (uint64_t)off);
            char* stored_key = get_key(data_region);
            uint64_t stored_val = get_val((void*)stored_key);

            if (strcmp(stored_key, key) == 0) {
                return stored_val;
            }
        }
    } else {
        int i;
        for (i = 0; i < num_data; i++) {
            uint16_t off = offset_array[i];
            void* data_region = (void*)((uint64_t)this + (uint64_t)off);
            char* stored_key = get_key(data_region);

            if (strcmp(key, stored_key) < 0) {
                break;
            }
        }
        void* child_ptr = (void*)((uint64_t)this + offset_array[i - 1]);
        page* child_page = (page*)child_ptr;
        return child_page->find(key);
    }
    
    return 0; // Key not found
}

page* page::split(char *key, uint64_t val, char** parent_key) {
    if (get_type() == LEAF) {
        page *new_page = new page(LEAF);
        new_page->set_leftmost_ptr(get_leftmost_ptr());

        uint32_t num_data = hdr.get_num_data();
        uint16_t* offset_array = (uint16_t*)hdr.get_offset_array();
        uint16_t record_size = sizeof(uint16_t) + strlen(key) + 1 + sizeof(uint64_t);

        if (is_full(record_size)) {
            defrag();
            num_data = hdr.get_num_data();
            offset_array = (uint16_t*)hdr.get_offset_array();
        }

        int insert_pos = 0;
        for (; insert_pos < num_data; insert_pos++) {
            uint16_t off = offset_array[insert_pos];
            void* record_ptr = (void*)((char*)this + (uint64_t)off);
            char* stored_key = get_key(record_ptr);

            if (strcmp(key, stored_key) < 0) {
                break;
            }
        }

        for (int i = num_data - 1; i >= insert_pos; i--) {
            memcpy(offset_array + (i + 1), offset_array + i, sizeof(uint16_t));
        }

        uint16_t region_off = hdr.get_data_region_off();

        void* new_record = (char*)this + region_off - record_size;
        put2byte(new_record, record_size);
        strcpy((char*)new_record + sizeof(uint16_t), key);
        memcpy((char*)new_record + sizeof(uint16_t) + strlen(key) + 1, &val, sizeof(val));

        offset_array[insert_pos] = region_off - record_size;

        hdr.set_num_data(num_data + 1);
        hdr.set_data_region_off(region_off - record_size);

        *parent_key = strdup(key);

        return new_page;
    } else {
        uint32_t num_data = hdr.get_num_data();
        uint16_t* offset_array = (uint16_t*)hdr.get_offset_array();

        int i;
        for (i = 0; i < num_data; i++) {
            uint16_t off = offset_array[i];
            void* data_region = (void*)((uint64_t)this + (uint64_t)off);
            char* stored_key = get_key(data_region);

            if (strcmp(key, stored_key) < 0) {
                break;
            }
        }

        void* child_ptr = (void*)((uint64_t)this + offset_array[i - 1]);
        page* child_page = (page*)child_ptr;
        return child_page->split(key, val, parent_key);
    }
}

void btree::insert(char *key, uint64_t val) {
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

uint64_t btree::lookup(char *key) {
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
