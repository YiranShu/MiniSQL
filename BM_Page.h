/*** the structure of a PAGE ***
// Page_Header
// Slot_Header_1
// Slot_Header_2
// ......
// Slot_Header_n
//
//  (some free space in between)
//
// Record_1
// Record_2
// ......
// Record_n (end of a PAGE)
// **********************************
the length of a record is variable  **/

#ifndef MINISQL_BM_PAGE_H
#define MINISQL_BM_PAGE_H

#include <string>
#include "GlobalDeclar.h"

/**
 * RecordID consists of ID for page and ID for file
 **/
struct RecordID{
    friend class BM_Page;
public:
    RecordID(): pID(1), sID(1) {}
    RecordID(PageID pid, SlotID sid): pID(pid), sID(sid) {}
    bool operator== (const RecordID& other) const
    {
         return (pID == other.pID) && (sID == other.sID);
    }

    bool operator!= (const RecordID& other) const
    {
        return !(*this == other);
    }

public:
    PageID pID;
    SlotID sID;   // slot id is from 1 to slotNum
};

/** metadata of page **/
struct BM_Page_Header{
    PageID thisPage;            // id of this page
    PageID nextPage;            // id of next page
    SlotID slotNum;             // number of slots in this page, some may be unused because of record deletion
    SlotID freeSlotNum;        // number of unused slots
    SlotID begin_of_free;       // the begin of free space after the last slot header
    SlotID end_of_free;         // the end of free space before the first record
};

/** metadata of slot **/
struct BM_Slot_Header{
    SlotID offset;               // position of the slot
    SlotID size;                 // size of the slot
};

class BM_Page{
    friend class BM_File;
    friend class BM_File_iterator;
public:
/**
 * constructor， copy constructor and destructor
 **/
    BM_Page();
    BM_Page& operator=( const BM_Page& other)
    {
        pHdr = other.pHdr;
        pData = other.pData;

        return *this;
    }
    ~BM_Page(){ };

/**
 * getter and setter
 **/
    void SetPageID(const PageID& pID){
        pHdr.thisPage = pID;
    }

    void SetNextPage(const PageID& pID){
        pHdr.nextPage = pID;
    }

    const PageID GetPageID() const
    {
        return pHdr.thisPage;
    }

    const PageID GetNextPage() const
    {
        return pHdr.nextPage;
    }

/**
 * input:
 * ouput:
 *
 * Clean() will clean the content of the page, including page header,
 * slot headers and slots.
 * But the Page id and its next page won't change.
**/
    void Clean();


/**
 * input: slot id
 * output: pointer to the slot
 *
 * GetSlotHdr() will return the pointer to the slot according the
 * given slot id.
**/
    BM_Slot_Header* GetSlotHdr(const SlotID& sID);


/**
 * input: record id
 * output: string
 *
 * GetRecord() will return the content of record according the
 * given record id.
**/
    std::string GetRecord(const RecordID& rID);


/**
 * input: string
 * output: record id
 *
 * InsertRecord() will receive a string and insert it into the
 * page as a new record and return its record id
**/
    void InsertRecord(const std::string& record, RecordID& rID);
	void InsertRecord(const std::string& record);

/**
 * input: record id
 * output:
 *
 * DeletRecord() will delete the record according to the given id.
 * Its slot header will be reserved so other slots won't be affected.
**/
    void DeletRecord(const RecordID& rID);


/**
 * input: record id, string
 * output:
 *
 * UpdateRecord() will replace the record with the given string
 * according to the given id.
**/
    void UpdateRecord(const RecordID& rID, const std::string& record);

/**
 * input: the size you want to use in the page
 * output:
 *
 * StillHaveSpace() will tell whether there is enough space for your operation.
 * Please use it before insert a record and update a record
**/
    bool StillHaveSpace(std::size_t something) const{
        return ((int)(pHdr.end_of_free - pHdr.begin_of_free - something)) >= 0;
    }
  
private:
    bool ValidSlotID(const SlotID& sID ) const{
        return sID <= pHdr.slotNum && sID > NOTHING;
    }

    bool ValidRecID(const RecordID& rID) const{
        return (rID.pID == pHdr.thisPage) && ValidSlotID(rID.sID);
    }
    void AdjustOtherRecord(const SlotID&  sID, BM_Slot_Header*& target_slot, const std::size_t& reclength);
    void WriteToSolt(BM_Slot_Header*& slot, const std::string& record)
    {
        pData.replace(slot->offset, slot->size, record);
    }

public:
    BM_Page_Header pHdr;   //page header
private:
    std::string pData;    // slot header + slot
};


#endif //MINISQL_BM_PAGE_H
