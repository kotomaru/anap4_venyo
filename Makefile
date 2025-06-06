CC = gcc -g -Df2cFortran -Wno-unused-variable -Wno-unused-but-set-variable
# all: anap4.c lorlib.o kinelib.o startup.o dataclient.o histdef.o anaevt.o anap4.h
# 	$(CC) -o anap4 anap4.c lorlib.o kinelib.o startup.o dataclient.o \
#                 histdef.o anaevt.o -lm -Wl,-Bstatic -lmathlib -lpacklib -lkernlib -Wl,-Bdynamic -lpthread -lncurses -lnsl -lgfortran

all: anap4.c kinelib.o startup.o dataclient.o histdef.o anaevt.o anap4.h
	$(CC) -o anap4 anap4.c lorlib.o kinelib.o startup.o dataclient.o \
                histdef.o anaevt.o -lm -Wl,-Bstatic -lmathlib -lpacklib -lkernlib -Wl,-Bdynamic -lpthread -lncurses -lnsl -lgfortran

startup.o: startup.c startup.h anap4.h userdef.h
	$(CC) -c startup.c

histdef.o: histdef.c anap4.h userdef.h
	$(CC) -c histdef.c

anaevt.o: anaevt.c anap4.h kinema.h userdef.h
	$(CC) -c -Wall anaevt.c

kinelib.o: kinelib.c kinema.h
	$(CC) -c kinelib.c

# lorlib.o: lorlib.c lorlib.h
# 	$(CC) -c lorlib.c

dataclient.o: dataclient.c dataclient.h common.h
	$(CC) -c dataclient.c

clean:
	rm -f anap4 *.o



