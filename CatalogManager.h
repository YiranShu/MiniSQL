#ifndef CATALOG_MANAGER_H
#define CATALOG_MANAGER_H

#include "GlobalDeclar.h"
#include "BufferManager.h"
#include "Result.h"
#include "BM_File_iterator.h"

class CatalogManager
{
public:
	CatalogManager(){}
	~CatalogManager(){}

	void OpenFile(string filename);

	void CloseFile();
/*
	input PageID, page_ptr
	ReadPage() will read the page in the catalog and point the page_ptr to it
 */
	void ReadPage(const PageID pid, BM_Page* &page_ptr);

/*
	input the name of the table to find
	FindTable() will return the pageid
	if there is no such table, it will return -1
 */
	PageID FindTable(const string search_name);

/*
	input the table name and pointer of class TABLE
	ReadTable() will store the table where t points to
	and return pageid
	if there is no such table, it will return -1
 */
	PageID ReadTable(const string table_name, TABLE* t);

/*
	input the pointer of class TABLE and Result
	WriteTable() will write the table into the catalog 
	and write the information in res
	if the table already exists, return -1
	if write successfully, return 0;
 */
	int WriteTable(const TABLE *t, Result &res);

/*
	input the table name and result 
	DeleteTable() will delete the table from the catalog
	and write the information in res
	if the table do not exists, return -1
	if delete successfully, return 0;
 */
	int DeleteTable(string table_name, Result &res);

/*
	input the pointer of class TABLE and Result
	UpdataTable will update the table in the catalog
	and write the information in res
	if the table do not exist, return -1
	if update successfully, return 0; 
 */
	int UpdateTable(const TABLE* t, Result &res);
	
private:
	BM_File* fptr;
};

#endif