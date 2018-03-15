#ifndef _DUPLICATE_INDEX_NAME_EXCEPTION_H_
#define _DUPLICATE_INDEX_NAME_EXCEPTION_H_
#include <iostream>
#include <string>
using namespace std;
class DuplicateIndexNameException
{
public:
	DuplicateIndexNameException(const string &indexName)
	{
		cout << indexName << " has already been used as an index name!" << endl;
	}
};
#endif