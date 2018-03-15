#include "BufferManager.h"
#include <iostream>

BufferManager::BufferManager(FrameID frameNum):bufNum(frameNum) {
    bufDescTable = new BM_BufDesc[frameNum];
    for(FrameID i = 0; i < frameNum; ++i)
    {
        bufDescTable[i].thisFrame = i;
        bufDescTable[i].valid = false;
    }

    int hashTableSize = (int)(frameNum * 1.5)+1;
    hashTable = new BM_Hash(hashTableSize);

    bufPool = new BM_Page[frameNum];

    clkptr = frameNum-1;
}

void BufferManager::AllocBuf(FrameID &fID) {
    FrameID scanCnt = 0;
    bool found = false;
    BM_BufDesc* temp_desc;
    while(scanCnt < 2*bufNum)   // travel through the buffer pool twice
    {
        AdvanceClock();
        scanCnt++;
        temp_desc = &bufDescTable[clkptr];
        if(!temp_desc->valid)  // reuse the invalid frame if any
        {
            found = true;
            break;
        }

        if(!temp_desc->refbit) {
            if (temp_desc->pinCnt == 0) {    // else free the unpinned and hasn't being visited recently frame
                hashTable->RemovePage(temp_desc->thisFile, temp_desc->thisPage);
                found = true;
                break;
            }
        }
        else{
            temp_desc->refbit = false;  // set the refbit false so that it may be reused next time
            bufStats.accessNum++;     // update the access number
        }
    }

    if(found) {
        if (temp_desc->dirty) {   // if the old page is dirty, write back
            bufStats.diskWriteNum++;
            temp_desc->thisFile->WritePage(bufPool[clkptr]);
        }

        temp_desc->Clean();

        fID = clkptr;   // return the clkptr through fID
    }

    // deal with exception
}

void BufferManager::ReadPage(BM_File *file, const PageID pID, BM_Page* &page) {
    FrameID fID;
    if(hashTable->LookUpPage(file, pID, fID))      // if it's already read in
    {
        bufDescTable[fID].refbit = true;
        bufDescTable[fID].pinCnt++;
        page = &bufPool[fID];                    // return the pointer
    }
    else{   // otherwise, read it into the buffer pool
        AllocBuf(fID);
        bufStats.diskReadNum++;
        bufPool[fID] = file->ReadPage(pID);
        bufDescTable[fID].Set(file, pID);
        hashTable->InsertPage(file, pID, fID);

        page = &bufPool[fID];       // return the pointer
    }

}

void BufferManager::unPinPage(BM_File *file, const PageID pID, const bool dirty) {
    FrameID fID;
    if(hashTable->LookUpPage(file, pID, fID))
    {
        bufDescTable[fID].pinCnt--;

        bufDescTable[fID].dirty = dirty;       // set dirty bit according to the dirty signal
    }

    // deal with exception
}

void BufferManager::AllocPage(BM_File *file, PageID &pID, BM_Page* &page) {
    FrameID fID;
    AllocBuf(fID);
    bufPool[fID] = file->AllocatePage();
    pID = bufPool[fID].GetPageID();   // return page id and page pointer
    page = &bufPool[fID];

    bufDescTable[fID].Set(file, pID);
    hashTable->InsertPage(file, pID, fID);

}

void BufferManager::DeletePage(BM_File *file, const PageID pID) {
    FrameID fID;
    if(hashTable->LookUpPage(file, pID, fID))  // get the corresponding frame ID
    {
        bufDescTable[fID].Clean();
        hashTable->RemovePage(file, pID);
        file->CleanPage(pID);
    }
    else{
        file->CleanPage(pID);
    }
    // deal with exception
}

void BufferManager::FlushFile(const BM_File *file) {
    BM_BufDesc* temp;
    for(FrameID i =0; i < bufNum; ++i)
    {
        temp = &bufDescTable[i];
        if(temp->thisFile != file || !temp->valid)
            continue;

        if(temp->pinCnt == 0) {
            if (temp->dirty) {
                (temp->thisFile)->WritePage(bufPool[temp->thisFrame]);  // if it's dirty, write back
                temp->dirty = false;
            }

            hashTable->RemovePage(file, temp->thisPage);
            temp->Clean();
        }

        // deal with exception
    }
}

void BufferManager::FlushPage(const BM_File *file, const PageID &pID) {
    FrameID fID;
    if(hashTable->LookUpPage(file, pID, fID))
    {
        if(bufDescTable[fID].pinCnt == 0)
        {
            if(bufDescTable[fID].dirty) {
                bufDescTable[fID].thisFile->WritePage(bufPool[fID]);
                bufDescTable[fID].dirty = false;
            }
            hashTable->RemovePage(file, pID);
            bufDescTable[fID].Clean();
        }
    }
}

BufferManager::~BufferManager() {
    BM_BufDesc* temp;
    for(FrameID i =0; i < bufNum; ++i)
    {
        temp = &bufDescTable[i];
        if(temp->valid && temp->dirty)
        {
            temp->thisFile->WritePage( bufPool[(temp->thisFrame)] );  // if the page is valid and dirty, write back
        }
    }
    delete[] bufPool;
    delete[] bufDescTable;
    delete hashTable;
}

void BufferManager::PrintDesc() {
    BM_BufDesc* temp;
    int validFrames = 0;

    for (FrameID i = 0; i < bufNum; i++)
    {
        temp = &(bufDescTable[i]);
        cout << "FrameNo:" << i << " ";
        temp->Print();

        if (temp->valid)
            validFrames++;
    }
    cout << "Total Number of Valid Frames:" << validFrames << endl;
}

void BufferManager::PrintBufStats() {
    cout << "Current Buffer Status:" << " ";
    cout << "AccessNum:" << bufStats.accessNum <<" ";
    cout << "Read from Disk: " << bufStats.diskReadNum << " ";
    cout << "Write to Disk:" << bufStats.diskWriteNum << endl;
}
