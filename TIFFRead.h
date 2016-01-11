#ifndef TIFF_READER_INCLUDED
#define TIFF_READER_INCLUDED

#include <string>
#include <set>
#include <fstream>
#include <cerrno>
#include <map>
#include "ImagePixel.h"
#include "Endians.h"

class TiffHead
{
	public:	
		unsigned short  identifier;  //determines if it is little or big endian
		unsigned short  version; //should always be 42
		unsigned long td_offset; //offset to first TiffDirectory
		
		TiffHead(unsigned char data[]);
		
		unsigned long getSize();//returns number of bytes the head takes up in the file
		unsigned long printHead(unsigned char print_to[])//prints the data in the head. Returns the location of the last
		
};
struct Tag{
	unsigned short tag_id;
};
template <typename T>
class TiffTag: public Tag
{
	private:
		static std::map<unsigned short, unsigned short> data_sizes; //maps the data type to the number of bytes it takes to hold it.
		
		unsigned long getDataLength();//returns 0 if data is located in the tag, else returns the amount of  bytes it takes to hold the data info in a file
		unsigned long printTagData(unsigned char print_to[], unsigned long offset, unsigned long data_offset);//returns the length of the data;
		
	public: 
		unsigned short   data_type;   //what type of data the tag holds
		unsigned long   data_number;  //number of items of data_type the tag holds
		unsigned long   data_offset;  //offset to where the data is held
		bool data_in_tag; 			  // true if data is located in tag, false if offset
		T *data;
		
		TiffTag(unsigned char data[], unsigned long loc);
		
		unsigned long getSize();//returns the total amount of  bytes it takes to hold the tag info in a file
		
		unsigned long printTag(unsigned char print_to[], unsigned long offset, unsigned long data_offset); //places the tag in print_to[] at offset, and the data that cannot fit inside the tags at data_offset. returns the offset of the end of the data_offset. 
		
		~TiffTag();
};

class TiffDirectory
{
	private:
		static std::map<std::string, unsigned short> tag_names; //changes names to 
		std::map<unsigned short, Tag> tag_map; //accesses the tag list via []
		
		
		void makeTagMap();//runs on init to make tag_map
		unsigned long printStripLocTag(unsigned char print_to[], unsigned long offset, unsigned long data_offset); //adds the strip location tag to the print_to array returns offset of the next empty place after the data_offset in the array
		
	public:
		unsigned long initial_offset;
		unsigned short   num_entries;   //number of tags
		unsigned long   next_td_offset; //offset to next TiffDirectory
		TiffDirectory* next_dir; //pointer to the next TiffDirectory;
		Tag**  tag_list;   			//array of tags
		
		TiffDirectory(unsigned char data[], unsigned long offset);
		Tag operator [](std::string i) const; //returns the tag associated with the given name
		
		Tag* findDir(unsigned short tag_id);//Given a number, returns the TiffTag with the given tag id
		unsigned long getSize();//returns the amount of  bytes it takes to hold the directory info in a file
		unsigned long  printDir(unsigned char print_to[], unsigned long offset);//places all the tags into print_to[] at offset. Returns the offset of the next empty spot
		unsigned long printStripLocTag(unsigned char print_to[], unsigned long offset, unsigned long data_offset, unsigned long strip_start)//places the strip locatoin data into the char array print_to
		~TiffDirectory();
};

class TiffStrip{
	private:
		unsigned long length;
		unsigned long offset;
	public:
		Pixel* strip_data;
		
		unsigned long getSize();
		virtual void printStrip(unsigned char print_to[], unsigned long offset);
		
};
class RGBTiffStrip: public TiffStrip {
		void setLength(TiffDirectory dir,  unsigned long strip_num);
		void setOffset(TiffDirectory dir,  unsigned long strip_num);
		void setStripData(unsigned char data[]);
	public:
		TiffStrip(TiffDirectory dir, unsigned char data[],  unsigned long strip_num);
		
		unsigned long getLength() const;
		unsigned long printStrip(unsigned char print_to[], unsigned long offset);
		Pixel operator [](unsigned short i) const;
		
		~TiffStrip();
};


//Always assumes no compression. currently only works on 0, 1, 2 color spaces.
class TiffImage{
	private:
		/*unsigned long subfile_type; 				//254
		unsigned long image_width; 					//256
		unsigned long image_length;					//257
		unsigned short photometric_interpretation;	//262
		unsigned short bits_per_sample;				//258
		unsigned short compression;					//259
		unsigned short num_strips; 					//not a default tag value. Used to make some things easier later.
		unsigned long *strip_offsets;				//273
		unsigned short samples_per_pixel;			//277
		unsigned long rows_per_strip;				//278
		unsigned long strip_byte_counts;			//279
		unsigned long int x_resolution				//282 not actually used, but support is required for tiff programs 
		unsigned long int y_resolution				//283 not actually used, but support is required for tiff programs 
		unsigned short resulution_unit				//296 not actually used, but support is required for tiff programs 
		unsigned short planar_configuration		//284
	*/
		void setStrips();
	public:
		
		TiffDirectory dir;
		unsigned char* data;
		unsigned short num_strips;
		TiffStrip* strips;
		
		TiffImage(TiffImage copyImage);//copy data, make new dir with data, 
		TiffImage(unsigned char data[], TiffDirectory dir);
		unsigned char* printImage();
		unsigned long getSize();//returns the amount of  bytes it takes to hold the TiffImage in a file
		
		~TiffImage();
};

class TiffFile{
	private:
		TiffHead *head; 				//Head of the tiff file
		unsigned short num_images;				//images within the Tiff file
		TiffDirectory *directories; //Array of the Directories for each of the images
		
		bool SetData(std::string file_name);//reads the data from the file
		unsigned short RecImageCount(TiffDirectory *dir, int count); //counts the number of directories in the Tiff, including and after the one provided. 
		void RecSetImages(TiffDirectory *dir, int imageNumber);//reads the image data for each of the directories into the image  array
	public:
		TiffImage** images;	// Array of the image objects in the same order as the directory array
		unsigned char* data;
		
		TiffFile(std::string file_name);
		~TiffFile();
};

#endif