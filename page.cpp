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

bool page::insert(char *key,uint64_t val){
	// Please implement this function in project 1.
	uint32_t num_data = hdr.get_num_data();
	uint16_t* offset_array = (uint16_t*)hdr.get_offset_array();
	uint16_t record_size = sizeof(uint16_t) + strlen(key) + 1 + sizeof(uint64_t);

	if (is_full(record_size)) {
		return false; // Page is full
	}

	int insert_pos = 0;

	for (; insert_pos < num_data; insert_pos++)
	{
		uint16_t off = offset_array[insert_pos];
		void* record_ptr = (void*)((char*)this + (uint64_t)off);
		char* stored_key = get_key(record_ptr);

		if (strcmp(key, stored_key) < 0)
		{
			break;
		}
	}

	for (int i = num_data - 1; i >= insert_pos; i--)
	{
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

	return true;
}

page* page::split(char *key, uint64_t val, char** parent_key){
	// Please implement this function in project 2.
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

bool page::is_full(uint64_t inserted_record_size){
	// Please implement this function in project 1.
	uint32_t num_data = hdr.get_num_data();
	uint16_t data_region_off = hdr.get_data_region_off();
	uint16_t used_size;

	// Calculate the total space used by the existing records
	used_size = sizeof(hdr);
	used_size += sizeof(uint16_t) * (num_data + 1);
	used_size += inserted_record_size;
	
	return (used_size > data_region_off);
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
		printf("| data_sz:%u | key: %s | val :%lu | key_sz : %d \n",record_size,(char*)stored_key, stored_val,strlen((char*)stored_key));
	}
}
