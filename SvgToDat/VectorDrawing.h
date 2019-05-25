#ifndef _VectorDrawing_h
#define _VectorDrawing_h

#include <string>

class VectorDrawing
{
public:
	VectorDrawing(void);
	virtual ~VectorDrawing(void);
	virtual bool Load(const std::string &strFilename) = 0;
protected:
};

#endif // _VectorDrawing_h
