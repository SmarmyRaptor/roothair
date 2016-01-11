#include "TIFFRead.h"
#include <fstream>
#include <string>
#include <cerrno>
#include <map>
#include <math.h>
//Does not support Tiff files written in  big-endian
//Does not currently support RATIONAL, SRATIONAL, FLOAT, or DOUBLE data types. Assumes Data small enough will be placed in the tag
std::map<unsigned short, unsigned short> TiffTag::data_sizes = {
	{1, 1},
	{2, 1},
	{6, 1},
	{7, 1},
	{3, 2},
	{7, 2},
	{4, 4},
	{9, 4},
};
template <typename T> TiffTag<T>::TiffTag(unsigned char data[], unsigned long loc){
	unsigned long short_loc = loc;
	this->tag_id = (data[short_loc+1]<<8) | (data[short_loc]);
	short_loc += 2;
	this->data_type = (data[short_loc+1]<<8) | (data[short_loc]);
	short_loc += 2;
	this->data_number = ((data[short_loc+3]<<24) | (data[short_loc+2]<<16)| (data[short_loc+1]<<8)| (data[short_loc]));
	short_loc += 4;
	this->data = new T[this->data_number];
	//check if data is located in the tag or offset
	//located in tag
	//TODO: clean this up
	if(((this->data_type == 1 || this->data_type == 2 || this->data_type == 6 || this->data_type == 7)&&this->data_number < 5)||((this->data_type == 3 ||this->data_type == 7) && this->data_number<3) ||((this->data_type == 4 ||this->data_type == 9) && this->data_number ==1) ||this->data_number == 0){
		this->data_in_tag = true;
		for (unsigned long i =0; i < this->data_number; ++i){
			if(this->data_type == 4 ||this->data_type == 9){
				this->data[i] = ((data[short_loc+3]<<24) | (data[short_loc+2]<<16)| (data[short_loc+1]<<8)| (data[short_loc]));
				short_loc += 4;
			}else if(this->data_type == 3 ||this->data_type == 7){
				this->data[i] = (data[short_loc+1]<<8) | (data[short_loc]);
				short_loc += 2;
			}else{
				this->data[i] = data[short_loc++];
			}
		}
		this->data_offset = 0;
	}
	//located in offset
	else{
		this->data_in_tag = false;
		this->data_offset = ((data[short_loc+3]<<24) | (data[short_loc+2]<<16)| (data[short_loc+1]<<8)| (data[short_loc]));
		short_loc += 4;
		short_loc = this->data_offset;
		this->data = new T[data_number];
		for (unsigned long i =0; i < this->data_number; ++i){
			if(this->data_type == 4 ||this->data_type == 9){
				this->data[i] = ((data[short_loc+3]<<24) | (data[short_loc+2]<<16)| (data[short_loc+1]<<8)| (data[short_loc]));
				short_loc += 4;
			}else if(this->data_type == 3 ||this->data_type == 7){
				this->data[i] = (data[short_loc+1]<<8) | (data[short_loc]);
				short_loc += 2;				
			}else{
				this->data[i] = data[short_loc++];
			}
		}
	}
}
unsigned long Template <typename T> TiffTag<T>::getDataLength(){
	if (data_in_tag){
		return 0
	}else{
		return data_sizes[data_type] * data_number;
	}
}
unsigned long Template <typename T> TiffTag<T>::getSize(){
	return 12+getDataLength();
}
//print all tag data out into the array print to. returns the offset of the 
unsigned long Template <typename T> TiffTag<T>::printTag(unsigned char print_to[], unsigned long offset, unsigned long data_offset){
	 Endians.bigEndianInsert(print_to, Endians.convertToBigEndian(tag_id), offset);
	 offset+=2;
	 Endians.bigEndianInsert(print_to, Endians.convertToBigEndian(data_type), offset);
	 offset+=2;
	 Endians.bigEndianInsert(print_to, Endians.convertToBigEndian(data_number), offset);
	 offset+=4;
	 data_offset+=printTagData(print_to, offset, data_offset);
	 return data_offset;
}

unsigned long Template <typename T> TiffTag<T>::printTagData(unsigned char print_to[], unsigned long offset, unsigned long data_offset){
	if (data_in_tag){
		for(unsigned long i = 0; i<data_number; i++){
			Endians.bigEndianInsert(print_to, Endians.convertToBigEndian(data[i]), data_offset);
			data_offset +=data_sizes[data_type];
		}
		return 0
	}else{
		for(unsigned long i = 0; i<data_number; i++){
			Endians.bigEndianInsert(print_to, Endians.convertToBigEndian(data[i]), offset);
			offset +=data_sizes[data_type];
		}
		return data_sizes[data_type] * data_number;
	}
}
template <typename T> TiffTag<T>::~TiffTag(){
	delete[] this->data;
}


std::map<std::string, unsigned short> TiffDirectory::tag_names = {
	{"subfile_type", 254},
	{"image_width", 256},
	{"image_length", 257},
	{"photometric_interpretation", 262},
	{"bits_per_sample", 258},
	{"compression", 259},
	{"strip_offsets", 273},//how do I do this?!
	{"samples_per_pixel", 277},
	{"rows_per_strip", 278},
	{"strip_byte_counts", 279},
	{"x_resolution", 282},//not actually used, but support is required for tiff programs 
	{"y_resolution", 283},//not actually used, but support is required for tiff programs 
	{"resolution_unit", 296},//not actually used, but support is required for tiff programs. Defaults to 1
	{"planar_configuration", 284},
};
TiffDirectory::TiffDirectory(unsigned char data[], unsigned long offset){
	unsigned long short_loc = offset;
	initial_offset = offset;
	this->num_entries = (data[short_loc+1]<<8) | (data[short_loc]);
	short_loc += 2;
	this->tag_list = new Tag* [this->num_entries];
	for(unsigned int short i = 0; i< this->num_entries; i++){
		switch((data[short_loc+1]<<8) | (data[short_loc])){
			case 1:
			case 2:
			case 6:
			case 7:
				this->tag_list[i] = new TiffTag<unsigned char>(data, short_loc);
				break;
			case 3:
				this->tag_list[i] = new TiffTag<unsigned short>(data, short_loc);
				break;
			case 4:
				this->tag_list[i] = new TiffTag<unsigned long>(data, short_loc);
				break;
			case 8:
				this->tag_list[i] = new TiffTag<short>(data, short_loc);
				break;
			case 9:
				this->tag_list[i] = new TiffTag<long>(data, short_loc);
				break;
		}
		short_loc += 12;
	}
	this->next_td_offset = ((data[short_loc+3]<<24) | (data[short_loc+2]<<16)| (data[short_loc+1]<<8)| (data[short_loc]));
	if(this->next_td_offset != 0){
		this->next_dir = new TiffDirectory(data, this->next_td_offset);
	}else
		this->next_dir = NULL;
	
	this->makeTagMap();
}
Tag TiffDirectory::operator [](std::string i)const{
	//if the tag actually exists and is in this directory
	if(this->tag_names.find(i) != this->tag_names.end()){
		if(this->tag_map.find(this->tag_names[i]) != this->tag_map.end()){
			return this->tag_map.at(this->tag_names.at(i));
		}
	}
}
void TiffDirectory::makeTagMap(){
	//TODO:Change this to a iterator. Will be marginally faster
	for(unsigned int i = 0; i<this->num_entries; i++){
		this->tag_map.insert( std::pair<unsigned short, Tag>(tag_list[i]->tag_id, *tag_list[i]));
	}
}
Tag* TiffDirectory::findDir(unsigned short tag_id){
	for(unsigned int i = 0; i<this->num_entries; i++){
		if(this->tag_list[i]->tag_id = tag_id){
			return this->tag_list[i];
		}
	}
	return NULL;
}
unsigned long TiffDirectory::getSize(){
	unsigned long ret = 6; //#2 bytes for num_entries, 4 for next_offset(will always be 0x00000000 for this)
	for(unsigned short i = 0; i<num_entries; i++){
		ret +=12;
		ret += tags[i]->getSize();
	}
	return ret;
}
unsigned long TiffDirectory::printDir(unsigned char print_to[], unsigned long offset){
	unsigned long original_offset = offset;
	//print out directory header
	Endians.bigEndianInsert(print_to, Endians.convertToBigEndian(num_entries))
	offset+=2;
	//only doing one image at time so we won't have any next directory. Each of these characters will always be 0
	for(unsigned short i = 0; i<4; i++)
		print_to[offset++] = 0x00;
	//probably need to fix this later. There are going to be more that are dependant on the write out for non-standard directories;
	unsigned long data_offset = data_offset+ num_entries*12;
	for(unsigned short i = 0; i<num_entries; i++){
		//special case for strip offsets because we don't know that data until we are done
		if(tag_list[i]->tag_id == 273){
			//get size, add to start of directory
			unsigned long strip_start = original_offset + getSize();
			//make an array of cumalative strip sizes
			data_offset = printStripLocTagTag(print_to, offset, data_offset, i, strip start)
			//fill data offset with the strips
			//add total size to data offset
		}else{
			data_offset = tag_list.printTag(print_to, offset, data_offset);
			
		}
		offset += 12;
	}
	return offset;
}
//determine size of tiffDir and place the first one after that, using the strip size to change where it goes.
unsigned long TiffDirectory::printStripLocTag(unsigned char print_to[], unsigned long offset, unsigned long data_offset, unsigned long strip_start){
	Endians.bigEndianInsert(print_to, Endians.convertToBigEndian(tag_id), offset);
	offset+=2;
	Endians.bigEndianInsert(print_to, Endians.convertToBigEndian(data_type), offset);
	offset+=2;
	Endians.bigEndianInsert(print_to, Endians.convertToBigEndian(data_number), offset);
	offset+=4;
	TiffTag strip_size = tag_map.at(279);
	unsigned long strip_loc = strip_start;
	for(unsigned int i = 0; i<sizeof(strip_size->data)) i++){
		Endians.bigEndianInsert(print_to, Endians.convertToBigEndian(strip_loc), data_offset)
		data_offset+=4;
		strip_loc += strip_size->data[i];
	}
	return data_offset;
}
TiffDirectory::~TiffDirectory(){
	delete[] this->tag_list;
	delete this->next_dir;
}

TiffHead::TiffHead(unsigned char data[]){
	this->identifier = (data[1]<<8) | (data[0]);
	this->version = (data[3]<<8) | (data[2]);
	this->td_offset = (data[7]<<24) | data[6]<<16| data[5]<<8) | data[4];
}
unsigned long TiffHead::getSize(){
	return 8;
}
unsigned long TiffHead::printHead(unsigned char print_to[]){
	Endians.bigEndianInsert(print_to, Endians.convertToBigEndian(identifier), 0)
	Endians.bigEndianInsert(print_to, Endians.convertToBigEndian(version), 2)
	Endians.bigEndianInsert(print_to, Endians.convertToBigEndian(0x00000008), 4)
	return 8;
}

TiffFile::TiffFile(std::string file_name){
	//read data
	this->SetData(file_name);
	//load header
	this->head = new TiffHead(this->data);
	//load each of the directories
	this->directories = new TiffDirectory(data, this->head->td_offset);
	//load images, name tags
	this->num_images = this->RecImageCount(this->directories, 0);
	this->images = new TiffImage* [this->num_images];
	this->RecSetImages(this->directories, 0);
	
}
//filename must be under
bool TiffFile::SetData(std::string file_name){
	//this way is slightly faster for large files than using traditional stream methods.
	std::FILE *f = std::fopen(file_name.c_str(), "rb");
	if(f){
		std::fseek(f, 0, SEEK_END);
		//find end of stream
		long content_size = std::ftell(f);
		//rewind and copy to this.data
		std::rewind(f);
		this->data = new unsigned char[content_size];
		std::fread(this->data, 1, content_size, f);
		std::fclose(f);
		return true;
	}else
		throw(errno);
}
unsigned short TiffFile::RecImageCount(TiffDirectory *dir, int count){
	count += 1;
	if(dir->next_dir == nullptr){
		return this->RecImageCount(dir->next_dir, count);
	}
	return count;
}
void TiffFile::RecSetImages(TiffDirectory *dir, int imageNumber){
	this->images[imageNumber] = new TiffImage(this->data, dir);
	if(imageNumber < this->num_images){
		RecSetImages(this->images[imageNumber]->dir->next_dir, imageNumber+1);
	}
}
TiffFile::~TiffFile(){
	delete[] this->data;
	delete[] this->directories;
	delete[] this->images;
}

TiffImage::TiffImage(unsigned char data[], TiffDirectory dir){
	this->dir = dir;
	this->data = data;
	setStrips();
}
TiffImage::TiffImage(TiffImage copyImage){
	unsigned long old_data_length = sizeof(copyImage->data);
	data = new unsigned char[old_data_length];
	for(var i = 0; i<old_data_length){
		data[i] = copyImage->data[i];
	}
	dir = new TiffDirectory(data, copyImage.dir.initial_offset);
}
void TiffImage::setStrips(){
	num_strips = floor((dir["image_length"].data[0] * (dir["rows_per_strip"].data[0] - 1)) / dir["rows_per_strip"].data[0]);
	strips = new TiffStrip[num_strips];
	for (unsigned short i = 0; i<num_strips; i++){
		//TODO make this work for everything not PlanarConfiguration = 1, and RBG
		strips[i] = new Tiffstrip(dir, data, i);
	}
}
unsigned long TiffImage::getSize(){
	ret = 8;
	ret += dir.getSize();
	for(unsigned short i =0; i<num_strips; i++){
		ret += strips[i].getSize();
	}
	return ret
}
unsigned char* TiffImage::printImage(TiffHead head){
	unsigned long size = getSize();
	unsigned long offset = 0;
	unsigned char *ret = new unsigned char[size];
	head.printHead(ret);
	offset = dir.printDir(ret, 8);
	for(unsigned short i = 0; i<num_strips; i++){
		offset = strips[i].printStrip(ret, offset);
	}
	return ret;
}
TiffImage::~TiffImage(){
	delete[] strips;
}

unsigned long TiffStrip::getSize(){
	return length;
}
//TODO:Need to actually implemnet this
virtual void TiffStrip::printStrip(unsigned char print_to[], unsigned long offset){
	return;
}

RGBTiffStrip::RGBTiffStrip(TiffDirectory dir, unsigned char data[], unsigned long strip_num){
	//get length of strip
	setLength(dir, strip_num);
	setOffset(dir, strip_num);
	setStripData(data);
}
void RGBTiffStrip::setLength(TiffDirectory dir,  unsigned long strip_num){
	length = dir["strip_byte_counts"].data[strip_num];
	strip_data = new Pixel[length/3];
}
void RGBTiffStrip::setOffset(TiffDirectory dir,  unsigned long strip_num){
		offset = dir["strip_offsets"].data[strip_num];
}
void RGBTiffStrip::setStripData(unsigned char data[]){
	for(unsigned long i = 0; i<length; i+=3){
		strip_data[i/3] = new RGBPixel(data[offset+i], data[offset+i+1], data[offset+i+2]);
	}
}
unsigned long RGBTiffStrip::printStrip(unsigned char print_to[], unsigned long offset){
	for(unsigned long i = 0;i<length/3; i++){
		bigEndianInsert(print_to, strip_data[i].printPixel(), offset+(i*3))
	}
	return offset+length;
}
unsigned long RGBTiffStrip::getLength()const{
	return length;
}
Pixel RGBTiffStrip::operator [](unsigned short i) const{
	return strip_data[i];
}
RGBTiffStrip::~RGBTiffStrip(){
	delete[] strip_data;
}
