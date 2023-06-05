void btree::insert(char *key, uint64_t val) {
    std::stack<page*> stack;
    page* current_page = root;

    while (current_page->get_type() != LEAF) {
        stack.push(current_page);
        current_page = (page*)current_page->find(key);
    }

    // Insert data into the leaf node
    if (!current_page->is_full(key)) {
        current_page->insert(key, val);
    } else {
        char* parent_key = nullptr;
        page* new_page = current_page->split(key, val, &parent_key);
        current_page->insert(key, val);

        // Propagate the split operation until finding a node that is not full
        while (!stack.empty()) {
            current_page = stack.top();
            stack.pop();
            
            if (!current_page->is_full(parent_key)) {
                current_page->insert(parent_key, (uint64_t)new_page);
                break;
            } else {
                char* new_parent_key = nullptr;
                page* new_parent_page = current_page->split(parent_key, (uint64_t)new_page, &new_parent_key);
                new_page = new_parent_page;
                parent_key = new_parent_key;
            }
        }

        // If the root was split, create a new root
        if (stack.empty() && !current_page->is_full(parent_key)) {
            page* old_root = root;
            root = new page(INTERNAL);
            root->set_leftmost_ptr(old_root);
            root->insert(parent_key, (uint64_t)new_page);
            height++;
        }
    }
}

uint64_t btree::lookup(char *key) {
    page* current_page = root;

    while (current_page->get_type() != LEAF) {
        current_page = (page*)current_page->find(key);
    }

    return current_page->find(key);
}
