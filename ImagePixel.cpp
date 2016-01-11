unsigned char Pixel::getLuminosity(unsigned char r, unsigned char g, unsigned char b)
{
	return r*.21f + .72f*G + 0.07*B
}
virtual unsigned char* Pixel::printPixel(){
	
}

RGBPixel::RGBPixel(char r, char g, char b)
{
	this->r = r;
	this->g = g;
	this-> b = b;
}
unsigned char* RGBPixel::printPixel(){
	unsigned char* ret[3] = {r, g, b};
	return ret;
}
RGBPixel::RGBPixel(RGBPixel copyPixel)
{
	this-> r = copyPixel.r;
	this->g = copyPixel.g;
	this-> b = copyPixel.b;
}
void RGBPixel::upContrast();
WhitePixel::WhitePixel(unsigned char v)
{
	val = v;
}
WhitePixel::WhitePixel(unsigned char r, unsigned char g, unsigned char b)
{
	val = getLuminosity(r, g, b);
}
WhitePixel::WhitePixel(WhitePixel copyPixel)
{
	val = copyPixel.getVal();
}
WhitePixel::WhitePixel(BlackPixel copyPixel)
{
	val = 255 - copyPixel.getVal();
}
WhitePixel::WhitePixel(RGBPixel convert)
{
	val = getLuminosity(convert.getR(), convert.getG(), convert.getB());
}

BlackPixel::BlackPixel(unsigned char v)
{
	val = v;
}
BlackPixel::BlackPixel(unsigned char r, unsigned char g, unsigned char b)
{
	val = 255 - getLuminosity(r, g, b);
}
BlackPixel::BlackPixel(WhitePixel copyPixel)
{
	val = 255 - copyPixel.getVal();
}
BlackPixel::BlackPixel(BlackPixel copyPixel)
{
	val = copyPixel.getVal();
}
BlackPixel::BlackPixel(RGBPixel convert)
{
	val = 255 - getLuminosity(convert.getR(), convert.getG(), convert.getB());
}