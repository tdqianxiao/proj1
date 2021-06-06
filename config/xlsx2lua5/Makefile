ZLIB_DIR = luaclib/zlib/

ZLIB_SRC = $(addprefix $(ZLIB_DIR), \
 adler32.c\
 compress.c\
 crc32.c\
 deflate.c\
 gzclose.c\
 gzlib.c\
 gzread.c\
 gzwrite.c\
 infback.c\
 inffast.c\
 inflate.c\
 inftrees.c\
 trees.c\
 uncompr.c\
 zutil.c\
)

MINIZIP_DIR = luaclib/minizip/

MINIZIP_SRC = $(addprefix $(MINIZIP_DIR), \
ioapi.c\
mztools.c\
unzip.c\
zip.c\
)

CFLAGS = -O2 -Wall

#all : xlsx2lua.exe

#main.c : xlsx2lua.lua srlua.lua
#	../lua-5.3.5/src/lua.exe srlua.lua $< > $@

PLAT= none
PLATS= linux windows

all:	$(PLAT)

none:
	@echo "Please choose a platform:"
	@echo "   $(PLATS)"

windows : main.c md5sum.c luaclib/luazip.c $(ZLIB_SRC) $(MINIZIP_SRC)
	cd ./lua-5.3.5/src && rm -rf *.o && make mingw
	gcc $(CFLAGS) -o xlsx2lua.exe $^ -I$(ZLIB_DIR) -I$(MINIZIP_DIR) -I/usr/local/include -L/usr/local/lib -I./lua-5.3.5/src ./lua-5.3.5/src/liblua.a -I./pthread pthreadGC2.dll

linux : main.c md5sum.c luaclib/luazip.c $(ZLIB_SRC) $(MINIZIP_SRC)
	cd ./lua-5.3.5/src && rm -rf *.o && make linux
	gcc $(CFLAGS)  -o xlsx2lua.out $^ -I$(ZLIB_DIR) -I$(MINIZIP_DIR) -I/usr/local/include -L/usr/local/lib -I./lua-5.3.5/src ./lua-5.3.5/src/liblua.a -lpthread -ldl -lrt -lc -rdynamic -lpthread -lm

clean :
	cd ./lua-5.3.5/src && rm *.o && rm  *.a
	rm xlsx2lua.*

