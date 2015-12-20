#include "TIFFRead.h"
#include <fstream>
#include <string>
#include <cerrno>
#include <map>
#include <math.h>
//Does not support Tiff files written in  big-endian
//Does not currently support RATIONAL, SRATIONAL, FLOAT, or DOUBLE data types. Assumes Data small enough will be placed in the tag
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
	}else{
		//located in offset
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
	{"strip_offsets", 273},
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
TiffDirectory::~TiffDirectory(){
	delete[] this->tag_list;
	delete this->next_dir;
}

TiffHead::TiffHead(unsigned char data[]){
	this->identifier = (data[1]<<16) | (data[0]);
	this->version = (data[2]<<16) | (data[3]);
	this->td_offset = ((data[2]<<16) | (data[3]));
}

TiffFile::TiffFile(std::string file_name){
	//read data
	this->SetData(file_name);
	//load header
	this->head = new TiffHead(this->data);
	//load each of the directories
	this->directories = new TiffDirectory(data, this->head->td_offset);
	//load images, name tags and shit
	this->num_images = this->RecImageCount(this->directories, 0);
	this->images = new TiffImage* [this->num_images];
	this->RecSetImages(this->directories, 0);
	
}
//filename must be under
bool TiffFile::SetData(std::string file_name){
	//this way is slightly faster for large files than using streams.
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
int TiffFile::RecImageCount(TiffDirectory *dir, int count){
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
void TiffImage::setStrips(){
	unsigned short num_strips = floor((dir["image_length"].data[0] * (dir["rows_per_strip"].data[0] - 1)) / dir["rows_per_strip"].data[0]);
	strips = new TiffStrip[num_strips];
	for (unsigned short i = 0; i<num_strips; i++){
		//TODO make this work for everything not PlanarConfiguration = 1, and RBG
		strips[i] = new Tiffstrip(dir, data, i);
	}
}
TiffImage::~TiffImage(){
	delete[] strips;
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
unsigned long RGBTiffStrip::getLength()const{
	return length;
}
Pixel RGBTiffStrip::operator [](unsigned short i) const{
	return strip_data[i];
}
RGBTiffStrip::~RGBTiffStrip(){
	delete[] strip_data;
}
