// some information about each frame in the buffer pool

#ifndef MINISQL_BM_BUFDESC_H
#define MINISQL_BM_BUFDESC_H

#include "BM_Hash.h"
#include <iostream>

class BM_BufDesc {
    friend class BufferManager;
private:
/**
 * constructor
 **/
    BM_BufDesc(){
        Clean();
    }
/**
 * input: the file pointer, page id
 * ouput:
 *
 * Set() is used to initialize the bufDesc for a new page which is read into the memory
**/
    void Set(BM_File* file, PageID pID)
    {
        thisFile = file;
        thisPage = pID;
        pinCnt = 1;
        dirty = false;
        refbit = true;
        valid = true;
    }

    void Clean()
    {
        thisFile = nullptr;
        thisPage = NOTHING;
        pinCnt = NOTHING;
        dirty = false;
        refbit = false;
        valid = false;
    }

/**
 * function for test
 **/
    void Print()
    {
        if(thisPage)
        {
            cout << "file:" << thisFile->GetFileName() << " ";
            cout << "pageID:" << thisPage << " ";
        }
        else cout << "file:NULL"<< " ";
        cout << "pinCnt:"<< pinCnt << " ";
        cout << "valid:"<< valid << " ";
        cout << "dirty:" << dirty << " ";
        cout << "refbit:" << refbit << endl;
    }

private:
    FrameID thisFrame;
    BM_File* thisFile;
    PageID thisPage;
    int pinCnt;   // the number of the page being pinned
    bool dirty;   // is true when the pager in the memory is inconsistent with the page in the disk
    bool refbit;  // is true when the pager is visited by the clkptr recently
    bool valid;   // used by lazy deletion
};


#endif //MINISQL_BM_BUFDESC_H
