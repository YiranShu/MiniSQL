#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_
#include <string>
#include "BM_Page.h"
#include "GlobalDeclar.h"
using namespace std;

template<class T> T translate(const string &content);
template<class T> string toString(T content);
void filterDigit(string &content);
void findRootName(string &file);
string createFileName(const string &rootName);
void createIndexFile();
void createIndex(const string &indexName, const string &rootName, const string &tableName, const string &attributeName, const string &type);
void createLeaf(const string &fileName, const string &dataType, const string &parent);
void createLeaf(const string &fileName, const string &dataType, const string &parent, const string &key, const RecordID &recordID);
void createRoot(const string &rootName, const string &key, const RecordID &recordID);
string selectLeafFile(const string &root, const string &key);
void splitLeaf(const string &file, const string &dataType, const string &parent, const string &key1, const string &key2, const string &key3, const RecordID &recordID1, const RecordID &recordID2, const RecordID &recordID3);
void splitBranch(const string &file, const string &dataType, const string &parent, const string &key1, const string &key2, const string &child1, const string &child2, const string &child3);
void newRoot(const string &file, const string &dataType, const string &key, const string &branch1, const string &branch2);
void changeParent(const string &file, const string &newParent);
void insert(const string &leaf, const string &key, const RecordID &recordID);
void insertBranch(const string &branch, const string &child, const string &key);
void overWriteKey(const string &file, const string &oldKey, const string &newKey);
void overWriteKey(const string &file, const string &newKey);
string rightBrother(const string &file);
string leftBrother(const string &file);
void borrowRight(const string &file, const string &record);
void borrowLeft(const string &file, const string &record);
void removePointer(const string &file, const string &key, const string &pointer);
void joinRight(const string &file, const string &record);
void joinLeft(const string &file, const string &record);
string minKey(const string &file);
void joinRightInter(const string &file, const string &key, const string &pointer);
void joinLeftInter(const string &file, const string &key, const string &pointer);
void borrowRightInter(const string &file, const string &key, const string &pointer);
void borrowLeftInter(const string &file, const string &key, const string &pointer);
bool haveRightBrother(const string &file);
void deleteInter(const string &file, const string &key, const string &pointer);
void deleteRecord(const string &node, const string &record);
RecordID select(const string &root, const string &key);
#endif