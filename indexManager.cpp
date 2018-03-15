#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
#include "FileOpeningError.h"
#include "DuplicateIndexNameException.h"
#include "DuplicateAttributeException.h"
#include "Ascend.h"
#include "Functions.h"
#include "RecordNotFound.h"
#include "BM_Page.h"
#include "GlobalDeclar.h"
using namespace std;

#define INDEX_FILE_NAME "index\\index.txt"
const int Order = 6;
const int MAX_CNT = 5;
const int MIN_CNT = 2;

enum NodeType
{
	Root,
	Branch,
	Leaf
};

struct Node
{
	bool isLeaf;
	NodeType type;
	string parent;
	int size;
	int key[5];
	string child[6];
};

template<class T>
T translate(const string &content)
{
	stringstream ss;
	T translation;

	ss << content;
	ss >> translation;
	ss.clear();

	return translation;
}

template<class T>
string toString(T content)
{
	string s;
	stringstream ss;

	ss << content;
	ss >> s;
	ss.clear();

	return s;
}

int GetSuffixplus()
{
	fstream idx(INDEX_FILE_NAME, ios::in | ios::out);
	int suffix;
	idx.seekp(0, ios::beg);
	idx >> suffix;
	int newsuffix = suffix + 1;
	string nsufstr = toString(newsuffix);	
	int nlength = nsufstr.length();
	nsufstr.insert(0, 10 - nlength, '0');

	idx.seekg(0, ios::beg);
	idx << nsufstr;
	return suffix;
}

void filterDigit(string &content)
{
	string::iterator t;

	for (t = content.begin(); t != content.end(); t++)
	{
		if (*t == '.')
		{
			content = content.substr(0, t - content.begin());
			break;
		}
	}
	t = content.begin();
	while (t != content.end())
	{
		if (*t >= '0' && *t <= '9')
			t = content.erase(t);
		else
			t++;
	}
	content = content + toString<int>(GetSuffixplus()) + ".txt";
}

void findRootName(string &file)
{
	string::iterator t;
	t = file.begin();
	while (t != file.end())
	{
		if (*t >= '0' && *t <= '9')
			t = file.erase(t);
		else
			t++;
	}
}

string createFileName(const string &rootName)
{
	return INDEX_FILE_PATH + rootName + toString(GetSuffixplus()) + ".txt";
}

void createIndexFile()
{
	ofstream index(INDEX_FILE_NAME);
	index << "0000000000" << endl;
	index.close();
}

void createIndex(const string &indexName, const string &rootName, const string &tableName, const string &attributeName, const string &type)
{
	ifstream index(INDEX_FILE_NAME);
	string indexBuffer, tableBuffer, attributeBuffer, typeBuffer, rootBuffer;
	int flag = 0;
	string temp;

	if (!index.is_open())
	{
		throw FileOpenError(INDEX_FILE_NAME);
	}
	index >> temp;
	while (index >> temp)
	{
		if(flag == 0)
		{
			flag++;
			indexBuffer = temp;
			if (indexBuffer == indexName) //duplicate index name
				throw DuplicateIndexNameException(indexName);
		}

		else if(flag == 1)
		{
			flag++;
			rootBuffer = temp;
		}

		else if(flag == 2)
		{
			flag++;
			tableBuffer = temp;
			if (tableBuffer == tableName && attributeBuffer == attributeName )
				throw DuplicateAttributeException(tableName, tableName);
		}

		else if(flag == 3)
		{
			flag++;
			attributeBuffer = temp;
		}

		else if(flag == 4)
		{
			flag = 0;
			typeBuffer = temp;
		}
		//Exception 4, will be extended
	}
	//So far there have been no exceptions.
	index.close();

	ofstream write;
	write.open(INDEX_FILE_NAME, ios_base::app);
	if (!write.is_open())
	{
		throw FileOpenError(INDEX_FILE_NAME);
	}
	if(type == "int" || type == "float")
		write << indexName << endl << rootName << endl << tableName << endl << attributeName << endl << type << endl;
	else
		write << indexName << endl << rootName << endl << tableName << endl << attributeName << endl << "string" << endl;
	write.close();

	write.open(rootName, ios_base::app); //create root file
	if (!write.is_open())
	{
		throw FileOpenError(rootName);
	}
	if(type == "int" || type == "float")
		write << type << endl << 1 << endl;
	else
		write << "string" << endl << 1 << endl;
	write.close();
}

void createLeaf(const string &fileName, const string &dataType, const string &parent)
{
	ofstream write(fileName);
	write << dataType << endl << 1 << endl << 2 << endl << parent << endl
		<< 0 << endl;

	write.close();
}

void createLeaf(const string &fileName, const string &dataType, const string &parent, const string &key, const RecordID &recordID)
{
	ofstream write(fileName);
	write << dataType << endl << 1 << endl << 2 << endl << parent << endl
		<< 1 << endl << key << endl << recordID.pID << " " << recordID.sID << endl;

	write.close();
}

void createRoot(const string &rootName, const string &key, const RecordID &recordID)
{
	ifstream in(INDEX_FILE_PATH + rootName + ".txt");
	string dataType;

	if (!in.is_open())
	{
		throw FileOpenError(INDEX_FILE_PATH + rootName + ".txt");
	}
	in >> dataType;
	in.close();

	string fileName = createFileName(rootName);
	ofstream write;
	write.open(INDEX_FILE_PATH + rootName + ".txt", ios::trunc);
	write.close();
	write.open(INDEX_FILE_PATH + rootName + ".txt", ios_base::app); //update root file
	write << dataType << endl << 0 << endl << 0 << endl << 0 << endl
		<< 1 << endl << fileName << endl; //isLeaf, type, parent, size, the first pointer is null
	createLeaf(fileName, dataType, INDEX_FILE_PATH + rootName + ".txt");

	fileName = createFileName(rootName);
	write << key << endl << fileName << endl;
	createLeaf(fileName, dataType, INDEX_FILE_PATH + rootName + ".txt", key, recordID);

	write.close();
}

//select the leaf file that new record will be inserted or old record will be deleted
string selectLeafFile(const string &root, const string &key)
{
	ifstream search;
	string dataType;
	bool isLeaf;
	int type;
	string parent;
	int size;
	string key1;
	string temp;
	int i;
	int intKey1, intKey2;
	float floatKey1, floatKey2;
	string result; //initialize

	search.open(root, ios_base::in); //open the root file
	search >> dataType >> isLeaf >> type;
	search >> parent;
	search >> size;
	
	if (isLeaf)
	{
		result = root;
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			search >> temp >> key1;
			if (dataType == "int")
			{
				intKey1 = translate<int>(key1);
				intKey2 = translate<int>(key);
				if (intKey2 < intKey1)
				{
					result = selectLeafFile(temp, key);
					break;
				}
			}
			else if (dataType == "float")
			{
				floatKey1 = translate<float>(key1);
				floatKey2 = translate<float>(key);
				if (floatKey2 < floatKey1)
				{
					result = selectLeafFile(temp, key);
					break;
				}
			}
			else if (dataType == "string")
			{
				if (key < key1)
				{
					result = selectLeafFile(temp, key);
					break;
				}
			}
		}
		if (i >= size && size != 0)
		{
			search >> temp;
			result = selectLeafFile(temp, key);
		}
	}
	search.close();
	return result;
}

void splitLeaf(const string &file, const string &dataType, const string &parent, const string &key1, const string &key2, const string &key3, const RecordID &recordID1, const RecordID &recordID2, const RecordID &recordID3)
{
	ofstream write;
	write.open(file, ios_base::app);
	write << dataType << endl << 1 << endl << 2 << endl << parent << endl
		<< Order / 2 << endl << key1 << endl << recordID1.pID << " " << recordID1.sID << endl
		<< key2 << endl << recordID2.pID << " " << recordID2.sID << endl
		<< key3 << endl << recordID3.pID << " " << recordID3.sID << endl;

	write.close();
}

void splitBranch(const string &file, const string &dataType, const string &parent, const string &key1, const string &key2, const string &child1, const string &child2, const string &child3)
{
	ofstream write;
	write.open(file, ios_base::app);
	write << dataType << endl << 0 << endl << 1 << endl << parent << endl
		<< Order / 2 - 1 << endl << child1 << endl << key1 << endl
		<< child2 << endl << key2 << endl
		<< child3 << endl;

	write.close();
}

void newRoot(const string &file, const string &dataType, const string &key, const string &branch1, const string &branch2)
{
	ofstream write;

	write.open(file, ios::app);
	write << dataType << endl << 0 << endl << 0 << endl << 0 << endl << 1 << endl
		<< branch1 << endl << key << endl << branch2 << endl;
	write.close();
}

void changeParent(const string &file, const string &newParent)
{
	string dataType;
	string temp;
	vector<string> key, pointer;
	vector<RecordID> recordIDs;
	PageID pid;
	SlotID sid;
	bool isLeaf;
	int type;
	string parent;
	int size;
	ifstream read;
	int i;

	read.open(file, ios_base::in);
	read >> dataType >> isLeaf >> type >> parent >> size;

	if (isLeaf)
	{
		for (i = 0; i < size; i++)
		{
			read >> temp >> pid >> sid;
			key.push_back(temp);
			recordIDs.push_back(RecordID(pid, sid));
		}
		read.close();
		ofstream write;
		write.open(file, ios::trunc);
		write.close();
		write.open(file, ios_base::app);
		write << dataType << endl << isLeaf << endl << type << endl
			<< newParent << endl << size << endl;
		for (i = 0; i < size; i++)
			write << key[i] << endl << recordIDs[i].pID << " " << recordIDs[i].sID << endl;
		write.close();
	}
	else
	{
		for (i = 0; i < size; i++)
		{
			read >> temp;    //@@@
			pointer.push_back(temp);
			read >> temp;    //@@@
			key.push_back(temp);
		}
		if (size)
		{
			read >> temp;  //@@@
			pointer.push_back(temp);
		}
		read.close();

		ofstream write;
		write.open(file, ios::trunc);
		write.close();
		write.open(file, ios_base::app);
		write << dataType << endl << isLeaf << endl << type << endl
			<< newParent << endl << size << endl;
		for (i = 0; i < size; i++)
			write << pointer[i] << endl << key[i] << endl;
		if (size)
			write << pointer[i] << endl;
		write.close();
	}
}

//string leaf = selectLeafFile(root, key); //find the leaf file which will be updated
void insert(const string &leaf, const string &key, const RecordID &recordID) //!!!
{
	ifstream search;
	string dataType;
	bool isLeaf;
	int type;
	string parent;
	int size;
	vector<int> intRecord;
	vector<float> floatRecord;
	vector<string> stringRecord;
	vector<RecordID> recordIDs;
	string temp;
	int i;
	unsigned int j;
	int tempPID, tempSID;

	search.open(leaf, ios_base::in); //open the leaf file
	search >> dataType >> isLeaf >> type >> parent >> size;

	if (size == 0)
	{
		search.close();
		ofstream write;
		write.open(leaf, ios::trunc);
		write.close();
		write.open(leaf, ios_base::app);
		write << dataType << endl << isLeaf << endl << type << endl
			<< parent << endl << 1 << endl << key << endl << recordID.pID << " " << recordID.sID << endl;
		write.close();
	}
	else
	{
		if (dataType == "int")
		{
			for (i = 0; i < size; i++)
			{
				search >> temp >> tempPID >> tempSID;  //@@@
				intRecord.push_back(translate<int>(temp));
				recordIDs.push_back(RecordID(tempPID, tempSID));
			}
			intRecord.push_back(translate<int>(key)); //###
			sort(intRecord.begin(), intRecord.end(), Ascend<int>()); //###
			for (j = 0; j < intRecord.size(); j++)
				if (intRecord[j] == translate<int>(key))
					break;
			recordIDs.insert(recordIDs.begin() + j, recordID);
			search.close();
			ofstream write;
			write.open(leaf, ios::trunc);
			write.close(); //清空重写
			write.open(leaf, ios_base::app);
			if (intRecord.size() <= MAX_CNT)//直接插入
			{
				write << dataType << endl << isLeaf << endl << type << endl
					<< parent << endl << intRecord.size() << endl;
				for (i = 0; i < intRecord.size(); i++) //###
					write << intRecord[i] << endl << recordIDs[i].pID << " " << recordIDs[i].sID << endl; //###
				write.close();
			}
			else //分裂
			{
				write << dataType << endl << isLeaf << endl << type << endl
					<< parent << endl << Order / 2 << endl;
				for (i = 0; i < Order / 2; i++)
					write << intRecord[i] << endl << recordIDs[i].pID << " " << recordIDs[i].sID << endl;
				write.close();
				string newLeaf(leaf);
				filterDigit(newLeaf);
				string key1 = toString<int>(intRecord[i]);
				string key2 = toString<int>(intRecord[i + 1]);
				string key3 = toString<int>(intRecord[i + 2]);
				splitLeaf(newLeaf, dataType, parent, key1, key2, key3, recordIDs[i], recordIDs[i + 1], recordIDs[i + 2]);
				insertBranch(parent, newLeaf, key1);
			}
		}
		else if (dataType == "float")
		{
			for (i = 0; i < size; i++)
			{
				search >> temp >> tempPID >> tempSID;  //@@@
				floatRecord.push_back(translate<float>(temp));
				recordIDs.push_back(RecordID(tempPID, tempSID));
			}
			floatRecord.push_back(translate<float>(key)); //###
			sort(floatRecord.begin(), floatRecord.end(), Ascend<float>()); //###
			for (j = 0; j < floatRecord.size(); j++)
				if (floatRecord[j] == translate<float>(key))
					break;
			recordIDs.insert(recordIDs.begin() + j, recordID);
			search.close();
			ofstream write;
			write.open(leaf, ios::trunc);
			write.close(); //清空重写
			write.open(leaf, ios_base::app);
			if (floatRecord.size() <= MAX_CNT)//直接插入
			{
				write << dataType << endl << isLeaf << endl << type << endl
					<< parent << endl << floatRecord.size() << endl;
				for (i = 0; i < floatRecord.size(); i++) //###
					write << floatRecord[i] << endl << recordIDs[i].pID << " " << recordIDs[i].sID << endl; //###
				write.close();
			}
			else //分裂
			{
				write << dataType << endl << isLeaf << endl << type << endl
					<< parent << endl << Order / 2 << endl;
				for (i = 0; i < Order / 2; i++)
					write << floatRecord[i] << endl << recordIDs[i].pID << " " << recordIDs[i].sID << endl; //###
				write.close();
				string newLeaf(leaf);
				filterDigit(newLeaf);
				string key1 = toString<float>(floatRecord[i]);
				string key2 = toString<float>(floatRecord[i + 1]);
				string key3 = toString<float>(floatRecord[i + 2]);
				splitLeaf(newLeaf, dataType, parent, key1, key2, key3, recordIDs[i], recordIDs[i + 1], recordIDs[i + 2]);
				insertBranch(parent, newLeaf, key1);
			}
		}
		else if (dataType == "string")
		{
			for (i = 0; i < size; i++)
			{
				search >> temp >> tempPID >> tempSID;  //@@@
				stringRecord.push_back(temp);
				recordIDs.push_back(RecordID(tempPID, tempSID));
			}
			stringRecord.push_back(key); //###
			sort(stringRecord.begin(), stringRecord.end(), Ascend<string>()); //###
			for (j = 0; j < stringRecord.size(); j++)
				if (stringRecord[j] == key)
					break;
			recordIDs.insert(recordIDs.begin() + j, recordID);
			search.close();
			ofstream write;
			write.open(leaf, ios::trunc);
			write.close(); //清空重写
			write.open(leaf, ios_base::app);
			if (stringRecord.size() <= MAX_CNT)//直接插入
			{
				write << dataType << endl << isLeaf << endl << type << endl
					<< parent << endl << stringRecord.size() << endl;
				for (i = 0; i < stringRecord.size(); i++) //###
					write << stringRecord[i] << endl << recordIDs[i].pID << " " << recordIDs[i].sID << endl; //###
				write.close();
			}
			else //分裂
			{
				write << dataType << endl << isLeaf << endl << type << endl
					<< parent << endl << Order / 2 << endl;
				for (i = 0; i < Order / 2; i++)
					write << stringRecord[i] << endl << recordIDs[i].pID << " " << recordIDs[i].sID << endl; //###
				write.close();
				string newLeaf(leaf);
				filterDigit(newLeaf);
				string key1 = stringRecord[i];
				string key2 = stringRecord[i + 1];
				string key3 = stringRecord[i + 2];
				splitLeaf(newLeaf, dataType, parent, key1, key2, key3, recordIDs[i], recordIDs[i + 1], recordIDs[i + 2]);
				insertBranch(parent, newLeaf, key1);
			}
		}
	}
}

void insertBranch(const string &branch, const string &child, const string &key)
{
	ifstream read;
	string dataType;
	bool isLeaf;
	int type;
	string parent;
	int size;
	vector<int> intKey;
	vector<float> floatKey;
	vector<string> stringKey;
	vector<string> pointer;
	string temp;
	int i, location;

	read.open(branch, ios_base::in);
	read >> dataType >> isLeaf >> type >> parent >> size;

	if (dataType == "int")
	{
		for (i = 0; i < size; i++)
		{
			read >> temp;  //@@@
			pointer.push_back(temp);
			read >> temp;  //@@@
			intKey.push_back(translate<int>(temp));
		}
		read >> temp;
		pointer.push_back(temp);
		read.close();
		intKey.push_back(translate<int>(key));
		sort(intKey.begin(), intKey.end(), Ascend<int>());
		if (intKey.size() <= MAX_CNT) //有空位
		{
			ofstream write;
			write.open(branch, ios::trunc);
			write.close();
			write.open(branch, ios_base::app);
			write << dataType << endl << isLeaf << endl << type << endl
				<< parent << endl << intKey.size() << endl;
			write << pointer[0] << endl;
			vector<int>::iterator it = find(intKey.begin(), intKey.end(), translate<int>(key));
			location = distance(intKey.begin(), it);
			for (i = 0; i < location; i++)
				write << intKey[i] << endl << pointer[i + 1] << endl;
			write << intKey[location] << endl << child << endl;
			for (i = location + 1; i < intKey.size(); i++)
				write << intKey[i] << endl << pointer[i] << endl;
			write.close();
			return;
		}
		else //继续分裂
		{
			vector<int>::iterator it = find(intKey.begin(), intKey.end(), translate<int>(key));
			location = distance(intKey.begin(), it); //find the location of key
			pointer.insert(pointer.begin() + 1 + location, child);
			ofstream write;
			write.open(branch, ios::trunc);
			write.close();
			write.open(branch, ios_base::app);

			if (type == 0) //root split
			{
				write << dataType << endl << isLeaf << endl << 1 << endl
					<< branch << endl << Order / 2 << endl;
				write << pointer[0] << endl;
				for (i = 0; i < Order / 2; i++)
				{
					write << intKey[i] << endl << pointer[i + 1] << endl;
				}
				write.close();
				string newBranch1(branch);
				filterDigit(newBranch1);
				string newBranch2(branch);
				filterDigit(newBranch2);
				string key1 = toString<int>(intKey[i + 1]);
				string key2 = toString<int>(intKey[i + 2]);
				splitBranch(newBranch2, dataType, branch, key1, key2, pointer[i + 1], pointer[i + 2], pointer[i + 3]);
				rename(branch.c_str(), newBranch1.c_str());
				for (i = Order / 2 + 1; i <= Order; i++)
					changeParent(pointer[i], newBranch2);
				for (i = 0; i <= Order / 2; i++)
					changeParent(pointer[i], newBranch1);
				newRoot(branch, dataType, toString<int>(intKey[Order / 2]), newBranch1, newBranch2);
			}
			else
			{
				write << dataType << endl << isLeaf << endl << type << endl
					<< parent << endl << Order / 2 << endl;
				write << pointer[0] << endl;
				for (i = 0; i < Order / 2; i++)
				{
					write << intKey[i] << endl << pointer[i + 1] << endl;
				}
				write.close();
				string newBranch(branch);
				filterDigit(newBranch);
				string key1 = toString<int>(intKey[i + 1]);
				string key2 = toString<int>(intKey[i + 2]);
				splitBranch(newBranch, dataType, parent, key1, key2, pointer[i + 1], pointer[i + 2], pointer[i + 3]);
				changeParent(pointer[i + 1], newBranch);
				changeParent(pointer[i + 2], newBranch);
				changeParent(pointer[i + 3], newBranch);
				insertBranch(parent, newBranch, toString<int>(intKey[Order / 2]));
			}
		}
	}
	else if (dataType == "float")
	{
		for (i = 0; i < size; i++)
		{
			read >> temp;
			pointer.push_back(temp);
			read >> temp;
			floatKey.push_back(translate<float>(temp));
		}
		read >> temp; //@@@
		pointer.push_back(temp);
		read.close();
		floatKey.push_back(translate<float>(key));
		sort(floatKey.begin(), floatKey.end(), Ascend<float>());
		if (floatKey.size() <= MAX_CNT) //有空位
		{
			ofstream write;
			write.open(branch, ios::trunc);
			write.close();
			write.open(branch, ios_base::app);
			write << dataType << endl << isLeaf << endl << type << endl
				<< parent << endl << floatKey.size() << endl;
			write << pointer[0] << endl;
			vector<float>::iterator it = find(floatKey.begin(), floatKey.end(), translate<float>(key));
			location = distance(floatKey.begin(), it);
			for (i = 0; i < location; i++)
				write << floatKey[i] << endl << pointer[i + 1] << endl;
			write << floatKey[location] << endl << child << endl;
			for (i = location + 1; i < floatKey.size(); i++)
				write << floatKey[i] << endl << pointer[i] << endl;
			write.close();
			return;
		}
		else //继续分裂
		{
			vector<float>::iterator it = find(floatKey.begin(), floatKey.end(), translate<float>(key));
			location = distance(floatKey.begin(), it); //find the location of key
			pointer.insert(pointer.begin() + 1 + location, child);
			ofstream write;
			write.open(branch, ios::trunc);
			write.close();
			write.open(branch, ios_base::app);

			if (type == 0) //root split
			{
				write << dataType << endl << isLeaf << endl << 1 << endl
					<< branch << endl << Order / 2 << endl;
				write << pointer[0] << endl;
				for (i = 0; i < Order / 2; i++)
				{
					write << floatKey[i] << endl << pointer[i + 1] << endl;
				}
				write.close();
				string newBranch1(branch);
				filterDigit(newBranch1);
				string newBranch2(branch);
				filterDigit(newBranch2);
				string key1 = toString<float>(floatKey[i + 1]);
				string key2 = toString<float>(floatKey[i + 2]);
				splitBranch(newBranch2, dataType, branch, key1, key2, pointer[i + 1], pointer[i + 2], pointer[i + 3]);
				rename(branch.c_str(), newBranch1.c_str());
				for (i = Order / 2 + 1; i <= Order; i++)
					changeParent(pointer[i], newBranch2);
				for (i = 0; i <= Order / 2; i++)
					changeParent(pointer[i], newBranch1);
				newRoot(branch, dataType, toString<float>(floatKey[Order / 2]), newBranch1, newBranch2);
			}
			else
			{
				write << dataType << endl << isLeaf << endl << type << endl
					<< parent << endl << Order / 2 << endl;
				write << pointer[0] << endl;
				for (i = 0; i < Order / 2; i++)
				{
					write << floatKey[i] << endl << pointer[i + 1] << endl;
				}
				write.close();
				string newBranch(branch);
				filterDigit(newBranch);
				string key1 = toString<float>(floatKey[i + 1]);
				string key2 = toString<float>(floatKey[i + 2]);
				splitBranch(newBranch, dataType, parent, key1, key2, pointer[i + 1], pointer[i + 2], pointer[i + 3]);
				changeParent(pointer[i + 1], newBranch);
				changeParent(pointer[i + 2], newBranch);
				changeParent(pointer[i + 3], newBranch);
				insertBranch(parent, newBranch, toString<float>(floatKey[Order / 2]));
			}
		}
	}
	else if (dataType == "string")
	{
		for (i = 0; i < size; i++)
		{
			read >> temp;  //@@@
			pointer.push_back(temp);
			read >> temp;  //@@@
			stringKey.push_back(temp);
		}
		read >> temp;  //@@@
		pointer.push_back(temp);
		read.close();
		stringKey.push_back(key);
		sort(stringKey.begin(), stringKey.end(), Ascend<string>());
		if (stringKey.size() <= MAX_CNT) //有空位
		{
			ofstream write;
			write.open(branch, ios::trunc);
			write.close();
			write.open(branch, ios_base::app);
			write << dataType << endl << isLeaf << endl << type << endl
				<< parent << endl << stringKey.size() << endl;
			write << pointer[0] << endl;
			vector<string>::iterator it = find(stringKey.begin(), stringKey.end(), key);
			location = distance(stringKey.begin(), it);
			for (i = 0; i < location; i++)
				write << stringKey[i] << endl << pointer[i + 1] << endl;
			write << stringKey[location] << endl << child << endl;
			for (i = location + 1; i < stringKey.size(); i++)
				write << stringKey[i] << endl << pointer[i] << endl;
			write.close();
			return;
		}
		else //继续分裂
		{
			vector<string>::iterator it = find(stringKey.begin(), stringKey.end(), key);
			location = distance(stringKey.begin(), it); //find the location of key
			pointer.insert(pointer.begin() + 1 + location, child);
			ofstream write;
			write.open(branch, ios::trunc);
			write.close();
			write.open(branch, ios_base::app);

			if (type == 0) //root split
			{
				write << dataType << endl << isLeaf << endl << 1 << endl
					<< branch << endl << Order / 2 << endl;
				write << pointer[0] << endl;
				for (i = 0; i < Order / 2; i++)
				{
					write << stringKey[i] << endl << pointer[i + 1] << endl;
				}
				write.close();
				string newBranch1(branch);
				filterDigit(newBranch1);
				string newBranch2(branch);
				filterDigit(newBranch2);
				string key1 = stringKey[i + 1];
				string key2 = stringKey[i + 2];
				splitBranch(newBranch2, dataType, branch, key1, key2, pointer[i + 1], pointer[i + 2], pointer[i + 3]);
				rename(branch.c_str(), newBranch1.c_str());
				for (i = Order / 2 + 1; i <= Order; i++)
					changeParent(pointer[i], newBranch2);
				for (i = 0; i <= Order / 2; i++)
					changeParent(pointer[i], newBranch1);
				newRoot(branch, dataType, stringKey[Order / 2], newBranch1, newBranch2);
			}
			else
			{
				write << dataType << endl << isLeaf << endl << type << endl
					<< parent << endl << Order / 2 << endl;
				write << pointer[0] << endl;
				for (i = 0; i < Order / 2; i++)
				{
					write << stringKey[i] << endl << pointer[i + 1] << endl;
				}
				write.close();
				string newBranch(branch);
				filterDigit(newBranch);
				string key1 = stringKey[i + 1];
				string key2 = stringKey[i + 2];
				splitBranch(newBranch, dataType, parent, key1, key2, pointer[i + 1], pointer[i + 2], pointer[i + 3]);
				changeParent(pointer[i + 1], newBranch);
				changeParent(pointer[i + 2], newBranch);
				changeParent(pointer[i + 3], newBranch);
				insertBranch(parent, newBranch, stringKey[Order / 2]);
			}
		}
	}
}

//以上是insert, 从此开始delete
//string leaf = selectLeafFile(root, key); 
//find the leaf file which will be updated

//删去孩子结点的键后替换父亲结点的键
void overWriteKey(const string &file, const string &oldKey, const string &newKey)
{
	ifstream read;
	ofstream write;
	string dataType, parent;
	bool isLeaf;
	int type, size;
	string tempKey, tempPointer;
	vector<string> keys, pointers;

	read.open(file, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size;
	read >> tempPointer;
	pointers.push_back(tempPointer);
	for (int i = 0; i < size; i++)
	{
		read >> tempKey >> tempPointer;
		if (tempKey == oldKey)
			tempKey = newKey;
		keys.push_back(tempKey);
		pointers.push_back(tempPointer);
	}
	read.close();

	write.open(file, ios::trunc);
	write.close();
	write.open(file, ios::app);
	write << dataType << endl << isLeaf << endl << type << endl
		<< parent << endl << size << endl << pointers[0] << endl;
	for (int i = 0; i < size; i++)
		write << keys[i] << endl << pointers[i + 1] << endl;
	write.close();
}

void overWriteKey(const string &file, const string &newKey)
{
	ifstream read;
	ofstream write;
	string dataType;
	bool isLeaf;
	int type;
	string parent, nextParent;
	int size;
	vector<string> keys, pointers;
	string tempKey, tempPointer;

	read.open(file, ios::in);
	read >> dataType >> isLeaf >> type >> nextParent >> size;
	read.close();
	if (type == 1 || type == 2)
	{
		read.open(nextParent, ios::in);
		read >> dataType >> isLeaf >> type >> parent >> size;
		read >> tempPointer;
		pointers.push_back(tempPointer);
		for (int i = 0; i < size; i++)
		{
			read >> tempKey >> tempPointer; //@@@
			keys.push_back(tempKey);
			pointers.push_back(tempPointer);
		}
		read.close();
		if (file == pointers[0])
			overWriteKey(nextParent, newKey);
		else
		{
			for (unsigned int i = 1; i < pointers.size(); i++)
			{
				if (file == pointers[i])
					keys[i - 1] = newKey;
			}
			write.open(nextParent, ios::trunc);
			write.close();
			write.open(nextParent, ios::app);
			write << dataType << endl << isLeaf << endl << type << endl
				<< parent << endl << size << endl;
			write << pointers[0] << endl;
			for (int i = 0; i < size; i++)
				write << keys[i] << endl << pointers[i + 1] << endl;
			write.close();
		}
	}
}

//find the right brother of the file
string rightBrother(const string &file)
{
	ifstream read;
	string dataType, parent;
	bool isLeaf;
	int type, size;
	vector<string> pointers;
	string tempKey, tempPointer;

	read.open(file, ios::in);
	read >> dataType >> isLeaf >> type >> parent;
	read.close();

	read.open(parent, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size;
	read >> tempPointer;
	pointers.push_back(tempPointer);
	for (int i = 0; i < size; i++)
	{
		read >> tempKey >> tempPointer;
		pointers.push_back(tempPointer);
	}
	read.close();
	vector<string>::iterator it = find(pointers.begin(), pointers.end(), file);
	if (it != pointers.end() && it + 1 != pointers.end())
		return *(it + 1);
	else
		return "null";
}

//find the left brother of the file
string leftBrother(const string &file)
{
	ifstream read;
	string dataType, parent;
	bool isLeaf;
	int type, size;
	vector<string> pointers;
	string tempKey, tempPointer;
	string result;

	read.open(file, ios::in);
	read >> dataType >> isLeaf >> type >> parent;
	read.close();

	read.open(parent, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size;
	read >> tempPointer;
	pointers.push_back(tempPointer);
	for (int i = 0; i < size; i++)
	{
		read >> tempKey >> tempPointer;
		pointers.push_back(tempPointer);
	}
	read.close();
	vector<string>::iterator it;
	for (it = pointers.begin(); *(it + 1) != file; it++);
	return *it;
}

//borrow the smallest key or record from the right brother of the given file
void borrowRight(const string &file, const string &record)
{
	ifstream read;
	string dataType, originalParent, rightParent;
	bool isLeaf;
	int type, size;
	vector<string> originalRecords, rightRecords;
	vector<RecordID> originalrecordIDs, rightrecordIDs;
	string tempRecord;
	string borrow;
	int tempPID, tempSID;
	unsigned int k;

	read.open(file, ios::in);
	read >> dataType >> isLeaf >> type >> originalParent >> size;
	for (int i = 0; i < size; i++)
	{
		read >> tempRecord >> tempPID >> tempSID;
		originalRecords.push_back(tempRecord);
		originalrecordIDs.push_back(RecordID(tempPID, tempSID));
	}
	read.close();

	string right = rightBrother(file);
	read.open(right, ios::in);
	read >> dataType >> isLeaf >> type >> rightParent >> size >> borrow >> tempPID >> tempSID;
	read.close();
	originalRecords.push_back(borrow);
	originalrecordIDs.push_back(RecordID(tempPID, tempSID));

	string target = originalRecords[0];
	vector<string>::iterator it = find(originalRecords.begin(), originalRecords.end(), record);
	k = it - originalRecords.begin();
	it = originalRecords.erase(it); //######
	vector<RecordID>::iterator rit = find(originalrecordIDs.begin(), originalrecordIDs.end(), originalrecordIDs[k]);
	rit = originalrecordIDs.erase(rit);
	ofstream write;
	write.open(file, ios::trunc);
	write.close();
	write.open(file, ios::app);
	write << dataType << endl << isLeaf << endl << type << endl
		<< originalParent << endl << originalRecords.size() << endl;
	for (unsigned int j = 0; j < originalRecords.size(); j++)
		write << originalRecords[j] << endl << originalrecordIDs[j].pID << " " << originalrecordIDs[j].sID << endl;
	write.close();
	overWriteKey(originalParent, target, originalRecords[0]); //######

	read.open(right, ios::in);
	read >> dataType >> isLeaf >> type >> rightParent >> size;
	for (int i = 0; i < size; i++)
	{
		read >> tempRecord >> tempPID >> tempSID;
		rightRecords.push_back(tempRecord);
		rightrecordIDs.push_back(RecordID(tempPID, tempSID));
	}
	read.close();
	target = rightRecords[0];
	rightRecords.erase(rightRecords.begin()); //######
	rightrecordIDs.erase(rightrecordIDs.begin());
	write.open(right, ios::trunc);
	write.close();
	write.open(right, ios::app);
	write << dataType << endl << isLeaf << endl << type << endl
		<< rightParent << endl << rightRecords.size() << endl;
	for (unsigned int j = 0; j < rightRecords.size(); j++)
		write << rightRecords[j] << endl << rightrecordIDs[j].pID << " " << rightrecordIDs[j].sID << endl;
	write.close();
	overWriteKey(rightParent, target, rightRecords[0]); //######
}

//borrow the greatest key or record from the left brother of the given file
void borrowLeft(const string &file, const string &record)
{
	ifstream read;
	string dataType, parent;
	bool isLeaf;
	int type, size;
	vector<string> originalKeys, leftKeys;
	vector<RecordID> originalRecordIDs, leftRecordIDs;
	string tempKey;
	string borrow; //target is the element that will replace the first element of file
	int tempPID, tempSID;
	RecordID borrowRecordID;
	unsigned int k;

	read.open(file, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size;

	for (int i = 0; i < size; i++)
	{
		read >> tempKey >> tempPID >> tempSID;
		originalKeys.push_back(tempKey);
		originalRecordIDs.push_back(RecordID(tempPID, tempSID));
	}
	read.close();
	string target = originalKeys[0];

	string left = leftBrother(file);
	read.open(left, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size;
	for (int i = 0; i < size; i++)
	{
		read >> tempKey >> tempPID >> tempSID;
		leftKeys.push_back(tempKey);
		leftRecordIDs.push_back(RecordID(tempPID, tempSID));
	}
	read.close();

	borrow = leftKeys[size - 1];
	borrowRecordID = leftRecordIDs[size - 1];
	vector<string>::iterator it = find(leftKeys.begin(), leftKeys.end(), borrow);
	k = it - leftKeys.begin();
	it = leftKeys.erase(it);
	vector<RecordID>::iterator rit = find(leftRecordIDs.begin(), leftRecordIDs.end(), leftRecordIDs[k]);
	rit = leftRecordIDs.erase(rit);
	it = find(originalKeys.begin(), originalKeys.end(), record);
	k = it - originalKeys.begin();
	it = originalKeys.erase(it);
	rit = find(originalRecordIDs.begin(), originalRecordIDs.end(), originalRecordIDs[k]);
	rit = originalRecordIDs.erase(rit);
	originalKeys.insert(originalKeys.begin(), borrow);
	originalRecordIDs.insert(originalRecordIDs.begin(), borrowRecordID);
	overWriteKey(parent, target, borrow);

	ofstream write;
	write.open(file, ios::trunc);
	write.close();
	write.open(file, ios::app);
	write << dataType << endl << isLeaf << endl << type << endl
		<< parent << endl << originalKeys.size() << endl;
	for (unsigned int i = 0; i < originalKeys.size(); i++)
		write << originalKeys[i] << endl << originalRecordIDs[i].pID << " " << originalRecordIDs[i].sID << endl;
	write.close();

	write.open(left, ios::trunc);
	write.close();
	write.open(left, ios::app);
	write << dataType << endl << isLeaf << endl << type << endl
		<< parent << endl << leftKeys.size() << endl;
	for (unsigned int i = 0; i < leftKeys.size(); i++)
		write << leftKeys[i] << endl << leftRecordIDs[i].pID << " " << leftRecordIDs[i].sID << endl;
	write.close();
}

void removePointer(const string &file, const string &key, const string &pointer)
{
	string dataType;
	bool isLeaf;
	int type;
	string parent;
	int size;
	string tempKey, tempPointer;
	vector<string> keys, pointers;
	ifstream read;
	ofstream write;
	int i;

	read.open(file, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size >> tempPointer;
	pointers.push_back(tempPointer);
	for (i = 0; i < size; i++)
	{
		read >> tempKey >> tempPointer; //@@@
		keys.push_back(tempKey);
		pointers.push_back(tempPointer);
	}
	read.close();

	vector<string>::iterator it = find(keys.begin(), keys.end(), key);
	it = keys.erase(it);
	it = find(pointers.begin(), pointers.end(), pointer);
	it = pointers.erase(it);

	write.open(file, ios::trunc);
	write.close();
	write.open(file, ios::app);
	write << dataType << endl << isLeaf << endl << type << endl
		<< parent << endl << keys.size() << endl;
	write << pointers[0] << endl;
	for (unsigned i = 0; i < keys.size(); i++)
		write << keys[i] << endl << pointers[i + 1] << endl;
	write.close();
}

//get the key index of the child file
int keyIndex(const string &file)
{
	ifstream read;
	string dataType;
	bool isLeaf;
	int type;
	string parent;
	int size;
	int i;
	string tempPointer, tempKey;
	vector<string> pointers;

	read.open(file, ios::in);
	read >> dataType >> isLeaf >> type >> parent;
	read.close();

	read.open(parent, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size;
	read >> tempPointer;
	pointers.push_back(tempPointer);

	for (i = 0; i < size; i++)
	{
		read >> tempKey >> tempPointer; //@@@
		pointers.push_back(tempPointer);
	}
	read.close();

	for (i = 0; i < pointers.size(); i++)
		if (file == pointers[i])
			break;
	if (i == 0 || i == 1)
		return 0;
	else
		return i - 1;
}

//for leaf ###
void joinRight(const string &file, const string &record)
{
	string dataType;
	bool isLeaf;
	int type;
	string parent, nextParent;
	int size, rightSize;
	string tempKeys;
	string target;
	vector<string> originalRecords, rightRecords, keys;
	vector<RecordID> originalRecordIDs, rightRecordIDs;
	int tempPID, tempSID;
	unsigned int k;
	ifstream read;
	ofstream write;

	read.open(file, ios::in);
	read >> dataType >> isLeaf >> type >> nextParent >> size;
	for (int i = 0; i < size; i++)
	{
		read >> tempKeys >> tempPID >> tempSID; //@@@
		originalRecords.push_back(tempKeys);
		originalRecordIDs.push_back(RecordID(tempPID, tempSID));
	}
	read.close();
	vector<string>::iterator it = find(originalRecords.begin(), originalRecords.end(), record);
	k = it - originalRecords.begin();
	it = originalRecords.erase(it);
	vector<RecordID>::iterator rit = find(originalRecordIDs.begin(), originalRecordIDs.end(), originalRecordIDs[k]);
	rit = originalRecordIDs.erase(rit);

	string right = rightBrother(file);
	read.open(right, ios::in);
	read >> dataType >> isLeaf >> type >> nextParent >> rightSize;
	for (int i = 0; i < rightSize; i++)
	{
		read >> tempKeys >> tempPID >> tempSID; //@@@
		rightRecords.push_back(tempKeys);
		rightRecordIDs.push_back(RecordID(tempPID, tempSID));
	}
	read.close();

	target = rightRecords[0];
	for (int i = originalRecords.size() - 1; i >= 0; i--)
	{
		rightRecords.insert(rightRecords.begin(), originalRecords[i]);
		rightRecordIDs.insert(rightRecordIDs.begin(), originalRecordIDs[i]);
	}
	overWriteKey(nextParent, target, rightRecords[0]);

	write.open(right, ios::trunc);
	write.close();
	write.open(right, ios::app);
	write << dataType << endl << isLeaf << endl << type << endl
		<< nextParent << endl << rightRecords.size() << endl;
	for (unsigned int i = 0; i < rightRecords.size(); i++)
		write << rightRecords[i] << endl << rightRecordIDs[i].pID << " " << rightRecordIDs[i].sID << endl;
	write.close();

	read.open(nextParent, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size >> tempKeys; //@@@
	for (int i = 0; i < size; i++)
	{
		read >> tempKeys; //@@@
		keys.push_back(tempKeys);
		read >> tempKeys; //@@@
	}
	read.close();
	//delete the key and pointer in the parent file of the given file
	int index = keyIndex(file);
	remove(file.c_str());
	deleteInter(nextParent, keys[index], file);
}

void joinLeft(const string &file, const string &record)
{
	string dataType;
	bool isLeaf;
	int type;
	string parent, Nparent;
	int size, leftSize;
	string tempKeys;
	vector<string> originalRecords, leftRecords, keys;
	vector<RecordID> originalRecordIDs, leftRecordIDs;
	int tempPID, tempSID;
	unsigned int k;
	ifstream read;
	ofstream write;

	read.open(file, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size;
	for (int i = 0; i < size; i++)
	{
		read >> tempKeys >> tempPID >> tempSID; //@@@
		originalRecords.push_back(tempKeys);
		originalRecordIDs.push_back(RecordID(tempPID, tempSID));
	}
	read.close();
	vector<string>::iterator it = find(originalRecords.begin(), originalRecords.end(), record);
	k = it - originalRecords.begin();
	it = originalRecords.erase(it);
	vector<RecordID>::iterator rit = find(originalRecordIDs.begin(), originalRecordIDs.end(), originalRecordIDs[k]);
	originalRecordIDs.erase(rit);

	string left = leftBrother(file);
	read.open(left, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> leftSize;
	for (int i = 0; i < leftSize; i++)
	{
		read >> tempKeys >> tempPID >> tempSID; //@@@
		leftRecords.push_back(tempKeys);
		leftRecordIDs.push_back(RecordID(tempPID, tempSID));
	}
	read.close();

	for (int i = 0; i < originalRecords.size(); i++)
	{
		leftRecords.push_back(originalRecords[i]);
		leftRecordIDs.push_back(originalRecordIDs[i]);
	}

	write.open(left, ios::trunc);
	write.close();
	write.open(left, ios::app);
	write << dataType << endl << isLeaf << endl << type << endl
		<< parent << endl << leftRecords.size() << endl;
	for (unsigned int i = 0; i < leftRecords.size(); i++)
		write << leftRecords[i] << endl << leftRecordIDs[i].pID << " " << leftRecordIDs[i].sID << endl;
	write.close();

	read.open(parent, ios::in);
	read >> dataType >> isLeaf >> type >> Nparent >> size >> tempKeys;
	for (int i = 0; i < size; i++)
	{
		read >> tempKeys; //@@@
		keys.push_back(tempKeys);
		read >> tempKeys;  //@@@
	}
	read.close();

	int index = keyIndex(file);
	remove(file.c_str());
	deleteInter(parent, keys[index], file); //######
}

//return the minimum record of the file tree.
string minKey(const string &file) //######
{
	ifstream read;
	string dataType;
	bool isLeaf;
	int type;
	string parent;
	int size;
	string child;

	read.open(file, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size;
	read >> child; //@@@
	read.close();
	if (isLeaf)
		return child;
	else
		return minKey(child);
}

void joinRightInter(const string &file, const string &key, const string &pointer)
{
	ifstream read;
	ofstream write;
	string dataType;
	bool isLeaf;
	int type;
	string parent;
	int size;
	vector<string> keys, pointers, rightKeys, rightPointers;
	string tempKey, tempPointer;

	read.open(file, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size;
	read >> tempPointer;
	pointers.push_back(tempPointer);
	for (int i = 0; i < size; i++)
	{
		read >> tempKey >> tempPointer; //@@@
		keys.push_back(tempKey);
		pointers.push_back(tempPointer);
	}
	read.close();
	vector<string>::iterator it = find(keys.begin(), keys.end(), key);
	it = keys.erase(it);
	it = find(pointers.begin(), pointers.end(), pointer);
	it = pointers.erase(it);

	string right = rightBrother(file);
	read.open(right, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size;
	read >> tempPointer;
	rightPointers.push_back(tempPointer);
	for (int i = 0; i < size; i++)
	{
		read >> tempKey >> tempPointer; //@@@
		rightKeys.push_back(tempKey);
		rightPointers.push_back(tempPointer);
	}
	read.close();

	string label = minKey(right);
	string label2 = minKey(file);
	rightKeys.insert(rightKeys.begin(), label);
	for (int i = keys.size() - 1; i >= 0; i--)
		rightKeys.insert(rightKeys.begin(), keys[i]);
	for (int i = pointers.size() - 1; i >= 0; i--)
	{
		rightPointers.insert(rightPointers.begin(), pointers[i]);
		changeParent(pointers[i], right);
	}
	write.open(right, ios::trunc);
	write.close();
	write.open(right, ios::app);
	write << dataType << endl << isLeaf << endl << type << endl << parent << endl
		<< rightKeys.size() << endl;
	write << rightPointers[0] << endl;
	for (unsigned int i = 0; i < rightKeys.size(); i++)
		write << rightKeys[i] << endl << rightPointers[i + 1] << endl;
	write.close();

	int index = keyIndex(file);
	remove(file.c_str());
	overWriteKey(parent, label, label2);

	string parentParent;
	vector<string> parentKeys;
	read.open(parent, ios::in);
	read >> dataType >> isLeaf >> type >> parentParent >> size;
	read >> tempPointer;
	for (int i = 0; i < size; i++)
	{
		read >> tempKey >> tempPointer;
		parentKeys.push_back(tempKey);
	}
	deleteInter(parent, parentKeys[index], file);
}

void joinLeftInter(const string &file, const string &key, const string &pointer)
{
	ifstream read;
	ofstream write;
	string dataType;
	bool isLeaf;
	int type;
	string parent;
	int size;
	vector<string> keys, pointers, leftKeys, leftPointers;
	string tempKey, tempPointer;

	read.open(file, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size;
	read >> tempPointer;
	pointers.push_back(tempPointer);
	for (int i = 0; i < size; i++)
	{
		read >> tempKey >> tempPointer; //@@@
		keys.push_back(tempKey);
		pointers.push_back(tempPointer);
	}
	read.close();
	vector<string>::iterator it = find(keys.begin(), keys.end(), key);
	it = keys.erase(it);
	it = find(pointers.begin(), pointers.end(), pointer);
	it = pointers.erase(it);

	string left = leftBrother(file);
	read.open(left, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size;
	read >> tempPointer;
	leftPointers.push_back(tempPointer);
	for (int i = 0; i < size; i++)
	{
		read >> tempKey >> tempPointer; //@@@
		leftKeys.push_back(tempKey);
		leftPointers.push_back(tempPointer);
	}
	read.close();

	string label = minKey(file);
	leftKeys.push_back(label);
	for (unsigned int i = 0; i < keys.size(); i++)
		leftKeys.push_back(keys[i]);
	for (unsigned int i = 0; i < pointers.size(); i++)
	{
		leftPointers.push_back(pointers[i]);
		changeParent(pointers[i], left);
	}

	write.open(left, ios::trunc);
	write.close();
	write.open(left, ios::app);
	write << dataType << endl << isLeaf << endl << type << endl << parent << endl
		<< leftKeys.size() << endl;
	write << leftPointers[0] << endl;
	for (unsigned int i = 0; i < leftKeys.size(); i++)
		write << leftKeys[i] << endl << leftPointers[i + 1] << endl;
	write.close();

	remove(file.c_str());
	deleteInter(parent, label, file);
}

void borrowRightInter(const string &file, const string &key, const string &pointer)
{
	ifstream read;
	ofstream write;
	string dataType;
	bool isLeaf;
	int type;
	string parent;
	int size;
	vector<string> keys, pointers, rightKeys, rightPointers;
	string tempKey, tempPointer;

	read.open(file, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size;
	read >> tempPointer;
	pointers.push_back(tempPointer);
	for (int i = 0; i < size; i++)
	{
		read >> tempKey >> tempPointer; //@@@
		keys.push_back(tempKey);
		pointers.push_back(tempPointer);
	}
	read.close();
	vector<string>::iterator it = find(keys.begin(), keys.end(), key);
	it = keys.erase(it);
	it = find(pointers.begin(), pointers.end(), pointer);
	it = pointers.erase(it);

	string right = rightBrother(file);
	read.open(right, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size;
	read >> tempPointer;
	rightPointers.push_back(tempPointer);
	for (int i = 0; i < size; i++)
	{
		read >> tempKey >> tempPointer; //@@@
		rightKeys.push_back(tempKey);
		rightPointers.push_back(tempPointer);
	}
	read.close();

	string label = minKey(right);
	keys.push_back(label);
	pointers.push_back(rightPointers[0]);
	it = rightKeys.erase(rightKeys.begin());
	it = rightPointers.erase(rightPointers.begin());

	write.open(file, ios::trunc);
	write.close();
	write.open(file, ios::app);
	write << dataType << endl << isLeaf << endl << type << endl
		<< parent << endl << keys.size() << endl;
	write << pointers[0] << endl;
	for (unsigned int i = 0; i < keys.size(); i++)
		write << keys[i] << endl << pointers[i + 1] << endl;
	write.close();

	write.open(right, ios::trunc);
	write.close();
	write.open(right, ios::app);
	write << dataType << endl << isLeaf << endl << type << endl
		<< parent << endl << rightKeys.size() << endl;
	write << rightPointers[0] << endl;
	for (unsigned int i = 0; i < rightKeys.size(); i++)
		write << rightKeys[i] << endl << rightPointers[i + 1] << endl;
	write.close();

	changeParent(pointers[pointers.size() - 1], file);
	overWriteKey(parent, label, minKey(right));
}

void borrowLeftInter(const string &file, const string &key, const string &pointer)
{
	ifstream read;
	ofstream write;
	string dataType;
	bool isLeaf;
	int type;
	string parent;
	int size;
	vector<string> keys, pointers, leftKeys, leftPointers;
	string tempKey, tempPointer;

	read.open(file, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size;
	read >> tempPointer;
	pointers.push_back(tempPointer);
	for (int i = 0; i < size; i++)
	{
		read >> tempKey >> tempPointer; //@@@
		keys.push_back(tempKey);
		pointers.push_back(tempPointer);
	}
	read.close();
	vector<string>::iterator it = find(keys.begin(), keys.end(), key);
	it = keys.erase(it);
	it = find(pointers.begin(), pointers.end(), pointer);
	it = pointers.erase(it);

	string left = leftBrother(file);
	read.open(left, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size;
	read >> tempPointer;
	leftPointers.push_back(tempPointer);
	for (int i = 0; i < size; i++)
	{
		read >> tempKey >> tempPointer; //@@@
		leftKeys.push_back(tempKey);
		leftPointers.push_back(tempPointer);
	}

	string label = minKey(file);
	keys.insert(keys.begin(), label);
	pointers.insert(pointers.begin(), leftPointers[leftPointers.size() - 1]);
	leftKeys.pop_back();
	leftPointers.pop_back();

	write.open(file, ios::trunc);
	write.close();
	write.open(file, ios::app);
	write << dataType << endl << isLeaf << endl << type << endl
		<< parent << endl << keys.size() << endl;
	write << pointers[0] << endl;
	for (unsigned int i = 0; i < keys.size(); i++)
		write << keys[i] << endl << pointers[i + 1] << endl;
	write.close();

	write.open(left, ios::trunc);
	write.close();
	write.open(left, ios::app);
	write << dataType << endl << isLeaf << endl << type << endl
		<< parent << endl << leftKeys.size() << endl;
	write << leftPointers[0] << endl;
	for (unsigned int i = 0; i < leftKeys.size(); i++)
		write << leftKeys[i] << endl << leftPointers[i + 1] << endl;
	write.close();

	changeParent(pointers[0], file);
	overWriteKey(parent, label, minKey(file));
}

bool haveRightBrother(const string &file)
{
	ifstream read;
	string dataType;
	bool isLeaf;
	int type;
	string parent;
	int size;
	int i;
	string tempRecord, tempPointer;
	vector<string> pointers;

	read.open(file, ios::in);
	read >> dataType >> isLeaf >> type >> parent;
	read.close();

	read.open(parent, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size;
	read >> tempPointer;
	pointers.push_back(tempPointer);
	for (i = 0; i < size; i++)
	{
		read >> tempRecord >> tempPointer; //@@@
		pointers.push_back(tempPointer);
	}
	read.close();

	vector<string>::iterator it = find(pointers.begin(), pointers.end(), file);
	if (it + 1 != pointers.end())
		return true;
	else
		return false;
}

void deleteInter(const string &file, const string &key, const string &pointer)
{
	ifstream read;
	ofstream write;
	string dataType;
	bool isLeaf;
	int type;
	string parent;
	int size;
	vector<string> keys, pointers;
	string tempKey, tempPointer;

	read.open(file, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size;
	read >> tempPointer;
	pointers.push_back(tempPointer);
	for (int i = 0; i < size; i++)
	{
		read >> tempKey >> tempPointer; //@@@
		keys.push_back(tempKey);
		pointers.push_back(tempPointer);
	}
	read.close();

	if (size >= Order / 2 && type == 1 || size > 1 && type == 0) //delete directly
	{
		vector<string>::iterator it = find(keys.begin(), keys.end(), key);
		it = keys.erase(it);
		it = find(pointers.begin(), pointers.end(), pointer);
		it = pointers.erase(it);
		write.open(file, ios::trunc);
		write.close();
		write.open(file, ios::app);
		write << dataType << endl << isLeaf << endl << type << endl
			<< parent << endl << keys.size() << endl;
		write << pointers[0] << endl;
		for (unsigned int i = 0; i < keys.size(); i++)
			write << keys[i] << endl << pointers[i + 1] << endl;
		write.close();
	}
	else
	{
		if (type == 1)
		{
			if (haveRightBrother(file))
			{
				string right = rightBrother(file);
				read.open(right, ios::in);
				read >> dataType >> isLeaf >> type >> parent >> size;
				read.close();
				if (size >= Order / 2) //borrow
				{
					borrowRightInter(file, key, pointer);
				}
				else //join
				{
					joinRightInter(file, key, pointer);
				}
			}
			else
			{
				string left = leftBrother(file);
				read.open(left, ios::in);
				read >> dataType >> isLeaf >> type >> parent >> size;
				read.close();
				if (size >= Order / 2) //borrow
				{
					borrowLeftInter(file, key, pointer);
				}
				else //join
				{
					joinLeftInter(file, key, pointer);
				}
			}
		}
		else if (type == 0) //root
		{
			string childDataType;
			bool childIsLeaf;
			int childType;
			string childParent;
			int childSize;
			vector<string> childKeys, childPointers;

			remove(file.c_str());
			if (pointers[0] == pointer)
			{
				read.open(pointers[1], ios::in);
				read >> childDataType >> childIsLeaf >> childType >> childParent >> childSize;
				read >> tempPointer;
				childPointers.push_back(tempPointer);
				for (int i = 0; i < childSize; i++)
				{
					read >> tempKey >> tempPointer; //@@@
					childKeys.push_back(tempKey);
					childPointers.push_back(tempPointer);
				}
				read.close();
				write.open(pointers[1], ios::trunc);
				write.close();
				write.open(pointers[1], ios::app);
				write << childDataType << endl << 0 << endl << 0 << endl
					<< 0 << endl << childKeys.size() << endl;
				write << childPointers[0] << endl;
				for (unsigned int i = 0; i < childKeys.size(); i++)
					write << childKeys[i] << endl << childPointers[i + 1] << endl;
				write.close();
				rename(pointers[1].c_str(), file.c_str());
			}
			else
			{
				read.open(pointers[0], ios::in);
				read >> childDataType >> childIsLeaf >> childType >> childParent >> childSize;
				read >> tempPointer;
				childPointers.push_back(tempPointer);
				for (int i = 0; i < childSize; i++)
				{
					read >> tempKey >> tempPointer; //@@@
					childKeys.push_back(tempKey);
					childPointers.push_back(tempPointer);
				}
				read.close();
				write.open(pointers[0], ios::trunc);
				write.close();
				write.open(pointers[0], ios::app);
				write << childDataType << endl << 0 << endl << 0 << endl
					<< 0 << endl << childKeys.size() << endl;
				write << childPointers[0] << endl;
				for (unsigned int i = 0; i < childKeys.size(); i++)
					write << childKeys[i] << endl << childPointers[i + 1] << endl;
				write.close();
				rename(pointers[0].c_str(), file.c_str());
			}
		}
	}
}

void deleteRecord(const string &node, const string &record)
{
	ifstream read;
	ofstream write;
	string dataType;
	bool isLeaf;
	int type;
	string parent;
	int size;
	int i;
	string tempRecord, tempPointer;
	vector<string> records, pointers;
	vector<RecordID> recordIDs;
	int tempPID, tempSID;
	unsigned int k;

	read.open(node, ios::in);
	read >> dataType >> isLeaf >> type >> parent >> size;

	for (i = 0; i < size; i++)
	{
		read >> tempRecord >> tempPID >> tempSID; //@@@
		records.push_back(tempRecord);
		recordIDs.push_back(RecordID(tempPID, tempSID));
	}
	read.close();
	vector<string>::iterator it = find(records.begin(), records.end(), record);
	if (it != records.end())
	{
		if (size >= Order / 2) //half full
		{
			string target = records[0]; //######
			it = find(records.begin(), records.end(), record);
			k = it - records.begin();
			if (it == records.end())
				throw RecordNotFound();
			it = records.erase(it);
			vector<RecordID>::iterator rit = find(recordIDs.begin(), recordIDs.end(), recordIDs[k]);
			rit = recordIDs.erase(rit);
			overWriteKey(node, records[0]); //######
			write.open(node, ios::trunc);
			write.close();
			write.open(node, ios::app);
			write << dataType << endl << isLeaf << endl << type << endl
				<< parent << endl << records.size() << endl;
			for (unsigned i = 0; i < records.size(); i++)
				write << records[i] << endl << recordIDs[i].pID << " " << recordIDs[i].sID << endl;
			write.close();
		}
		else //not half full
		{
			if (haveRightBrother(node))
			{
				string right = rightBrother(node);
				read.open(right, ios::in);
				read >> dataType >> isLeaf >> type >> parent >> size;
				read.close();
				if (size >= Order / 2) //右兄弟达到半满，借记录
				{
					borrowRight(node, record);
				}
				else //右兄弟不到半满，合并，删父节点对应key和pointer
				{
					joinRight(node, record);
				}
			}
			else
			{
				string left = leftBrother(node);
				read.open(left, ios::in);
				read >> dataType >> isLeaf >> type >> parent >> size;
				read.close();
				if (size >= Order / 2)
				{
					borrowLeft(node, record);
				}
				else
				{
					joinLeft(node, record);
				}
			}
		}
	}
	else
	{
		throw RecordNotFound();
	}
}

RecordID select(const string &root, const string &key)
{
	ifstream read;
	string dataType;
	bool isLeaf;
	int type;
	string parent;
	int size;
	int i;
	string tempKey;
	PageID pid;
	SlotID sid;
	string leaf = selectLeafFile(root, key);
	read.open(root, ios::in);
	read >> dataType >> isLeaf;
	read.close();
	if (isLeaf)
	{
		return RecordID(0, 0);
	}
	
	else
	{
		read.open(leaf, ios::in);
		read >> dataType >> isLeaf >> type >> parent >> size;
		for (i = 0; i < size; i++)
		{
			read >> tempKey >> pid >> sid;
			if (tempKey == key)
				break;
		}
		read.close();
		if (i < size)
			return RecordID(pid, sid);
		else
			return RecordID(0, 0);
	}
}