EXE = tcp_socket_pool

DEP_LIB = lib/pthread_pool/lib.a
RM = rm -f

IMPL_CFLAGS = -std=c99 -D_XOPEN_SOURCE=600 \
	        -D_DEFAULT_SOURCE \
		-g -I${DEP_DIR} \
		-pedantic \
		-Wall \
		-W \
		-Wmissing-prototypes \
		-Wno-sign-compare \
		-Wimplicit-fallthrough \
		-Wno-unused-parameter

 IMPL_THREAD_FLAGS = -pthread

CFLAGS = ${IMPL_CFLAGS} ${IMPL_THREAD_FLAGS}
LDLIBS = ${DEP_LIB}

all: ${EXE} ${DEP_LIB}

clean:
	rm -f ${EXE}
