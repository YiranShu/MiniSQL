// the HashTable is to figure out which frame contains the target page quickly
// use separate chaining with linked list to handle with collision

#ifndef MINISQL_BM_HASH_H
#define MINISQL_BM_HASH_H

#include "BM_File.h"

/**
 * the actual node of hash bucket chain
 **/
typedef struct BM_Bucket *hashBucket;
struct BM_Bucket{
    BM_File* file;
    PageID pID;
    FrameID fID;
    hashBucket next;
};

class BM_Hash {
public:
/**
 * constructor, copy constructor and destructor
 **/
    BM_Hash(const int Hsize);
    BM_Hash& operator=(const BM_Hash& other)
    {
        TableSize = other.TableSize;
        ht = other.ht;

        return *this;
    }
    ~BM_Hash();

/**
 * input: the file pointer, page id, frame id
 * ouput:
 *
 * InsertPage() will build the connection between the given page and frame
**/
    void InsertPage(const BM_File* file, const PageID& pID, const FrameID& fID);

/**
 * input: the file pointer, page id
 * ouput:
 *
 * RemovePage() will delete the hash bucket which includes the given page
**/
    void RemovePage(const BM_File* file, const PageID& pID);

/**
 * input: the file pointer, page id, frame id
 * ouput: bool
 *
 * LookUpPage() will return true if the page is in the hashtable and vise versa.
**/
    bool LookUpPage(const BM_File* file, const PageID& pID, FrameID& fID);

private:
/**
 * the hash function
 **/
/**
 * input: the file pointer, page id
 * ouput: int
 *
 * hash() is the hash function. It will return the index of a given page,
**/
    int hash(const BM_File* file, const PageID& pID) const;
private:
    hashBucket* ht;  // ht is the two-dimension array of BM_Bucket
    int TableSize;   // the size of hash table
};


#endif //MINISQL_BM_HASH_H
