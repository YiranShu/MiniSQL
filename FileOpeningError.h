#ifndef _FILEOPENINGERROR_H_
#define _FILEOPENINGERROR_H_
#include <iostream>
#include <string>
using namespace std;
class FileOpenError
{
public:
	FileOpenError(const string &fileName)
	{
		cout << "Unable to open " << fileName << endl;
	}
};
#endif