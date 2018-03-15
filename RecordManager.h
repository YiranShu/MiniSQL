#ifndef _RECORD_MANAGER_H_
#define _RECORD_MANAGER_H_
#include <string>
#include <sstream>
#include <vector>
#include "GlobalDeclar.h"
#include "Result.h"
#include "CatalogManager.h"
using namespace std;

class RecordManager
{
public:
	void selectRecords(const TABLE &table, const vector<condition> &cond, Result &result); //store the records in  the result
	void deleteRecords(const TABLE &table, const vector<condition> &cond, Result &result);
	void insertRecords(const TABLE &table, const string &record, Result &result);
	void create(const string &index_name, const string &table_name, const string &attr_name, Result &res);
	void drop(const string &index_name, Result &res);
	void DropIndexOnTable(const string &table_name, Result &res);
private:
	BM_File* fptr;
};
#endif // !_RECORD_MANAGER_H_

