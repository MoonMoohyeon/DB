#include "page.hpp"
#include <iostream> 
#include <cstring>

#define RECORD_SIZE 2
#define VALUE_SIZE 8

void put2byte(void *dest, uint16_t data){
	*(uint16_t*)dest = data;
}

uint16_t get2byte(void *dest){
	return *(uint16_t*)dest;
}

page::page(uint16_t type){
	hdr.set_num_data(0);
	hdr.set_data_region_off(PAGE_SIZE-1-sizeof(page*));
	hdr.set_offset_array((void*)((uint64_t)this+sizeof(slot_header)));
	hdr.set_page_type(type);
}

uint16_t page::get_type(){
	return hdr.get_page_type();
}

uint16_t page::get_record_size(void *record){
	uint16_t size = *(uint16_t *)record;
	return size;
}

char *page::get_key(void *record){
	char *key = (char *)((uint64_t)record+sizeof(uint16_t));
	return key;
}

uint64_t page::get_val(void *key){
	uint64_t val= *(uint64_t*)((uint64_t)key+(uint64_t)strlen((char*)key)+1);
	return val;
}

void page::set_leftmost_ptr(page *p){
	leftmost_ptr = p;
}

page *page::get_leftmost_ptr(){
	return leftmost_ptr;	
}

uint64_t page::find(char *key){
	// Please implement this function in project 1.
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

bool page::insert(char *key,uint64_t val){
	// Please implement this function in project 1.
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

page* page::split(char *key, uint64_t val, char** parent_key){
	// Please implement this function in project 2.
	page *new_page;
	return new_page;
}

bool page::is_full(uint64_t inserted_record_size){
	// Please implement this function in project 1.
	// Get the number of records in the page
    uint32_t num_records = hdr.get_num_data();

    // Get the offset array
    uint16_t *offset_array = (uint16_t*)hdr.get_offset_array();

    // Calculate the size of the new record
    uint16_t new_record_size = num_records * (*offset_array);

    // Calculate the free space available in the page
    uint16_t free_space = hdr.get_data_region_off() - (num_records * sizeof(uint16_t)) - sizeof(uint16_t) - new_record_size;

    // If there is not enough free space, return true
    if (free_space < 0) {
        return true;
    }

    return false;
}

void page::defrag(){
	page *new_page = new page(get_type());
	int num_data = hdr.get_num_data();
	void *offset_array=hdr.get_offset_array();
	void *stored_key=nullptr;
	uint16_t off=0;
	uint64_t stored_val=0;
	void *data_region=nullptr;

	for(int i=0; i<num_data/2; i++){
		off= *(uint16_t *)((uint64_t)offset_array+i*2);	
		data_region = (void *)((uint64_t)this+(uint64_t)off);
		stored_key = get_key(data_region);
		stored_val= get_val((void *)stored_key);
		new_page->insert((char*)stored_key,stored_val);
	}	
	new_page->set_leftmost_ptr(get_leftmost_ptr());

	memcpy(this, new_page, sizeof(page));
	hdr.set_offset_array((void*)((uint64_t)this+sizeof(slot_header)));
	delete new_page;

}

void page::print(){
	uint32_t num_data = hdr.get_num_data();
	uint16_t off=0;
	uint16_t record_size= 0;
	void *offset_array=hdr.get_offset_array();
	void *stored_key=nullptr;
	uint64_t stored_val=0;

	printf("## slot header\n");
	printf("Number of data :%d\n",num_data);
	printf("offset_array : |");
	for(int i=0; i<num_data; i++){
		off= *(uint16_t *)((uint64_t)offset_array+i*2);	
		printf(" %d |",off);
	}
	printf("\n");

	void *data_region=nullptr;
	for(int i=0; i<num_data; i++){
		off= *(uint16_t *)((uint64_t)offset_array+i*2);	
		data_region = (void *)((uint64_t)this+(uint64_t)off);
		record_size = get_record_size(data_region);
		stored_key = get_key(data_region);
		stored_val= get_val((void *)stored_key);
		printf("==========================================================\n");
		printf("| data_sz:%u | key: %s | val :%lu |\n",record_size,(char*)stored_key, stored_val,strlen((char*)stored_key));

	}
}




