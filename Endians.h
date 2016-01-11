#ifndef ENDIANS
#define ENDIANS

namespace Endians{
	unsigned long bigEndianInsert(unsigned char data[], unsigned char to_add[], unsigned long insert_loc);//inserts an array of characters into the selected location
	unsigned long bigEndianInsert(unsigned char data[], unsigned char to_add, unsigned long insert_loc);//inserts a single character into the selected location
	
	//converts different data types to an array of characters
	char* convertToBigEndian(unsigned long convertee);
	char* convertToBigEndian(unsigned short convertee);
	char* convertToBigEndian(unsigned char convertee);
	char* convertToBigEndian(short convertee);
	char* convertToBigEndian(long convertee);
}
#endif