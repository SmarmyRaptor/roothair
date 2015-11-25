#ifndef TIFF_READER_INCLUDED
#define TIFF_READER_INCLUDED

#include <string>
#include <fstream>
#include <cerrno>
#include <map>

struct TiffHead
{
	TiffHead(char data[]);
	unsigned short  identifier;  //determines if it is little or big endian
	unsigned short  version; //should always be 42
	unsigned long td_offset; //offset to first TiffDirectory
};



struct TiffDirectory
{
	public:
		TiffDirectory(char data[], unsigned long offset);
		unsigned short   num_entries;   //number of tags
		TiffTag*  tag_list;   			//array of tags
		unsigned long   next_td_offset; //offset to next TiffDirectory
		TiffDirectory* next_dir; //pointer to the next TiffDirectory;
		~TiffDirectory();
};

template <typename T>
struct TiffTag
{
	public
		TiffTag(char data[], unsigned long loc);
		unsigned short   tag_id;       
		unsigned short   data_type;   //what type of data the tag holds
		unsigned long   data_number;  //number of items of data_type the tag holds
		unsigned long   data_offset;  //offset to where the data is held
		bool data_in_tag; 			  // true if data is located in tag, false if offset
		T *data;
		~TiffTag();
};

class TiffFile{
	private:
		TiffFile(std::string file_name);
		TiffHead head; 				//Head of the tiff file
		int num_images;				//images within the Tiff file
		TiffDirectory *directories; //Array of the Directories for each of the images
		
		bool SetData(std::string file_name);//reads the data from the file
		int RecImageCount(TiffDirectory dir, int count); //counts the number of directories in the Tiff, including and after the one provided. 
		void RecSetImages (TiffDirectory dir, int imageNumber);//reads the image data for each of the directories into the image  array
	public:
		TiffImage *images;	// Array of the image objects in the same order as the directory array
		char* data;
		~TiffFile();
}

class TiffImage{
	private:
	public:
		TiffImage(char data[], TiffDirectory dir);
		TiffDirectory dir;
		~TiffImage()
		
}


#endif