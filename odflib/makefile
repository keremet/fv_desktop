APP=agr2odt
OBJ= odt_content_editor.o num2word.o $(APP).o
CXXFLAGS=-O3 -Ilibzip-0.10.1_bin/include -Ilibzip-0.10.1_bin/lib/libzip/include
LDFLAGS=

all:$(APP)
$(APP): libzip-0.10.1_bin/lib/libzip.a $(OBJ)
	g++ -o$(APP) $(OBJ) libzip-0.10.1_bin/lib/libzip.a -lz

odt_content_editor.o: odt_content_editor.cpp odt_content_editor.h
	g++ -c -o$@ $(CXXFLAGS) odt_content_editor.cpp
	
num2word.o: num2word.cpp num2word.h
	g++ -c -o$@ $(CXXFLAGS) num2word.cpp
	
$(APP).o: $(APP).cpp odt_content_editor.h
	g++ -c -o$@ $(CXXFLAGS) $(APP).cpp

libzip-0.10.1_bin/lib/libzip.a:
	tar xf libzip-0.10.1.tar.bz2
	cd libzip-0.10.1/ && patch -p0 < ../zip_close_into_new_file.diff && (CFLAGS=-O3 ./configure --prefix=`pwd`/../libzip-0.10.1_bin ) && make && make install
	
clean:
	rm -f $(APP) $(OBJ)
	rm -fr libzip-0.10.1_bin
	rm -fr libzip-0.10.1

run:
	./$(APP)
