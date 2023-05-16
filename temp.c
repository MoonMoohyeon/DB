#include <cstring>
#include <algorithm>

#define RECORD_SIZE_LENGTH 2
#define KEY_LENGTH PAGE_SIZE - sizeof(slot_header) - sizeof(page*)
#define VALUE_LENGTH 8

uint64_t page::find(char *key) {
    // Get the number of records in the page
    uint32_t num_records = hdr.get_num_data();

    // Get the offset array
    uint16_t *offset_array = (uint16_t*)hdr.get_offset_array();

    // Loop through the records to find the key
    for (uint32_t i = 0; i < num_records; i++) {
        // Calculate the offset of the record
        uint16_t offset = offset_array[i];

        // Calculate the size of the record
        uint16_t record_size = get_record_size((void*)(data + offset));

        // Extract the key from the record
        char *record_key = get_key((void*)(data + offset));

        // Compare the key with the given key
        int cmp = std::memcmp(key, record_key, std::min(strlen(key), strlen(record_key)));

        // If the keys match, return the value
        if (cmp == 0) {
            return get_val((void*)(data + offset));
        }
    }

    // If the key is not found, return 0
    return 0;
}

bool page::insert(char *key, uint64_t value) {
    // Get the number of records in the page
    uint32_t num_records = hdr.get_num_data();

    // Get the offset array
    uint16_t *offset_array = (uint16_t*)hdr.get_offset_array();

    // Calculate the size of the new record
    uint16_t new_record_size = RECORD_SIZE_LENGTH + strlen(key) + 1 + VALUE_LENGTH;

    // Calculate the free space available in the page
    uint16_t free_space = hdr.get_data_region_off() - (num_records * sizeof(uint16_t)) - sizeof(uint16_t) - new_record_size;

    // If there is not enough free space, return false
    if (free_space < 0) {
        return false;
    }

    // Add the offset of the new record to the offset array
    offset_array[num_records] = hdr.get_data_region_off() - new_record_size;

    // Copy the new record to the page
    char *new_record = data + offset_array[num_records];
    std::memcpy(new_record, &new_record_size, RECORD_SIZE_LENGTH);
    std::memcpy(new_record + RECORD_SIZE_LENGTH, key, strlen(key) + 1);
    std::memcpy(new_record + RECORD_SIZE_LENGTH + strlen(key) + 1, &value, VALUE_LENGTH);

    // Sort the offset array by the keys
    std::sort(offset_array, offset_array + num_records + 1, [&](uint16_t a, uint16_t b) {
        char *key_a = get_key((void*)(data + a));
        char *key_b = get_key((void*)(data + b));
        return std::strcmp(key_a, key_b) < 0;
    });

    // Update the number of records and the end of the free space block in the header
    hdr.set_num_data(num_records + 1);
    hdr.set_data_region_off(offset_array[num_records] - sizeof(uint16_t));

    return true;
}

bool page::is_full(uint64_t value) {
    // Get the number of records in the page
    uint32_t num_records = hdr.get_num_data();

    // Get the offset array
    uint16_t *offset_array = (uint16_t*)hdr.get_offset_array();

    // Calculate the size of the new record
    uint16_t new_record_size = RECORD_SIZE_LENGTH + KEY_LENGTH + VALUE_LENGTH;

    // Calculate the free space available in the page
    uint16_t free_space = hdr.get_data_region_off() - (num_records * sizeof(uint16_t)) - sizeof(uint16_t) - new_record_size;

    // If there is not enough free space, return true
    if (free_space < 0) {
        return true;
    }

    return false;
}
