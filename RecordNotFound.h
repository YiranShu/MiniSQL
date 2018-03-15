#ifndef _RECORDNOTFOUND_H_
#define _RECORDNOTFOUND_H_
#include <iostream>
using namespace std;
class RecordNotFound
{
public:
	RecordNotFound()
	{
		cout << "Unable to find the record!" << endl;
	}
};
#endif