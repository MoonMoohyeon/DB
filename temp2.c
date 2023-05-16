uint64_t page::find(char *key) {
    uint16_t num_records = hdr.get_num_data();
    void *offset_array = hdr.get_offset_array();
    uint16_t data_region_off = hdr.get_data_region_off();

    for (int i = 0; i < num_records; i++) {
        uint16_t offset = *((uint16_t*)offset_array + i);
        void *record_ptr = data + data_region_off + offset;
        char *record_key = get_key(record_ptr);
        if (strcmp(record_key, key) == 0) {
            return get_val(record_ptr);
        }
    }
    return 0;
}

bool page::insert(char *key, uint64_t val) {
    // check if the page is already full
    if (is_full(get_record_size(key) + sizeof(uint64_t))) {
        return false;
    }

    uint16_t key_len = strlen(key) + 1;
    uint16_t record_size = sizeof(uint16_t) + key_len + sizeof(uint64_t);

    // find the correct position to insert the new record
    uint16_t i;
    for (i = 0; i < hdr.get_num_data(); i++) {
        char *cur_key = get_key((char *)hdr.get_offset_array() + i * sizeof(uint16_t));
        if (strcmp(key, cur_key) < 0) {
            break;
        }
    }

    // shift the existing records to make room for the new record
    uint16_t shift_size = hdr.get_data_region_off() - (i * sizeof(uint16_t) + record_size);
    char *src = data + i * sizeof(uint16_t) + record_size;
    char *dst = data + (i + 1) * sizeof(uint16_t) + record_size;
    memmove(dst, src, shift_size);

    // update the offset array and insert the new record
    char *offset_array = (char *)hdr.get_offset_array();
    uint16_t offset = i * sizeof(uint16_t) + hdr.get_data_region_off() - record_size;
    *((uint16_t *)(offset_array + i * sizeof(uint16_t))) = offset;
    *((uint16_t *)(data + offset)) = key_len;
    memcpy(data + offset + sizeof(uint16_t), key, key_len);
    *((uint64_t *)(data + offset + sizeof(uint16_t) + key_len)) = val;

    // update the slot header
    hdr.set_num_data(hdr.get_num_data() + 1);
    hdr.set_data_region_off(offset - sizeof(uint16_t));

    return true;
}

bool page::is_full(uint64_t record_size) {
    // check if there is enough space for the record
    if (hdr.get_data_region_off() - (hdr.get_num_data() + 1) * sizeof(uint16_t) < record_size) {
        return true;
    }
    return false;
}

