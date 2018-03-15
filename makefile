CC = g++

TARG = main
OBJS = BufferManager.o BM_File.o BM_Page.o BM_Hash.o CatalogManager.o Result.o RecordManager.o indexManager.o main.o

$(TARG): $(OBJS)
	$(CC) -o  $@ $(OBJS)
BufferManager.o: BufferManager.cpp BufferManager.h
	$(CC) -c -std=c++11 BufferManager.cpp
BM_File.o: BM_File.cpp BM_File.h
	$(CC) -c -std=c++11 BM_File.cpp
BM_Page.o: BM_Page.cpp BM_Page.h
	$(CC) -c -std=c++11 BM_Page.cpp
BM_Hash.o:BM_Hash.cpp BM_Hash.h
	$(CC) -c -std=c++11 BM_Hash.cpp
CatalogManager.o: CatalogManager.cpp CatalogManager.h
	$(CC) -c -std=c++11 CatalogManager.cpp	
Result.o: Result.cpp Result.h
	$(CC) -c -std=c++11 Result.cpp	
RecordManager.o: RecordManager.cpp RecordManager.h
	$(CC) -c -std=c++11 RecordManager.cpp
indexManager.o: indexManager.cpp Functions.h
	$(CC) -c -std=c++11 indexManager.cpp	
main.o: main.cpp
	$(CC) -c -std=c++11 main.cpp
clean:
	rm -f $(OBJS)