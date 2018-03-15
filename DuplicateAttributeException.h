#ifndef _DUPLICATE_ATTRIBUTE_EXCEPTION_
#define _DUPLICATE_ATTRIBUTE_EXCEPTION_
#include <iostream>
#include <string>
using namespace std;
class DuplicateAttributeException
{
public:
	DuplicateAttributeException(const string &tableName, const string &attributeName)
	{
		cout << "There has already been index on " << attributeName << " of " << tableName << endl;
	}
};
#endif