#export OPTIMFLAGS=-g -O0
export OPTIMFLAGS=-O3



objects = interface.o transparentMath.o MainWindow.o  AgrWindow.o MembersWindow.o db.o base.o PaymentProcessing.o julianday.o financial.o sqlite-amalgamation-3071602/sqlite3.o odflib/odt_content_editor.o odflib/num2word.o agr2odt.o

APP = lk
export OSTYPE = $(shell uname)

CXXFLAGS = $(OPTIMFLAGS) `pkg-config gtk+-2.0 libglade-2.0 --cflags`

ifeq ($(OSTYPE), SunOS)
export LDFLAGS += `pkg-config gtk+-2.0 libglade-2.0 --libs` 
endif

ifeq ($(OSTYPE), Linux)
export LDFLAGS += `pkg-config gtk+-2.0 libglade-2.0 --libs` 
endif

ifeq ($(OSTYPE), MINGW32_NT-5.1)
LDFLAGS += -Wl,--subsystem,windows -L/z/readyLK -L/usr/local/lib /z/libglade-2.6.3/glade/.libs/libglade-2.0.a -lxml2 `pkg-config gtk+-2.0 --libs`
endif
export CXXFLAGS

ifeq ($(OSTYPE), MINGW32_NT-5.1)
LDFLAGS += -lWs2_32
endif

SQLITE = sqlite-amalgamation-3071602/sqlite3

all:	$(APP) $(SQLITE)

$(APP):odflib/libzip-0.10.1_bin/lib/libzip.a $(objects) 
	g++ -o$@ $(objects) $(LDFLAGS) odflib/libzip-0.10.1_bin/lib/libzip.a -lz

$(SQLITE): sqlite-amalgamation-3071602/sqlite3.o
	gcc -O3 -o$(SQLITE)  -DSQLITE_OMIT_LOAD_EXTENSION sqlite-amalgamation-3071602/shell.c sqlite-amalgamation-3071602/sqlite3.o

odflib/libzip-0.10.1_bin/lib/libzip.a: 
	cd odflib && make

#odflib/odt_content_editor.o: odflib/odt_content_editor.cpp odflib/odt_content_editor.h
#	g++ -c odflib/odt_content_editor.cpp -o$@ $(CXXFLAGS) -Iodflib/libzip-0.10.1_bin/include -Iodflib/libzip-0.10.1_bin/lib/libzip/include
	
#odflib/num2word.o:odflib/num2word.cpp odflib/num2word.h
#	g++ -c odflib/num2word.cpp -o$@ $(CXXFLAGS)

agr2odt.o: agr2odt.cpp agr2odt.h db.h financial.h julianday.h
	g++ -c agr2odt.cpp -o$@ $(CXXFLAGS) -Iodflib/libzip-0.10.1_bin/include -Iodflib/libzip-0.10.1_bin/lib/libzip/include

interface.o: interface.cpp MainWindow.h db.h financial.h
	g++ -c interface.cpp -o$@ $(CXXFLAGS)

MainWindow.o: MainWindow.cpp MainWindow.h AgrWindow.h db.h financial.h PaymentProcessing.h base.h
	g++ -c MainWindow.cpp -o$@ $(CXXFLAGS)

AgrWindow.o: AgrWindow.cpp AgrWindow.h db.h base.h PaymentProcessing.h financial.h
	g++ -c AgrWindow.cpp -o$@ -fpermissive $(CXXFLAGS)

base.o: base.cpp base.h
	g++ -c base.cpp -o$@ $(CXXFLAGS)

db.o: db.cpp db.h financial.h
	g++ -c db.cpp -o$@ $(CXXFLAGS)

MembersWindow.o: MembersWindow.cpp MembersWindow.h	db.h financial.h base.h
	g++ -c MembersWindow.cpp -o$@ $(CXXFLAGS)

PaymentProcessing.o: PaymentProcessing.cpp PaymentProcessing.h db.h  financial.h transparentMath.h
	g++ -c PaymentProcessing.cpp -o$@ $(CXXFLAGS)

transparentMath.o: transparentMath.cpp 	transparentMath.h julianday.h financial.h
	g++ -c transparentMath.cpp -o$@ $(CXXFLAGS)

financial.o: financial.cpp 	financial.h
	g++ -c financial.cpp -o$@ $(CXXFLAGS)

sqlite-amalgamation-3071602/sqlite3.o: sqlite-amalgamation-3071602/sqlite3.c
	gcc -c -O3 sqlite-amalgamation-3071602/sqlite3.c -o$@ -DSQLITE_THREADSAFE=0 -DOMIT_ALTERTABLE -DSQLITE_OMIT_EXPLAIN -DOMIT_LIKE_OPTIMIZATION -DSQLITE_OMIT_TRUNCATE_OPTIMIZATION -DSQLITE_OMIT_DEPRECATED -DSQLITE_OMIT_DATETIME_FUNCS -DSQLITE_OMIT_LOCALTIME -DSQLITE_OMIT_COMPILEOPTION_DIAGS -DSQLITE_OMIT_TEMPDB -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_OMIT_AUTHORIZATION -DSQLITE_OMIT_PROGRESS_CALLBACK  -DSQLITE_OMIT_EXPLAIN -DSQLITE_OMIT_MEMORYDB

#-DSQLITE_OMIT_VIRTUALTABLE -DSQLITE_OMIT_VIEW	

clean:
	rm -f $(APP) $(objects) $(SQLITE) core *~
	cd odflib && (make clean)

run: all
	./$(APP)
