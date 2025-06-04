include ../_src_from_book/Makefile.inc

EXE = open_error_ETXTBSY

all : ${EXE}

include ../Makefile_lib

clean :
	${RM} ${EXE} *.o

${EXE} : ${TLPI_LIB}

test : ${EXE}
	./test.sh ${EXE}

