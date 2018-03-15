//
// To store global declarations.
// Please Do Not include any other file in this header file.
//

#ifndef MINISQL_GLOBALDECLAR_H
#define MINISQL_GLOBALDECLAR_H

#include <cstdint>
using namespace std;

typedef uint32_t PageID;  // Page ID and Slot ID are begin from 1
typedef uint16_t SlotID;
typedef uint32_t FrameID;  // Frame ID is begin from 0

struct ATTRIBUTE
{
	string attr_name;
	string type;
	int offset;
	int size;
	bool unique;
	bool index_true;

	ATTRIBUTE():unique(false), index_true(false){}
	~ATTRIBUTE(){}
};

struct TABLE
{
	string table_name;
	int num_attr;
	int primary_key_index;
	ATTRIBUTE attribute[33];

	TABLE():num_attr(0), primary_key_index(0){}
	~TABLE(){}
};

struct condition {
	ATTRIBUTE attr;
	string opt;
	string opd;
};

#define CATALOG_FILE_PATH 	"catalog\\"
#define INDEX_FILE_PATH	 	"index\\"
#define RECORD_FILE_PATH 	"record\\"

#define NOTHING      0
#define SIZE_OF_PAGE 8192          // 8KB
#define SIZE_OF_DATA 8192-sizeof(BM_Page_Header)
#define BUFNUM 		 100

#endif //MINISQL_GLOBALDECLAR_H
