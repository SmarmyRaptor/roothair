#ifndef TIFF_READER_INCLUDED
#define TIFF_READER_INCLUDED

#include <string>
#include <set>
#include <fstream>
#include <cerrno>
#include <map>
#include "ImagePixel.h"

struct TiffHead
{
	TiffHead(unsigned char data[]);
	unsigned short  identifier;  //determines if it is little or big endian
	unsigned short  version; //should always be 42
	unsigned long td_offset; //offset to first TiffDirectory
};
struct Tag{
	unsigned short tag_id;
};
template <typename T>
class TiffTag: public Tag
{
	public: 
		TiffTag(unsigned char data[], unsigned long loc);
		
		unsigned short   data_type;   //what type of data the tag holds
		unsigned long   data_number;  //number of items of data_type the tag holds
		unsigned long   data_offset;  //offset to where the data is held
		bool data_in_tag; 			  // true if data is located in tag, false if offset
		T *data;
	
		
		~TiffTag();
};

struct TiffDirectory
{
	private:
		static std::map<std::string, unsigned short> tag_names; //changes names to 
		std::map<unsigned short, Tag> tag_map; //accesses the tag list via []
		
		void makeTagMap();//runs on init to make tag_map
	public:
		TiffDirectory(unsigned char data[], unsigned long offset);
	
		unsigned short   num_entries;   //number of tags
		unsigned long   next_td_offset; //offset to next TiffDirectory
		TiffDirectory* next_dir; //pointer to the next TiffDirectory;
		Tag**  tag_list;   			//array of tags
		
		Tag operator [](std::string i) const; //returns the tag associated with the given name
		Tag* findDir(unsigned short tag_id);//Given a number, returns the TiffTag with the given tag id
		
		~TiffDirectory();
};

class TiffStrip{
	private:
		unsigned long length;
		unsigned long offset;
	public:
		Pixel* strip_data;
};
class RGBTiffStrip: public TiffStrip {
		void setLength(TiffDirectory dir,  unsigned long strip_num);
		void setOffset(TiffDirectory dir,  unsigned long strip_num);
		void setStripData(unsigned char data[]);
	public:
		TiffStrip(TiffDirectory dir, unsigned char data[],  unsigned long strip_num);

		unsigned long getLength() const;
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
		TiffImage(unsigned char data[], TiffDirectory dir);
		
		TiffDirectory dir;
		unsigned char* data;
		TiffStrip *strips;
		
		~TiffImage();
};

class TiffFile{
	private:
		TiffHead *head; 				//Head of the tiff file
		int num_images;				//images within the Tiff file
		TiffDirectory *directories; //Array of the Directories for each of the images
		
		bool SetData(std::string file_name);//reads the data from the file
		int RecImageCount(TiffDirectory *dir, int count); //counts the number of directories in the Tiff, including and after the one provided. 
		void RecSetImages(TiffDirectory *dir, int imageNumber);//reads the image data for each of the directories into the image  array
	public:
		TiffFile(std::string file_name);
		
		TiffImage** images;	// Array of the image objects in the same order as the directory array
		unsigned char* data;
		
		~TiffFile();
};

#endif