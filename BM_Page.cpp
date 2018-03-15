#include "BM_Page.h"
#include <iostream>
using namespace std;
BM_Page::BM_Page() {
    pHdr.thisPage = NOTHING;
    pHdr.nextPage = NOTHING;

    Clean();
}

BM_Slot_Header* BM_Page::GetSlotHdr(const SlotID& sID)
{
    void* temp = static_cast<void*> (&pData[(sID-1)* sizeof(BM_Slot_Header)]);
    return static_cast<BM_Slot_Header*> (temp);
}

string BM_Page::GetRecord(const RecordID& rID)
{
    if(ValidRecID(rID))
    {
        BM_Slot_Header* sHdr = GetSlotHdr(rID.sID);
        return pData.substr(sHdr->offset, sHdr->size);
    }
    else{
        string s = "invalid record";
        return s;
    }
    // TODO: Exception
}

void BM_Page::InsertRecord(const std::string& record, RecordID& rID)
{
    BM_Slot_Header* new_slot;

    // if have unused slot then reuse it
    if(pHdr.freeSlotNum > NOTHING && StillHaveSpace(record.length()))
    {
        for(SlotID i = 1; i <= pHdr.slotNum; i++)
        {
            new_slot = GetSlotHdr(i);
            if(new_slot->size == NOTHING)
            {
                AdjustOtherRecord(i, new_slot, record.length());
                rID.pID = pHdr.thisPage;
                rID.sID = i;
                break;
            }
        }
        pHdr.freeSlotNum--;

        WriteToSolt(new_slot, record);

    }
    // else alloc a new slot
    else if(pHdr.freeSlotNum == NOTHING && StillHaveSpace(sizeof(BM_Slot_Header)+ record.length()))
    {
        new_slot = GetSlotHdr(++pHdr.slotNum);
        pHdr.begin_of_free += sizeof(new_slot);
        pHdr.end_of_free -= record.length();
        new_slot->size = (SlotID) record.length();
        new_slot->offset = pHdr.end_of_free;
        WriteToSolt(new_slot, record);

        rID.pID = pHdr.thisPage;
        rID.sID = pHdr.slotNum;
    }

}


void BM_Page::InsertRecord(const std::string& record)
{
    BM_Slot_Header* new_slot;

    // if have unused slot then reuse it
    if(pHdr.freeSlotNum > NOTHING && StillHaveSpace(record.length()))
    {
        for(SlotID i = 1; i <= pHdr.slotNum; i++)
        {
            new_slot = GetSlotHdr(i);
            if(new_slot->size == NOTHING)
            {
                AdjustOtherRecord(i, new_slot, record.length());
                break;
            }
        }
        pHdr.freeSlotNum--;

        WriteToSolt(new_slot, record);

    }
        // else alloc a new slot
    else if(pHdr.freeSlotNum == NOTHING && StillHaveSpace(sizeof(BM_Slot_Header)+ record.length()))
    {
        new_slot = GetSlotHdr(++pHdr.slotNum);
        pHdr.begin_of_free += sizeof(new_slot);
        pHdr.end_of_free -= record.length();
        new_slot->size = (SlotID) record.length();
        new_slot->offset = pHdr.end_of_free;
        WriteToSolt(new_slot, record);
    }

}


void BM_Page::DeletRecord(const RecordID& rID)
{
    if(ValidRecID(rID))
    {
        BM_Slot_Header* target_slot = GetSlotHdr(rID.sID);
        AdjustOtherRecord(rID.sID, target_slot, NOTHING);

        pHdr.freeSlotNum++;              // add it to the unused slot list
    }
}

void BM_Page::UpdateRecord(const RecordID& rID, const std::string& record)
{
    if(ValidRecID(rID))
    {
        BM_Slot_Header* target_slot = GetSlotHdr(rID.sID);
        if( (record.length() > target_slot->size && StillHaveSpace(sizeof(target_slot)+ record.length()-target_slot->size))
                || record.length() <= target_slot->size )
        {
            AdjustOtherRecord(rID.sID, target_slot, record.length());
            pData.replace(target_slot->offset, target_slot->size, record);
        }
    }
}

void BM_Page::AdjustOtherRecord(const SlotID&  sID, BM_Slot_Header*& target_slot, const std::size_t& reclength)
{
    SlotID  difference;
    string temp;
    BM_Slot_Header* other_slot;

    if((SlotID)reclength > target_slot->size) {              // the new record is larger
        difference = (SlotID)reclength - target_slot->size;

        pHdr.end_of_free -= difference;
        for (SlotID i = pHdr.slotNum; i > sID; i--) {
            other_slot = GetSlotHdr(i);
            if(other_slot->size == NOTHING)         // if the slot is unused
            {
                other_slot->offset -= difference;    // just adjust the offset
                continue;
            }

            temp = pData.substr(other_slot->offset, other_slot->size);
            other_slot->offset -= difference;     // adjust the offset of the slot
            pData.replace(other_slot->offset, other_slot->size, temp);
        }
        target_slot->offset -= difference;
        target_slot->size = (SlotID)reclength;
    }
    else if((SlotID)reclength < target_slot->size)         // the new record is smaller
    {
        difference = target_slot->size - (SlotID)reclength;

        target_slot->offset += difference;
        target_slot->size = (SlotID)reclength;
        for(SlotID i = (SlotID)(sID+1); i <= pHdr.slotNum; i++) {
            other_slot = GetSlotHdr(i);
            if(other_slot->size == NOTHING) {        // if the slot is unused
                other_slot->offset += difference;   // adjust the offset of the slot
                continue;
            }

            temp = pData.substr(other_slot->offset, other_slot->size);
            other_slot->offset += difference;   // adjust the offset of the slot
            pData.replace(other_slot->offset, other_slot->size, temp);
        }
        pData.replace(pHdr.end_of_free, difference, difference, '\0');
        pHdr.end_of_free += difference;
    }
}


void BM_Page::Clean() {
    pHdr.slotNum = NOTHING;
    pHdr.freeSlotNum = NOTHING;
    pHdr.begin_of_free = sizeof(pHdr);
    pHdr.end_of_free = SIZE_OF_DATA;
    pData.resize(SIZE_OF_DATA, '\0');
}