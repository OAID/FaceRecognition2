SUBDIRS :=
OBJS := $(addprefix $(OBJS_DIR), $(patsubst %.cpp,%.o,$(wildcard *.cpp)))

all: $(OBJS)
	$(foreach dir, $(SUBDIRS), make -C $(dir);)

$(OBJS_DIR)%.o : %.cpp
	${CXX} -c ${CFLAGS} ${INCLUDE_DIR} $< -o $@
$(OBJS_DIR)%.o : %.c
	${CC}  -c ${CFLAGS} ${INCLUDE_DIR} $< -o $@

clean:
	rm -f ${OBJS_DIR}*.o
	$(foreach dir, $(SUBDIRS), make clean -C $(dir);)


