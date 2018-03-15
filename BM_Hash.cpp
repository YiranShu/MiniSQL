#include "BM_Hash.h"

BM_Hash::BM_Hash(const int Hsize) : TableSize(Hsize) {
    ht = new hashBucket[Hsize];
    for(int i =0; i < Hsize; i++)
        ht[i] = nullptr;
}

BM_Hash::~BM_Hash() {
    for (int i = 0; i < TableSize; i++) {
        while (ht[i]) {
            hashBucket temp = ht[i];
            ht[i] = ht[i]->next;
            delete temp;
        }
    }
    delete[] ht;
}

int BM_Hash::hash(const BM_File *file, const PageID &pID) const {
    return (int)((long long)file + pID) % TableSize;
}

void BM_Hash::InsertPage(const BM_File *file, const PageID &pID, const FrameID &fID) {
    int index = hash(file, pID);
    hashBucket temp = new BM_Bucket;
    temp->file = const_cast<BM_File *>(file);
    temp->pID = pID;
    temp->fID = fID;
    temp->next = ht[index];
    ht[index] = temp;
}

bool BM_Hash::LookUpPage(const BM_File *file, const PageID &pID, FrameID &fID) {
    int index = hash(file, pID);
    hashBucket temp = ht[index];
    while (temp) {
        if (temp->file == file && temp->pID == pID) {
            fID = temp->fID;
            return true;
        }
        else temp = temp->next;
    }
    return false;
}

void BM_Hash::RemovePage(const BM_File *file, const PageID &pID) {
    int index = hash(file, pID);
    hashBucket temp = ht[index];
    hashBucket prev = NULL;
    while (temp) {
        if (temp->file == file && temp->pID == pID)
        {
            if(prev) prev->next = temp->next;
            else ht[index] = temp->next;

            delete temp;
            return;
        }
        else{
            prev = temp;
            temp = temp->next;
        }
    }
    // not-exist exception
}