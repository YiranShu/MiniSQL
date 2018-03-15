#include <string>
#include <sstream>
#include <iostream>
#include "CatalogManager.h"

using namespace std;

extern BufferManager BuffM;

void CatalogManager::OpenFile(string filename)
{
	fptr = new BM_File(CATALOG_FILE_PATH + filename);
}

void CatalogManager::CloseFile()
{
	delete fptr;
}

void CatalogManager::ReadPage(const PageID pid, BM_Page* &page_ptr)
{
	BuffM.ReadPage(fptr, pid, page_ptr);
	return;
}

PageID CatalogManager::FindTable(const string search_name)
{
	PageID pid;
	RecordID rid;
	string name;
	BM_File_iterator it(this->fptr);
	for(; it != this->fptr->end(); ++it){
		pid = (*it).GetPageID();
		rid.pID = pid;
		rid.sID = 1;
		name = (*it).GetRecord(rid);
		if(name == search_name){
			return pid;
		}
	}
	return -1;
}

PageID CatalogManager::ReadTable(const string table_name, TABLE* t)
{
	RecordID rid;
	PageID pid = FindTable(table_name);
	if(pid == -1){
		return pid;
	}

	BM_Page* page_ptr;
	BuffM.ReadPage(fptr, pid, page_ptr);
	rid.pID = pid;
	rid.sID = 2;
	t->table_name = table_name;
	t->num_attr = stoi(page_ptr->GetRecord(rid));
	rid.sID++;
	t->primary_key_index = stoi(page_ptr->GetRecord(rid));
	for(int i = 1; i <= t->num_attr; i++){
		rid.sID++;
		t->attribute[i].attr_name = page_ptr->GetRecord(rid);

		rid.sID++;
		t->attribute[i].type = page_ptr->GetRecord(rid);

		rid.sID++;
		t->attribute[i].offset = stoi(page_ptr->GetRecord(rid));

		rid.sID++;
		t->attribute[i].size = stoi(page_ptr->GetRecord(rid));

		rid.sID++;
		t->attribute[i].unique = stoi(page_ptr->GetRecord(rid));

		rid.sID++;
		t->attribute[i].index_true = stoi(page_ptr->GetRecord(rid));
	}

	return pid;
}

int CatalogManager::WriteTable(const TABLE *t, Result &res)
{
//if found table, can't create table
//output ERROR
	PageID pid = FindTable(t->table_name);
	if(pid != -1){									
		res.SetError( "Table '" + t->table_name + "' already exists" );
		return -1;
	}
//allocate a new page
	BM_Page* new_page_ptr;
	PageID new_page_id;
	BuffM.AllocPage(fptr, new_page_id, new_page_ptr);
//write table into the page
//insert table_name, num_attr and primary_key_index
	stringstream ss;
	string s;
	new_page_ptr->InsertRecord(t->table_name);

	ss << t->num_attr;
	ss >> s;
	new_page_ptr->InsertRecord(s);
	ss.clear();

	ss << t->primary_key_index;
	ss >> s;
	new_page_ptr->InsertRecord(s);
	ss.clear();
//insert attributes
	for(int i = 1; i <= t->num_attr; i++){
		new_page_ptr->InsertRecord(t->attribute[i].attr_name);
		new_page_ptr->InsertRecord(t->attribute[i].type);

		ss << t->attribute[i].offset;
		ss >> s;
		ss.clear();
		new_page_ptr->InsertRecord(s);

		ss << t->attribute[i].size;
		ss >> s;
		ss.clear();
		new_page_ptr->InsertRecord(s);

		if(t->attribute[i].unique == 1)
			new_page_ptr->InsertRecord("1");
		else
			new_page_ptr->InsertRecord("0");
		if(t->attribute[i].index_true == 1)
			new_page_ptr->InsertRecord("1");
		else
			new_page_ptr->InsertRecord("0");

	}
//write page to the disk
	BuffM.unPinPage(fptr, new_page_id, true);
	BuffM.FlushFile(fptr);
//close file
	BM_File* file_ptr = new BM_File(RECORD_FILE_PATH + t->table_name + ".db");
	BuffM.FlushFile(file_ptr);
	delete file_ptr;
	return 0;
}

int CatalogManager::DeleteTable(string table_name, Result &res)
{
	PageID pid;
	pid = FindTable(table_name);
	if(pid == -1){	
		res.SetError("ERROR: No such table");
		return -1;
	}
	BuffM.DeletePage(fptr, pid);
	table_name = RECORD_FILE_PATH + table_name + ".db";		
	remove(table_name.c_str());	//remove the record file 
	return 0;
}

int CatalogManager::UpdateTable(const TABLE* t, Result &res)
{
	PageID pid;
	pid = FindTable(t->table_name);
	if(pid == -1){	
		res.SetError("ERROR: No such table");
		return -1;
	}
	BuffM.DeletePage(fptr, pid);
	return WriteTable(t, res);
}
