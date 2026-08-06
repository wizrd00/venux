.PHONY: libc

LIBC_CFLAGS := \
	-I${PATH}/libc/include

SRC_FILES != find ${PATH}/libc/src/ -name "*.c"

.for src in ${SRC_FILES}

OBJ != echo ${BIN_DIR}/${src:T} | sed 's/\.c/\.o/g'
HDR != echo ${src} | sed 's/src/include/g' | sed 's/\.c/\.h/g'

${OBJ} : ${src} ${HDR}
	${CC} ${CFLAGS} ${LIBC_CFLAGS} -c -o ${OBJ} ${src}

.endfor
