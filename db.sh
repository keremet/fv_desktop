#wget http://www.sqlite.org/2013/sqlite-amalgamation-3071602.zip
#unzip sqlite-amalgamation-3071602.zip
#cd sqlite-amalgamation-3071602
#gcc -c -o sqlite3.o sqlite3.c
#gcc -o sqlite sqlite3.o shell.c
#cd -
rm -f db.sqlite
sqlite-amalgamation-3071602/sqlite3 db.sqlite < orv_ddl.sql


