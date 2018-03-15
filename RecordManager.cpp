#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include "GlobalDeclar.h"
#include "Result.h"
#include "CatalogManager.h"
#include "RecordManager.h"
#include "Functions.h"
#include "BM_File_iterator.h"
#include "BM_Page.h"
#include "BufferManager.h"
#include "DuplicateAttributeException.h"
#include "DuplicateIndexNameException.h"
#include "CatalogManager.h"
using namespace std;

extern BufferManager BuffM;
extern CatalogManager CataM;
void RecordManager::selectRecords(const TABLE &table, const vector<condition> &cond, Result &result) //store the records in  the result
{
	RecordID rid;
	string output("");
	string temp;
	int intValue, intData;
	float floatValue, floatData;
	string stringValue, stringData;
	string attribute;
	string item;
	stringstream ss;
	bool flag;
	int affectRows = 0;
	fptr = new BM_File( RECORD_FILE_PATH + table.table_name + ".db");
	BM_File_iterator it(this->fptr);
	if (cond.size() == 0)
	{
		for (; it != this->fptr->end(); ++it)
		{
			rid.pID = (*it).GetPageID();
			for (rid.sID = 1; rid.sID <= (*it).pHdr.slotNum; rid.sID++)
			{
				temp = (*it).GetRecord(rid);
				if (temp.size()) //the slot is not empty
				{
					for (unsigned int i = 1; i <= table.num_attr; i++)
					{
						if (table.attribute[i].type == "int")
						{
							ss << temp.substr(table.attribute[i].offset, table.attribute[i].size);
							ss >> intValue;
							ss.clear();
							ss << intValue;
							ss >> item;
							ss.clear();
							output += item + " ";
						}
						else if (table.attribute[i].type == "float")
						{
							ss << temp.substr(table.attribute[i].offset, table.attribute[i].size);
							ss >> floatValue;
							ss.clear();
							ss << floatValue;
							ss >> item;
							ss.clear();
							output += item + " ";
						}
						else
						{
							output += temp.substr(table.attribute[i].offset, table.attribute[i].size) + " ";
						}
					}
					output += "\n";
					affectRows++;
				}
			}
		}
	}
	else
	{
		if (cond[0].attr.index_true && cond[0].opt == "=")
		{
			string rootName = INDEX_FILE_PATH + table.table_name + cond[0].attr.attr_name + ".txt";
			rid = select(rootName, cond[0].opd);
			if (rid.pID == 0) //Not found
			{
				result.res_rows = affectRows;
				result.res = output;
				delete fptr;
				return;
			}
			else
			{
				BM_Page* page_ptr;
				BuffM.ReadPage(fptr, rid.pID, page_ptr);
				temp = page_ptr->GetRecord(rid);
				flag = true;
				for (unsigned int i = 1; i < cond.size(); i++)
				{
					attribute = temp.substr(cond[i].attr.offset, cond[i].attr.size);
					if (cond[i].attr.type == "int")
					{
						ss << attribute;
						ss >> intValue;
						ss.clear();
						ss << cond[i].opd;
						ss >> intData;
						ss.clear();
						if (cond[i].opt == "=" && intValue != intData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == "<" && intValue >= intData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == "<=" && intValue > intData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == ">=" && intValue < intData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == ">" && intValue <= intData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == "<>" && intValue == intData)
						{
							flag = false;
							break;
						}
					}
					else if (cond[i].attr.type == "float")
					{
						ss << attribute;
						ss >> floatValue;
						ss.clear();
						ss << cond[i].opd;
						ss >> floatData;
						ss.clear();
						if (cond[i].opt == "=" && floatValue != floatData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == "<" && floatValue >= floatData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == "<=" && floatValue > floatData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == ">=" && floatValue < floatData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == ">" && floatValue <= floatData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == "<>" && floatValue == floatData)
						{
							flag = false;
							break;
						}
					}
					else
					{
						stringValue = attribute;
						stringData = cond[i].opd;
						if (cond[i].opt == "=" && stringValue != stringData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == "<" && stringValue >= stringData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == "<=" && stringValue > stringData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == ">=" && stringValue < stringData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == ">" && stringValue <= stringData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == "<>" && stringValue == stringData)
						{
							flag = false;
							break;
						}
					}
				}
				if (flag)
				{
					for (unsigned int i = 1; i <= table.num_attr; i++)
					{
						if (table.attribute[i].type == "int")
						{
							ss << temp.substr(table.attribute[i].offset, table.attribute[i].size);
							ss >> intValue;
							ss.clear();
							ss << intValue;
							ss >> item;
							ss.clear();
							output += item + " ";
						}
						else if (table.attribute[i].type == "float")
						{
							ss << temp.substr(table.attribute[i].offset, table.attribute[i].size);
							ss >> floatValue;
							ss.clear();
							ss << floatValue;
							ss >> item;
							ss.clear();
							output += item + " ";
						}
						else
						{
							output += temp.substr(table.attribute[i].offset, table.attribute[i].size) + " ";
						}
					}
					output += "\n";
					affectRows++;
				}
			}
		}
		else
		{
			for (; it != this->fptr->end(); ++it)
			{
				rid.pID = (*it).GetPageID();
				for (rid.sID = 1; rid.sID <= (*it).pHdr.slotNum; rid.sID++)
				{
					temp = (*it).GetRecord(rid);
					if (temp.size()) //the slot is not empty
					{
						flag = true;
						for (unsigned int i = 0; i < cond.size(); i++)
						{
							attribute = temp.substr(cond[i].attr.offset, cond[i].attr.size);
							if (cond[i].attr.type == "int")
							{
								ss << attribute;
								ss >> intValue;
								ss.clear();
								ss << cond[i].opd;
								ss >> intData;
								ss.clear();
								if (cond[i].opt == "=" && intValue != intData)
								{
									flag = false;
									break;
								}
								else if (cond[i].opt == "<" && intValue >= intData)
								{
									flag = false;
									break;
								}
								else if (cond[i].opt == "<=" && intValue > intData)
								{
									flag = false;
									break;
								}
								else if (cond[i].opt == ">=" && intValue < intData)
								{
									flag = false;
									break;
								}
								else if (cond[i].opt == ">" && intValue <= intData)
								{
									flag = false;
									break;
								}
								else if (cond[i].opt == "<>" && intValue == intData)
								{
									flag = false;
									break;
								}
							}
							else if (cond[i].attr.type == "float")
							{
								ss << attribute;
								ss >> floatValue;
								ss.clear();
								ss << cond[i].opd;
								ss >> floatData;
								ss.clear();
								if (cond[i].opt == "=" && floatValue != floatData)
								{
									flag = false;
									break;
								}
								else if (cond[i].opt == "<" && floatValue >= floatData)
								{
									flag = false;
									break;
								}
								else if (cond[i].opt == "<=" && floatValue > floatData)
								{
									flag = false;
									break;
								}
								else if (cond[i].opt == ">=" && floatValue < floatData)
								{
									flag = false;
									break;
								}
								else if (cond[i].opt == ">" && floatValue <= floatData)
								{
									flag = false;
									break;
								}
								else if (cond[i].opt == "<>" && floatValue == floatData)
								{
									flag = false;
									break;
								}
							}
							else
							{
								stringValue = attribute;
								stringData = cond[i].opd;
								if (cond[i].opt == "=" && stringValue != stringData)
								{
									flag = false;
									break;
								}
								else if (cond[i].opt == "<" && stringValue >= stringData)
								{
									flag = false;
									break;
								}
								else if (cond[i].opt == "<=" && stringValue > stringData)
								{
									flag = false;
									break;
								}
								else if (cond[i].opt == ">=" && stringValue < stringData)
								{
									flag = false;
									break;
								}
								else if (cond[i].opt == ">" && stringValue <= stringData)
								{
									flag = false;
									break;
								}
								else if (cond[i].opt == "<>" && stringValue == stringData)
								{
									flag = false;
									break;
								}
							}
						}
						if (flag)
						{
							for (unsigned int i = 1; i <= table.num_attr; i++)
							{
								if (table.attribute[i].type == "int")
								{
									ss << temp.substr(table.attribute[i].offset, table.attribute[i].size);
									ss >> intValue;
									ss.clear();
									ss << intValue;
									ss >> item;
									ss.clear();
									output += item + " ";
								}
								else if (table.attribute[i].type == "float")
								{
									ss << temp.substr(table.attribute[i].offset, table.attribute[i].size);
									ss >> floatValue;
									ss.clear();
									ss << floatValue;
									ss >> item;
									ss.clear();
									output += item + " ";
								}
								else
								{
									output += temp.substr(table.attribute[i].offset, table.attribute[i].size) + " ";
								}
							}
							output += "\n";
							affectRows++;
						}
					}
				}
			}
		}
	}
	result.res_rows = affectRows;
	result.res = output;
	delete fptr;
}

void RecordManager::deleteRecords(const TABLE &table, const vector<condition> &cond, Result &result)
{
	RecordID rid;
	string output("");
	string temp;
	int intValue, intData;
	float floatValue, floatData;
	string stringValue, stringData;
	string attribute;
	stringstream ss;
	BM_Page this_page;
	bool flag;
	int affectRows = 0;
	int i;
	bool indexTrue = false;
	int intKey;
	float floatKey;
	string stringKey;

	fptr = new BM_File(RECORD_FILE_PATH + table.table_name + ".db");
	BM_File_iterator it(this->fptr);
	for (i = 1; i <= table.num_attr; i++)
	{
		if (table.attribute[i].index_true)
		{
			indexTrue = true;
			break;
		}
	}
	if (cond.size() == 0)
	{
		for (; it != this->fptr->end(); ++it)
		{
			rid.pID = (*it).GetPageID();
			for (rid.sID = 1; rid.sID <= (*it).pHdr.slotNum; rid.sID++)
			{
				temp = (*it).GetRecord(rid);
				if (temp.size()) //the slot is not empty
				{
					this_page = *it;
					this_page.DeletRecord(rid);
					fptr->WritePage(this_page);
					affectRows++;
				}
			}
		}
		for(int i = 1; i <=table.num_attr; i++){
			if (table.attribute[i].index_true == true)
			{
				ofstream write;
				ifstream read;
				string dataType;
				read.open(INDEX_FILE_PATH+ table.table_name + table.attribute[i].attr_name + ".txt", ios::in);
				read >> dataType;
				read.close();
				write.open(INDEX_FILE_PATH+ table.table_name + table.attribute[i].attr_name + ".txt", ios::trunc);
				write.close();
				write.open(INDEX_FILE_PATH+ table.table_name + table.attribute[i].attr_name + ".txt", ios::app);
				write << dataType << endl << 1 << endl;
				write.close();
			}			
		}

	}

	for (; it != this->fptr->end(); ++it)
	{
		rid.pID = (*it).GetPageID();
		for (rid.sID = 1; rid.sID <= (*it).pHdr.slotNum; rid.sID++)
		{
			temp = (*it).GetRecord(rid);
			if (temp.size()) //the slot is not empty
			{
				flag = true;
				for (unsigned int i = 0; i < cond.size(); i++)
				{
					attribute = temp.substr(cond[i].attr.offset, cond[i].attr.size);
					if (cond[i].attr.type == "int")
					{
						ss << attribute;
						ss >> intValue;
						ss.clear();
						ss << cond[i].opd;
						ss >> intData;
						ss.clear();
						if (cond[i].opt == "=" && intValue != intData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == "<" && intValue >= intData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == "<=" && intValue > intData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == ">=" && intValue < intData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == ">" && intValue <= intData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == "<>" && intValue == intData)
						{
							flag = false;
							break;
						}
					}
					else if (cond[i].attr.type == "float")
					{
						ss << attribute;
						ss >> floatValue;
						ss.clear();
						ss << cond[i].opd;
						ss >> floatData;
						ss.clear();
						if (cond[i].opt == "=" && floatValue != floatData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == "<" && floatValue >= floatData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == "<=" && floatValue > floatData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == ">=" && floatValue < floatData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == ">" && floatValue <= floatData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == "<>" && floatValue == floatData)
						{
							flag = false;
							break;
						}
					}
					else
					{
						stringValue = attribute;
						stringData = cond[i].opd;
						if (cond[i].opt == "=" && stringValue != stringData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == "<" && stringValue >= stringData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == "<=" && stringValue > stringData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == ">=" && stringValue < stringData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == ">" && stringValue <= stringData)
						{
							flag = false;
							break;
						}
						else if (cond[i].opt == "<>" && stringValue == stringData)
						{
							flag = false;
							break;
						}
					}
				}
				if (flag)
				{
					this_page = *it;
					this_page.DeletRecord(rid);
					fptr->WritePage(this_page);
					affectRows++;
					for(int i = 1; i <=table.num_attr; i++){
						if (table.attribute[i].index_true == true){
							string leaf;
							ifstream read;
							ofstream write;
							string dataType;
							bool isLeaf;
							int type;
							string parent;
							int size;
							string child1, child2;
							string key;

							read.open(INDEX_FILE_PATH + table.table_name + table.attribute[i].attr_name + ".txt", ios::in);
							read >> dataType >> isLeaf >> type >> parent >> size;
							read >> child1 >> key >> child2;
							read.close();
							read.open(child2, ios::in);
							read >> dataType >> isLeaf >> type >> parent >> size;
							read.close();
							if (size == 1 && isLeaf)
							{
								write.open(INDEX_FILE_PATH + table.table_name + table.attribute[i].attr_name + ".txt", ios::trunc);
								write.close();
								write.open(INDEX_FILE_PATH + table.table_name + table.attribute[i].attr_name + ".txt", ios::app);
								write << dataType << endl << 1 << endl;
								write.close();
							}
							else
							{
								if (table.attribute[i].type == "int")
								{
									ss << temp.substr(table.attribute[i].offset, table.attribute[i].size);
									ss >> intKey;
									ss.clear();
									ss << intKey;
									ss >> stringKey;
									ss.clear();
									leaf = selectLeafFile(INDEX_FILE_PATH + table.table_name + table.attribute[i].attr_name + ".txt", stringKey);
									deleteRecord(leaf, stringKey);
								}
								else if (table.attribute[i].type == "float")
								{
									ss << temp.substr(table.attribute[i].offset, table.attribute[i].size);
									ss >> floatKey;
									ss.clear();
									ss << floatKey;
									ss >> stringKey;
									ss.clear();
									leaf = selectLeafFile(INDEX_FILE_PATH + table.table_name + table.attribute[i].attr_name + ".txt", stringKey);
									deleteRecord(leaf, stringKey);
								}
								else
								{
									stringKey = temp.substr(table.attribute[i].offset, table.attribute[i].size);
									leaf = selectLeafFile(INDEX_FILE_PATH + table.table_name + table.attribute[i].attr_name + ".txt", stringKey);
									deleteRecord(leaf, stringKey);
								}
							}
						}
					}
				}
			}
		}
	}
	result.affected_rows = affectRows;
	delete fptr;
}

void RecordManager::insertRecords(const TABLE &table, const string &record, Result &result)
{
	RecordID recordID;
	BM_Page *page;
	BM_Page this_page;
	PageID pid;
	bool flag = false;
	bool indexTrue = false;
	
	bool isLeaf;
	string dataType;
	int intKey;
	float floatKey;
	string stringKey;
	stringstream ss;
	int i;
	for (i = 1; i <= table.num_attr; i++)
	{
		if (table.attribute[i].index_true)
		{
			indexTrue = true;
			break;
		}
	}

	fptr = new BM_File(RECORD_FILE_PATH + table.table_name + ".db");
	BM_File_iterator it(this->fptr);

	for (; it != this->fptr->end(); ++it)
	{
		this_page = *it;
		if (this_page.StillHaveSpace(sizeof(BM_Slot_Header) + record.length()))
		{
			this_page.InsertRecord(record, recordID);
			BuffM.unPinPage(fptr, recordID.pID, false);
			fptr->WritePage(this_page);
			BuffM.FlushFile(fptr);
			delete fptr;
			for(int i = 1; i <=table.num_attr; i++){
				bool notHaveRoot = false;
				if (table.attribute[i].index_true == true){
					ifstream read;
					read.open(INDEX_FILE_PATH + table.table_name + table.attribute[i].attr_name + ".txt", ios::in);
					read >> dataType >> isLeaf;
					if (isLeaf)
						notHaveRoot = true;
					else
						notHaveRoot = false;
					read.close();
				
					string leaf;
					if (table.attribute[i].type == "int")
					{
						ss << record.substr(table.attribute[i].offset, table.attribute[i].size);
						ss >> intKey;
						ss.clear();
						ss << intKey;
						ss >> stringKey;
						ss.clear();
						if (notHaveRoot)
							createRoot(table.table_name + table.attribute[i].attr_name, stringKey, recordID);
						else
						{
							leaf = selectLeafFile( INDEX_FILE_PATH +table.table_name + table.attribute[i].attr_name + ".txt", stringKey);
							insert(leaf, stringKey, recordID);
						}
					}
					else if (table.attribute[i].type == "float")
					{
						ss << record.substr(table.attribute[i].offset, table.attribute[i].size);
						ss >> floatKey;
						ss.clear();
						ss << floatKey;
						ss >> stringKey;
						ss.clear();
						if (notHaveRoot)
							createRoot(table.table_name + table.attribute[i].attr_name, stringKey, recordID);
						else
						{
							leaf = selectLeafFile( INDEX_FILE_PATH +table.table_name + table.attribute[i].attr_name + ".txt", stringKey);
							insert(leaf, stringKey, recordID);
						}
					}
					else
					{
						stringKey = record.substr(table.attribute[i].offset, table.attribute[i].size);
						if (notHaveRoot)
							createRoot(table.table_name + table.attribute[i].attr_name, stringKey, recordID);
						else
						{
							leaf = selectLeafFile( INDEX_FILE_PATH +table.table_name + table.attribute[i].attr_name + ".txt", stringKey);
							insert(leaf, stringKey, recordID);
						}
					}
				}
			}

			flag = true; //insert successfully
			break;
		}
	}
	if (!flag)
	{
		BuffM.AllocPage(fptr, recordID.pID, page);
		page->InsertRecord(record, recordID);
		BuffM.unPinPage(fptr, recordID.pID, false);
		fptr->WritePage(*page);
		BuffM.FlushFile(fptr);
		delete fptr;
		for(int i = 1; i <=table.num_attr; i++){
			bool notHaveRoot = false;
			if (table.attribute[i].index_true == true){
				ifstream read;
				read.open(INDEX_FILE_PATH + table.table_name + table.attribute[i].attr_name + ".txt", ios::in);
				read >> dataType >> isLeaf;
				if (isLeaf)
					notHaveRoot = true;
				else
					notHaveRoot = false;
				read.close();	

				string leaf;
				if (table.attribute[i].type == "int")
				{
					ss << record.substr(table.attribute[i].offset, table.attribute[i].size);
					ss >> intKey;
					ss.clear();
					ss << intKey;
					ss >> stringKey;
					ss.clear();
					if (notHaveRoot)
						createRoot(table.table_name + table.attribute[i].attr_name, stringKey, recordID);
					else
					{
						leaf = selectLeafFile(INDEX_FILE_PATH +table.table_name + table.attribute[i].attr_name + ".txt", stringKey);
						insert(leaf, stringKey, recordID);
					}
				}
				else if (table.attribute[i].type == "float")
				{
					ss << record.substr(table.attribute[i].offset, table.attribute[i].size);
					ss >> floatKey;
					ss.clear();
					ss << floatKey;
					ss >> stringKey;
					ss.clear();
					if (notHaveRoot)
						createRoot(table.table_name + table.attribute[i].attr_name, stringKey, recordID);
					else
					{
						leaf = selectLeafFile(INDEX_FILE_PATH +table.table_name + table.attribute[i].attr_name + ".txt", stringKey);
						insert(leaf, stringKey, recordID);
					}
				}
				else
				{
					stringKey = record.substr(table.attribute[i].offset, table.attribute[i].size);
					if (notHaveRoot)
						createRoot(table.table_name + table.attribute[i].attr_name, stringKey, recordID);
					else
					{
						leaf = selectLeafFile(INDEX_FILE_PATH +table.table_name + table.attribute[i].attr_name + ".txt", stringKey);
						insert(leaf, stringKey, recordID);
					}
				}
			}
		}
	}
	result.affected_rows = 1;
}

void RecordManager::create(const string &index_name, const string &table_name, const string &attr_name, Result &res)
{
	ifstream read;
	string type;
	Result result;
	TABLE t;
	int i;

	CataM.ReadTable(table_name, &t);
	for (i = 1; i <= t.num_attr; i++)
	{
		if (t.attribute[i].attr_name == attr_name && t.attribute[i].unique)
		{
			type = t.attribute[i].type;
			break;
		}
	}
	if (i > t.num_attr)
	{
		res.SetError("No such unique attribute");
		return;
	}
	read.open("index\\index.txt", ios::in);
	if (!read)
		createIndexFile();
	else
		read.close();
	try
	{
		createIndex(index_name, INDEX_FILE_PATH + table_name + attr_name + ".txt", table_name, attr_name, type);
	}
	catch (const DuplicateAttributeException &e)
	{
		res.SetError("Duplicate attribute!");
		return;
	}
	catch (const DuplicateIndexNameException &e)
	{
		res.SetError("Duplicate index!");
		return;
	}
	t.attribute[i].index_true = true;
	
	CataM.UpdateTable(&t, result);

	//sss
	string temp;
	RecordID rid;
	bool NotHaveRoot = true;
	int intKey;
	float floatKey;
	string stringKey;
	string leaf;
	stringstream ss;
	for (i = 1; i <= t.num_attr; i++)
	{
		if (attr_name == t.attribute[i].attr_name)
			break;
	}
	fptr = new BM_File(RECORD_FILE_PATH + t.table_name + ".db");
	BM_File_iterator it(this->fptr);
	for (; it != this->fptr->end(); ++it)
	{
		rid.pID = (*it).GetPageID();
		for (rid.sID = 1; rid.sID <= (*it).pHdr.slotNum; rid.sID++)
		{
			temp = (*it).GetRecord(rid);
			if (temp.size()) //it is a record
			{
				if (t.attribute[i].type == "int")
				{
					ss << temp.substr(t.attribute[i].offset, t.attribute[i].size);
					ss >> intKey;
					ss.clear();
					ss << intKey;
					ss >> stringKey;
					ss.clear();
					if (NotHaveRoot)
					{
						createRoot(t.table_name + t.attribute[i].attr_name, stringKey, rid);
						NotHaveRoot = false;
					}
					else
					{
						leaf = selectLeafFile(INDEX_FILE_PATH + t.table_name + t.attribute[i].attr_name + ".txt", stringKey);
						insert(leaf, stringKey, rid);
					}
				}
				else if (t.attribute[i].type == "float")
				{
					ss << temp.substr(t.attribute[i].offset, t.attribute[i].size);
					ss >> floatKey;
					ss.clear();
					ss << floatKey;
					ss >> stringKey;
					ss.clear();
					if (NotHaveRoot)
					{
						createRoot(t.table_name + t.attribute[i].attr_name, stringKey, rid);
						NotHaveRoot = false;
					}
					else
					{
						leaf = selectLeafFile(INDEX_FILE_PATH + t.table_name + t.attribute[i].attr_name + ".txt", stringKey);
						insert(leaf, stringKey, rid);
					}
				}
				else
				{
					stringKey = temp.substr(t.attribute[i].offset, t.attribute[i].size);
					if (NotHaveRoot)
					{
						createRoot(t.table_name + t.attribute[i].attr_name, stringKey, rid);
						NotHaveRoot = false;
					}
					else
					{
						leaf = selectLeafFile(INDEX_FILE_PATH + t.table_name + t.attribute[i].attr_name + ".txt", stringKey);
						insert(leaf, stringKey, rid);
					}
				}
			}
		}
	}
	delete fptr;
	//sss
}

string getSuffix()
{
	fstream idx("index\\index.txt", ios::in | ios::out);
	string suffix;
	idx.seekp(0, ios::beg);
	idx >> suffix;
	return suffix;
}

void RecordManager::drop(const string &index_name, Result &res)
{
	string temp;
	int flag = 0;
	string index;
	string root;
	string table;
	string attribute;
	string type;
	vector<string> indexVec, rootVec, tableVec, attributeVec, typeVec;
	ifstream read;
	unsigned int k;

	read.open("index\\index.txt", ios::in);
	read >> temp;
	while (read >> temp)
	{
		if(flag == 0)
		{
			flag++;
			indexVec.push_back(temp);
		}
		else if(flag == 1)
		{
			flag++;
			rootVec.push_back(temp);
		}
		else if(flag == 2)
		{
			flag++;
			tableVec.push_back(temp);
		}
		else if(flag == 3)
		{
			flag++;
			attributeVec.push_back(temp);
		}
		else if(flag == 4)
		{
			flag = 0;
			typeVec.push_back(temp);
		}
	}
	read.close();
	
	vector<string>::iterator it = find(indexVec.begin(), indexVec.end(), index_name);
	if (it != indexVec.end())
	{
		k = it - indexVec.begin();
		it = indexVec.erase(it);
		string delcmd = "del " + rootVec[k];
		delcmd = delcmd.substr(0, delcmd.length() - 4) + "*.txt";
		system( delcmd.c_str() );
		remove(rootVec[k].c_str());
		it = find(rootVec.begin(), rootVec.end(), rootVec[k]);
		it = rootVec.erase(it);
		table = tableVec[k];
		attribute = attributeVec[k];
		TABLE t;
		Result result;
		CataM.ReadTable(table, &t);
		for (int i = 1; i <= t.num_attr; i++)
		{
			if (t.attribute[i].attr_name == attribute)
			{
				t.attribute[i].index_true = false;
				break;
			}
		}
		CataM.UpdateTable(&t, result);
		it = find(tableVec.begin(), tableVec.end(), tableVec[k]);
		it = tableVec.erase(it);
		it = find(attributeVec.begin(), attributeVec.end(), attributeVec[k]);
		it = attributeVec.erase(it);
		it = find(typeVec.begin(), typeVec.end(), typeVec[k]);
		it = typeVec.erase(it);

		ofstream write;
		string suffix = getSuffix();
		write.open("index\\index.txt", ios::trunc);
		write.close();
		write.open("index\\index.txt", ios::app);
		if(indexVec.empty())
			write << 0 << endl;
		else
			write << suffix << endl;
		for (k = 0; k < indexVec.size(); k++)
			write << indexVec[k] << endl << rootVec[k] << endl << tableVec[k] << endl << attributeVec[k] << endl << typeVec[k] << endl;
		write.close();

	}
	else
	{
		res.SetError("Index not found!");
	}
	// fstream file("index\\index.txt", ios::in|ios::out);
	// string buf;
	// file.seekp(0, ios::beg);
	// file >> buf;
	// cout << file.eof() <<endl;
	// if(file.eof()){
	// 	file.seekg(0, ios::beg);
	// 	file << 0 << endl;
	// }
	// file.close();
}

void RecordManager::DropIndexOnTable(const string &table_name, Result &res)
{
	string temp;
	int flag = 0;
	string index;
	string root;
	string table;
	string attribute;
	string type;
	vector<string> indexVec, rootVec, tableVec, attributeVec, typeVec;
	ifstream read;
	unsigned int k;

	read.open("index\\index.txt", ios::in);
	read >> temp;
	while (read >> temp)
	{
		if (flag == 0)
		{
			flag++;
			indexVec.push_back(temp);
		}
		else if (flag == 1)
		{
			flag++;
			rootVec.push_back(temp);
		}
		else if (flag == 2)
		{
			flag++;
			tableVec.push_back(temp);
		}
		else if (flag == 3)
		{
			flag++;
			attributeVec.push_back(temp);
		}
		else if (flag == 4)
		{
			flag = 0;
			typeVec.push_back(temp);
		}
	}
	read.close();

	vector<string>::iterator it = find(tableVec.begin(), tableVec.end(), table_name);
	while (it != tableVec.end())
	{
		k = it - tableVec.begin();
		attribute = attributeVec[k];
		TABLE t;
		Result result;
		CataM.ReadTable(table_name, &t);
		for (int i = 1; i <= t.num_attr; i++)
		{
			if (t.attribute[i].attr_name == attribute)
			{
				t.attribute[i].index_true = false;
				break;
			}
		}
		CataM.UpdateTable(&t, result);
		it = tableVec.erase(it);
		string delcmd = "del " + rootVec[k];
		delcmd = delcmd.substr(0, delcmd.length() - 4) + "*.txt";
		system( delcmd.c_str() );
		remove(rootVec[k].c_str());
		it = find(rootVec.begin(), rootVec.end(), rootVec[k]);
		it = rootVec.erase(it);
		it = find(indexVec.begin(), indexVec.end(), indexVec[k]);
		it = indexVec.erase(it);
		it = find(attributeVec.begin(), attributeVec.end(), attributeVec[k]);
		it = attributeVec.erase(it);
		it = find(typeVec.begin(), typeVec.end(), typeVec[k]);
		it = typeVec.erase(it);

		ofstream write;
		string suffix = getSuffix();
		write.open("index\\index.txt", ios::trunc);
		write.close();
		write.open("index\\index.txt", ios::app);
		if(indexVec.empty())
			write << "0000000000" << endl;
		else
			write << suffix << endl;
		for (k = 0; k < indexVec.size(); k++)
			write << indexVec[k] << endl << rootVec[k] << endl << tableVec[k] << endl << attributeVec[k] << endl << typeVec[k] << endl;
		write.close();
		it = find(tableVec.begin(), tableVec.end(), table_name);
	}
	// fstream file("index\\index.txt", ios::in|ios::out);
	// string buf;
	// file.seekp(0, ios::beg);
	// file >> buf;
	// cout << file.eof() <<endl;
	// if(file.eof()){
	// 	file.seekg(0, ios::beg);
	// 	file << 0 << endl;
	// }
	// file.close();
}