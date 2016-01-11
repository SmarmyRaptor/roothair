#include "Endians.h"
//TODO: catch if asking to put it outside the scope
unsigned long Endians::bigEndianInsert(unsigned char data[], unsigned char to_add[], unsigned long insert_loc){
	unsigned int size = sizeof(to_add);
	for(unsigned int i = 0; i< size; i++){
		data[insert_loc+i] = to_add[i];
	}
	return insert_loc + size;
}
unsigned long Endians::bigEndianInsert(unsigned char data[], unsigned char to_add, unsigned long insertLoc){
	data[insert_loc] = to_add;
	return insert_loc + 1
}
char* Endians::convertToBigEndian(unsigned long convertee){
	unsigned char ret[4];
	ret[0] = convertee&0xFF;
	ret[1] = (convertee&0xFF00)>>8;
	ret[2] = (convertee&0xFF0000)>>16;
	ret[3] = (convertee&0xFF000000)>>24;
	return ret;
}
char* Endians::convertToBigEndian(unsigned short convertee){
	unsigned char ret[2];
	ret[0] = convertee&0xFF;
	ret[1] = (convertee&0xFF00)>>8;
	return ret;
}
char* Endians::convertToBigEndian(unsigned char convertee){
	unsigned char ret[1];
	return ret;
}
char* Endians::convertToBigEndian(short convertee){
	unsigned char ret[2];
	ret[0] = convertee&0xFF;
	ret[1] = (convertee&0xFF00)>>8;
	return ret;
}
char* Endians::convertToBigEndian(long convertee){
	unsigned char ret[4];
	ret[0] = convertee&0xFF;
	ret[1] = (convertee&0xFF00)>>8;
	ret[2] = (convertee&0xFF0000)>>16;
	ret[3] = (convertee&0xFF000000)>>24;
	return ret;
}