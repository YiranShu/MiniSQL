//
// Created by 朱曼青 on 2017/6/3.
//

#ifndef MINISQL_BM_FILE_ITERATOR_H
#define MINISQL_BM_FILE_ITERATOR_H

#include <assert.h>
#include "BM_File.h"

class BM_File_iterator{
    friend class BM_File;
public:
    BM_Page operator*()
    {
        assert(thisfile != nullptr);
        return thisfile->ReadPage(thisPage);
    }

    BM_File_iterator& operator++()
    {
        assert(thisfile != nullptr);
        thisPage = (thisfile->ReadPageHdr(thisPage)).nextPage;
        return *this;
    }
    bool operator==(const BM_File_iterator& other) const
    {
        assert(thisfile != nullptr);
        return (thisfile->GetFileName() == other.thisfile->GetFileName() ) && (thisPage == other.thisPage);
    }
    bool operator!=(const BM_File_iterator& other) const
    {
        return !(*this == other);
    }


    BM_File_iterator():thisfile(nullptr), thisPage(NOTHING){ }

    BM_File_iterator(BM_File* file):thisfile(file){
        assert(thisfile != nullptr);
        thisPage = thisfile->GetFirstUsed();
    }

    BM_File_iterator(BM_File* file, PageID pID):thisfile(file), thisPage(pID){ }
private:
    BM_File *thisfile;
    PageID thisPage;
};


#endif //MINISQL_BM_FILE_ITERATOR_H
