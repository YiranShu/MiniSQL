// use clock algorithm
// BufferManager() should be a Singleton class !!!
// the BufferManager() only cares about three things:
//  1) BufDesc, which records the status of each frame
//  2) hash table, which represents the map between frame and page
//  3) bufPool, the array storing pages which is read into memory

#ifndef MINISQL_BUFFERMANAGER_H
#define MINISQL_BUFFERMANAGER_H

#include "BM_BufDesc.h"

/** the Status log of the Buffer **/
struct BM_BufStats{
    BM_BufStats()
    {
        Clean();
    }

    void Clean()
    {
        accessNum = NOTHING;
        diskReadNum = NOTHING;
        diskWriteNum = NOTHING;
    }

    int accessNum;   // the number of accessing to the buffer pool
    int diskReadNum;  // the number of reading from disk
    int diskWriteNum;  // the number of writing to disk
};

class BufferManager{
public:
/**
 * constructor and destructor
 **/
    BufferManager(FrameID frameNum);
    ~BufferManager();

/**
 * input: the file pointer, page id
 * ouput: page pointer
 *
 * ReadPage() will return the page pointer according the given file pointer and page id.
 * Then, you can use the public function of BM_Page class to operate on the content of the page.
 *
 * This function will increase the pinCnt of the page automatically. Please remember to unpin it.
 *
**/
    void ReadPage(BM_File* file, const PageID pID, BM_Page* &page);

/**
 * input: the file pointer, page id, dirty signal
 * ouput:
 *
 * unPinPage() will decrease the pinCnt of the target page.
 * This function should be called EVERY TIME you finish the operations on the page pointer,which you get through ReadPage(),
 * and you don't need this pointer anymore. Otherwise, the page will stay in the memory FOREVER.
 *
 * the dirty bit should be MANUALLY set as true, if the content of the page is changed by you.
**/
    void unPinPage(BM_File* file, const PageID pID, const bool dirty);

/**
 * input: the file pointer
 * ouput: page id, page pointer
 *
 * AllocPage() will allocate a new page for the target file and return its page id and pointer.
 *
 * This function will set the pinCnt of the page automatically. Please remember to unpin it.
**/
    void AllocPage(BM_File* file, PageID& pID, BM_Page* &page);

/**
 * input: the file pointer
 * ouput:
 *
 * FlushFile() will flush out all pages in buffer of the given file.
 *
**/
    void FlushFile(const BM_File* file);
  
/**
 * input: the file pointer, page id
 * ouput:
 *
 * FlushFile() will flush out the given page.
 *
**/
    void FlushPage(const BM_File *file, const PageID& pID);

/**
 * input: the file pointer, page id
 * ouput:
 *
 * DeletePage() will clean the content of the target page, its page id will be reversed.
**/
    void DeletePage(BM_File* file, const PageID pID);

    void CleanBufStats()
    {
        bufStats.Clean();
    }

/**
 * function for test
 **/
    void PrintDesc();
    void PrintBufStats();

public:
    BM_Page* bufPool;   // the array storing the page which is read in, every entity is the copy of the page in disk

private:
    void AdvanceClock()
    {
        clkptr = (clkptr+1) % bufNum;
    }
    void AllocBuf(FrameID& fID);

private:
    FrameID clkptr;   // the clock hand pointer
    FrameID bufNum;   // the number of frames in the buffer pool
    BM_Hash* hashTable; // the pointer to hash table class
    BM_BufDesc* bufDescTable; // the array of buffer describe class
    BM_BufStats bufStats; // the Status log
};


#endif //MINISQL_BUFFERMANAGER_H
