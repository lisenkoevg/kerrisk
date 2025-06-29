include ../_src_from_book/Makefile.inc

EXE = va_list

all : ${EXE}

include ../Makefile_lib

clean :
	${RM} ${EXE} *.o

${EXE} : ${TLPI_LIB}

test : ${EXE}
	./test.sh ${EXE}

