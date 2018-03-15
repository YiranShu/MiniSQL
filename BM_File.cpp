#include "BM_File.h"
#include "BM_File_iterator.h"

map<string, int> openCount;
map<string, shared_ptr<fstream> > openStream;


BM_File_iterator BM_File::begin()
{
    BM_File_iterator iter(this);
    return iter;
}

BM_File_iterator BM_File::end()
{
    BM_File_iterator iter(this, NOTHING);
    return iter;
}

BM_File::BM_File(const string &fname){
    filename = fname;

    if(IsExist(fname))
    {
        if(IsOpen(fname))        // the file is already open
        {
            openCount[fname]++;
            filestream = openStream[fname];
            fHdr = ReadHdr();
        }
        else OpenFile();   // open this file
    }
    else CreateFile();       // create this file and open it
}

void BM_File::OpenFile() {
    ios_base::openmode mode = ios_base::in | ios_base::out | ios_base::binary;
    filestream.reset(new fstream(filename, mode));
    fHdr = ReadHdr();

    openCount[filename] = 1;
    openStream[filename] = filestream;
}

void BM_File::CreateFile() {
    ios_base::openmode mode = ios_base::in | ios_base::out | ios_base::binary | ios_base::trunc;
    filestream.reset(new fstream(filename, mode));
    fHdr.pageNum = NOTHING;
    fHdr.freeNum = NOTHING;
    fHdr.first_used = NOTHING;
    fHdr.first_free = NOTHING;

    openCount[filename] = 1;
    openStream[filename] = filestream;
}

void BM_File::CloseFile() {
    WriteHdr();
    filestream.reset();
    openCount[filename]--;
    if(openCount[filename] == 0)      // the file will not be used anymore
    {
        openCount.erase(filename);

        openStream[filename].reset();
        openStream.erase(filename);
    }
}

BM_File_Header BM_File::ReadHdr() {
    BM_File_Header fileHdr;
    filestream->seekg(0, ios::beg);
    filestream->read(reinterpret_cast<char*>(&fileHdr), sizeof(BM_File_Header));
    return fileHdr;
}

void BM_File::WriteHdr() {
    filestream->seekp(0, ios::beg);
    filestream->write(reinterpret_cast<char*>(&fHdr), sizeof(BM_File_Header));
    filestream->flush();
}

BM_Page BM_File::ReadPage(const PageID &pID) const{
    if(ValidPageID(pID))
    {
        BM_Page new_page;
        new_page.pHdr = ReadPageHdr(pID);
        filestream->seekg(PageDataPos(pID),ios::beg);
        filestream->read(&new_page.pData[0], SIZE_OF_DATA);

        return new_page;
    }
}

BM_Page_Header BM_File::ReadPageHdr(const PageID& pID) const
{
    if(ValidPageID(pID))
    {
        BM_Page_Header new_pHdr;
        filestream->seekg(PagePos(pID),ios::beg);
        filestream->read(reinterpret_cast<char*>(&new_pHdr), sizeof(BM_Page_Header));

        return new_pHdr;
    }
}

void BM_File::WritePage(BM_Page& new_page)
{
    if(ValidPageID(new_page.GetPageID()))
    {
        WritePageHdr(new_page.pHdr);

        filestream->seekp(PageDataPos(new_page.GetPageID()),ios::beg);
        filestream->write(&(new_page.pData[0]), SIZE_OF_DATA);
        filestream->flush();
    }
}

void BM_File::WritePageHdr(BM_Page_Header& new_pHdr)
{
    if(ValidPageID(new_pHdr.thisPage))
    {
        filestream->seekp(PagePos(new_pHdr.thisPage),ios::beg);
        filestream->write(reinterpret_cast<const char*>(&new_pHdr), sizeof(BM_Page_Header));
        filestream->flush();
    }
}

BM_Page BM_File::AllocatePage() {
    BM_Page new_page;
    BM_Page prev_page;
    // if exists unused page
    if(fHdr.freeNum > NOTHING)
    {
        new_page = ReadPage(fHdr.first_free);       // reuse the unused page

        fHdr.first_free = new_page.GetNextPage();   // adjust the free page list
        fHdr.freeNum--;

        // insert it into the used page list
        if(fHdr.first_used == NOTHING || fHdr.first_used > new_page.GetPageID())
        {
            new_page.SetNextPage(fHdr.first_used);
            fHdr.first_used = new_page.GetPageID();
        }
        else{
            BM_File_iterator iter = begin();
            BM_File_iterator iter_end = end();
            PageID temp;
            while(iter != iter_end)
            {
                temp = (*iter).GetNextPage();
                if(temp > new_page.GetPageID() || temp == NOTHING) {
                    prev_page = *iter;
                    break;
                }
                ++iter;
            }
            new_page.SetNextPage(temp);
            prev_page.SetNextPage(new_page.GetPageID());
        }
    }
    else{  // else create a new page
        new_page.SetPageID(++fHdr.pageNum);

        // insert into the used page list
        if(fHdr.first_used == NOTHING)
            fHdr.first_used = new_page.GetPageID();
        else{
            BM_File_iterator iter = begin();
            BM_File_iterator iter_end = end();
            PageID temp;
            while(iter != iter_end)
            {
                temp = (*iter).GetNextPage();
                if(temp == NOTHING) {
                    prev_page = *iter;
                    break;
                }
                ++iter;
            }
            new_page.SetNextPage(temp);
            prev_page.SetNextPage(new_page.GetPageID());
        }
    }

    if(prev_page.GetPageID() != NOTHING)
        WritePage(prev_page);
    WritePage(new_page);
    return new_page;
}



void BM_File::CleanPage(const PageID& pID)
{
    if(ValidPageID(pID)){
        BM_Page target_page = ReadPage(pID);
        BM_Page prev_page;

        // remove it from the used paper list
        if(pID == fHdr.first_used)
            fHdr.first_used = target_page.GetNextPage();
        else{
            BM_File_iterator iter = begin();
            while((*iter).GetNextPage() != pID)
                ++iter;

            prev_page = *iter;
            prev_page.SetNextPage(target_page.GetNextPage());
        }

        target_page.Clean();

        // add it to the free page list
        target_page.SetNextPage(fHdr.first_free);
        fHdr.first_free = target_page.GetPageID();
        fHdr.freeNum++;

        WritePage(target_page);
        if(prev_page.GetPageID() != NOTHING)
            WritePage(prev_page);
    }
}