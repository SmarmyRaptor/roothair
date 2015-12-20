#ifndef IMAGE_PIXEL_INCLUDED
#define IMAGE_PIXEL_INCLUDED
class Pixel{
	public:
		unsigned char getLuminosity(unsigned char r, unsigned char g, unsigned char b);
};

class RGBPixel: public Pixel{
	private:
		unsigned char r;
		unsigned char g;
		unsigned char b;
	public:
		RGBPixel(char r, char g, char b);
		RGBPixel(RGBPixel copyPixel);
		
		unsigned char getR(){return r}
		unsigned char getG(){return g}
		unsigned char getB(){return b}
};

//White is 255, black is 0. TIFF photometric interpretation = 1
class WhitePixel: public Pixel{
	private:
		unsigned char val;
	public:
		WhitePixel(unsigned char v);
		WhitePixel(unsigned char r, unsigned char g, unsigned char b);
		WhitePixel(WhitePixel copyPixel);
		WhitePixel(BlackPixel copyPixel);
		WhitePixel(RGBPixel convert); //uses luminosity method 
		
		unsigned char getVal(){return val}
};

//White is 0, black is 255. TIFF photometric interpretation = 0
class BlackPixel:  public Pixel{	
	private:
		unsigned char val;
	public:
		BlackPixel(unsigned char v);
		BlackPixel(unsigned char r, unsigned char g, unsigned char b);
		BlackPixel(BlackPixel copyPixel)
		BlackPixel(WhitePixel copyPixel); 
		BlackPixel(RGBPixel convert); //uses luminosity method 
		
		unsigned char getVal(){return val};
};

#endif