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
	void* offset_array = hdr.get_offset_array();

	for (int i = 0; i < num_data; i++) {
		uint16_t off = *(uint16_t*)((uint64_t)offset_array + i * 2);
		void* data_region = (void*)((uint64_t)this + (uint64_t)off);
		char* stored_key = get_key(data_region);
		uint64_t stored_val = get_val((void*)stored_key);

		if (strcmp(stored_key, key) == 0) {
			return stored_val;
		}
	}

	return 0; // Key not found
}

bool page::insert(char *key,uint64_t val){
	// Please implement this function in project 1.
	uint32_t num_data = hdr.get_num_data();
	void* offset_array = hdr.get_offset_array();
	uint16_t record_size = sizeof(uint16_t) + strlen(key) + 1 + sizeof(uint64_t);

	if (is_full(record_size)) {
		return false; // Page is full
	}

	// Find the correct position to insert the new record
	int insert_pos = 0;
	for (; insert_pos < num_data; insert_pos++) {
		uint16_t off = *(uint16_t*)((uint64_t)offset_array + insert_pos * 2);
		void* data_region = (void*)((uint64_t)this + (uint64_t)off);
		char* stored_key = get_key(data_region);

		if (strcmp(stored_key, key) > 0) {
			break;
		}
	}

	// Shift the offset array and make space for the new record
	for (int i = num_data - 1; i >= insert_pos; i--) {
		uint16_t off = *(uint16_t*)((uint64_t)offset_array + i * 2);
		*(uint16_t*)((uint64_t)offset_array + (i + 1) * 2) = off + record_size;
	}

	// Update the offset array and copy the new record
	*(uint16_t*)((uint64_t)offset_array + insert_pos * 2) = hdr.get_data_region_off() - record_size;
	void* new_record = (void*)((uint64_t)this + (uint64_t)hdr.get_data_region_off() - record_size);
	put2byte(new_record, record_size);
	strcpy((char*)((uint64_t)new_record + sizeof(uint16_t)), key);
	*(uint64_t*)((uint64_t)new_record + sizeof(uint16_t) + strlen(key) + 1) = val;

	// Update the header
	hdr.set_num_data(num_data + 1);
	hdr.set_data_region_off(hdr.get_data_region_off() - record_size);

}

page* page::split(char *key, uint64_t val, char** parent_key){
	// Please implement this function in project 2.
	page *new_page;
	return new_page;
}

bool page::is_full(uint64_t inserted_record_size){
	// Please implement this function in project 1.
	uint32_t num_data = hdr.get_num_data();
	void* offset_array = hdr.get_offset_array();
	uint16_t total_used_space = hdr.get_data_region_off();

	// Calculate the total space used by the existing records
	for (int i = 0; i < num_data; i++) {
		uint16_t off = *(uint16_t*)((uint64_t)offset_array + i * 2);
		void* data_region = (void*)((uint64_t)this + (uint64_t)off);
		uint16_t record_size = get_record_size(data_region);
		total_used_space -= record_size;
	}

	// Check if there is enough space for the new record
	return (total_used_space < (inserted_record_size + sizeof(uint16_t)));
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




