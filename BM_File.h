//*** the structure of a FILE ***
// File_Header                             --> Used_1 --> Used_2 ...
// PAGE_1                                /
// PAGE_2                             Hdr
//                                       \
// ......                                  --> Free_1 --> Free_2 ...
// PAGE_n (end of a FILE)
// **********************************
// the length of a PAGE is fixed
// automatically read file header when open a file

#ifndef MINISQL_BM_FILE_H
#define MINISQL_BM_FILE_H

#include "BM_Page.h"
#include <string>
#include <fstream>
#include <map>
#include <cstdio>
#include <memory>

extern map<string, int> openCount;   // record the open time of a file
extern map<string, shared_ptr<fstream> > openStream;  // record the fstream of a file


/** metadata of file**/
struct BM_File_Header{
    PageID pageNum;
    PageID freeNum;   // a page may be free due to deletion
    PageID first_used;   // the position of first used page
    PageID first_free;    // the position of first unused page
};

/** iterator of file **/
class BM_File_iterator;   // pointer to pages of the file

class BM_File{
public:
/**
 * constructor, copy constructor and destructor
 **/
    BM_File(const string& fname);

    BM_File(const BM_File& other):filename(other.filename), filestream(other.filestream), fHdr(other.fHdr){ }
    BM_File& operator= (const BM_File& other)
    {
        filename = other.filename;
        filestream = other.filestream;
        fHdr = other.fHdr;

        return *this;
    }

    ~BM_File()
    {
        CloseFile();
    }

/**
 * getter
 **/
    const string& GetFileName() const
    {
        return filename;
    }

    const PageID& GetFirstUsed() const {
        return fHdr.first_used;
    }

/**
 * input: page id
 * ouput: the copy of page header
 *
 * ReadPageHdr() will return the page header according to the given page id.
**/
    BM_Page_Header ReadPageHdr(const PageID& pID) const;

/**
 * input: page header
 * ouput:
 *
 * WritePageHdr() will replace the old page header with the input
**/
    void WritePageHdr(BM_Page_Header& new_pHdr);

/**
 * input: page id
 * ouput: the copy of page
 *
 * ReadPageHdr() will return the page according to the given page id.
**/
    BM_Page ReadPage(const PageID& pID) const;

/**
 * input: page
 * ouput:
 *
 * WritePageHdr() will replace the old page with the input
**/
    void WritePage(BM_Page& new_page);

/**
 * input:
 * ouput: the copy of page
 *
 * AllocatePage() will return a new page that can be used.
**/
    BM_Page AllocatePage();

/**
 * input: page id
 * ouput:
 *
 * CleanPage() will delete the content of the page but reverse it as a free page.
**/
    void CleanPage(const PageID& pID);

    BM_File_iterator begin();
    BM_File_iterator end();

public:
    bool IsExist(const string& fname) const
    {
        fstream temp(fname, ios::in);
        if(temp.is_open())
        {
            temp.close();
            return true;
        }
        else return false;
    }

    bool IsOpen(const string& fname) const
    {
        return (openCount.find(fname) != openCount.end()) ;
    }

    bool ValidPageID(const PageID& pID) const
    {
        return pID > NOTHING && pID <= fHdr.pageNum;
    }

    streampos PagePos(const PageID& pID) const
    {
        return sizeof(fHdr)+(pID - 1)*SIZE_OF_PAGE;
    }

    streampos PageDataPos(const PageID& pID) const
    {
        return sizeof(fHdr)+(pID - 1)*SIZE_OF_PAGE + sizeof(BM_Page_Header);
    }

private:
/**
 * file operation
 **/
    void OpenFile();
    void CreateFile();
    void CloseFile();

/**
 * file header operation
 **/
    BM_File_Header ReadHdr();
    void WriteHdr();

private:
    string filename;
    shared_ptr<fstream> filestream;
    BM_File_Header fHdr;
};

#endif //MINISQL_BM_FILE_H
