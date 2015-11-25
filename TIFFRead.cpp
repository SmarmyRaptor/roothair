#include "TIFFRead.h"
#include <fstream>
#include <string>
#include <cerrno>
#include <map>
//Does not support Tiff files written in  big-endian
//Does not currently support RATIONAL, SRATIONAL, FLOAT, or DOUBLE data types. Assumes Data small enough will be placed in the tag

template <typename T> TiffTag::TiffTag(char data[], unsigned long loc){
	unsigned long short_loc = loc;
	this.tag_id = (data[short_loc+1]<<8) | (data[short_loc])];
	short_loc += 2;
	this.data_type = (data[short_loc+1]<<8) | (data[short_loc])];
	short_loc += 2;
	this.data_number = ((data[short_loc+3]<<24) | (data[short_loc+2]<<16)| (data[short_loc+1]<<8)| (data[short_loc]));
	short_loc += 4;
	this.data = new T[this.data_number];
	//check if data is located in the tag or offset
	//located in tag
	if(((this.data_type == 1 || this.data_type == 2 || this.data_type == 6 || this.data_type == 7)&&this.data_number < 5)||((this.data_type == 3 ||this.data_type == 7) && this.data_number<3) ||this.data_number == 0)||((this.data_type == 4 ||this.data_type == 9) && this.data_number ==1) ||this.data_number == 0){
		this.data_in_tag = true;
		for (unsigned long i =0; i < this.data_number; ++i){
			if(this.data_type == 4 ||this.data_type == 9){
				this.data[i] = ((data[short_loc+3]<<24) | (data[short_loc+2]<<16)| (data[short_loc+1]<<8)| (data[short_loc]));
				short_loc += 4
			}else if(this.data_type == 3 ||this.data_type == 7){
				this.data[i] = (data[short_loc+1]<<8) | (data[short_loc])];
				short_loc += 2;
			}else{
				this.data[i] = data[short_loc++];
			}
		}
		this.data_offset = NULL;
	}else{
		//located in offset
		this.data_in_tag = false;
		this.data_offset = ((data[short_loc+3]<<24) | (data[short_loc+2]<<16)| (data[short_loc+1]<<8)| (data[short_loc]));
		short_loc += 4
		short_loc = this.data_offset;
		this.data = new T[data_number];
		for (unsigned long i =0; i < this.data_number; ++i){
			if(this.data_type == 4 ||this.data_type == 9){
				this.data[i] = ((data[short_loc+3]<<24) | (data[short_loc+2]<<16)| (data[short_loc+1]<<8)| (data[short_loc]));
				short_loc += 4
			}else if(this.data_type == 3 ||this.data_type == 7){
				this.data[i] = (data[short_loc+1]<<8) | (data[short_loc])];
				short_loc += 2;				
			}else{
				this.data[i] = data[short_loc++];
			}
		}
	}
}
template <typename T> TiffTag::~TiffTag(){
	delete[] this.data;
}

TiffDirectory::TiffDirectory(char data[], unsigned long offset){
	unsigned long short_loc = offset;
	this.num_entries = (data[short_loc+1]<<8) | (data[short_loc])];
	short_loc += 2
	this.tag_list = new TiffTag[this.num_entries]
	for(unsigned int short i = 0; i< this.num_entries; i++){
		switch((data[short_loc+1]<<8) | (data[short_loc]))
			case 1:
			case 2:
			case 6:
			case 7:
				this.tag_list[i] = TiffTag<char>(data, short_loc);
				break;
			case 3:
				this.tag_list[i] = TiffTag<unsigned short>(data, short_loc);
				break;
			case 4:
				this.tag_list[i] = TiffTag<unsigned long>(data, short_loc);
				break;
			case 8:
				this.tag_list[i] = TiffTag<short>(data, short_loc);
				break;
			case 9:
				this.tag_list[i] = TiffTag<long>(data, short_loc);
				break;
		}
		short_loc += 12
	}
	this.next_td_offset = ((data[short_loc+3]<<24) | (data[short_loc+2]<<16)| (data[short_loc+1]<<8)| (data[short_loc]));
	if(this.next_td_offset != 0){
		this.next_dir = new TiffDirectory(data, this.next_td_offset);
	}else
		this.next_tdir = NULL;
}
TiffDirectory::~TiffDirectory(){
	delete[] this.tag_list;
	delete this.next_dir;
}

TiffHead::TiffHead(char data[]){
	this.identifier = (data[1]<<16) | (data[0]);
	this.version = (data[2]<<16) | (data[3]);
	this.td_offset = ((data[2]<<16) | (data[3]));
}

TiffFile::TiffFile(std::string file_name){
	//read data
	this.SetData(file_name);
	//load header
	this.head = new TiffHead(this.data);
	//load each of the directories
	this.directories = new TiffDirectory(data, this.head.td_offset);
	//load images, name tags and shit
	this.numImages = this.RecImageCount(this.directories, 0);
	this.images = new TiffImage[this.numImages];
	this.RecSetImages(this.directories, 0);
	
}
void TiffFile::SetData(std::string file_name){
	//this way is slightly faster for large files than using streams . 
	std::FILE *f = std:::fopen(filename, "rb");
	if(f){
		std::fseek(f, 0, SEEK_END);
		//find end of stream
		long content_size = std::ftell(f);
		//rewind and copy to this.data
		std::rewind(f);
		this.data = new char[content_size];
		std::fread(this.data, 1, content_size, f);
		std::fclose(f);
		return true;
	}else
		throw(errno);
}
int TiffFile::RecImageCount(TiffDirectory dir, int count){
	count ++ 1;
	if(dir.next_tiff == nullptr){
		return this.RecImageCount(dir.next_tiff, count);
	}
	return count;
}
void RecSetImages(TiffDirectory dir, int imageNumber){
	this.images[imageNumber] = new TiffImage(this.data, this.dir);
	imageNumber++;
	if(imageNumber < this.numImages){
		RecSetImages(dir.next_tiff, imageNumber);
	}
}
TiffFile::~TiffFile(){
	delete[] this.data;
	delete[] this.directories;
	delete[] this.images;
}
TiffImage::TiffImage(char data[], TiffDirectory dir){
	this.directory = dir;
	
}
