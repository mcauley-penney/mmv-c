# GCC Options: https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html

# files
bin = mmv # output binary name
bindir = /usr/bin/
src_dir = src
src_files = $(notdir $(wildcard $(src_dir)/*.c)) # src files without dir prefix
VPATH = $(src_dir) # where to look for src files

# flags
optim = -O2
w-arith = -Wdouble-promotion -Wfloat-equal
w-basic = -pedantic -Wall -Wextra
w-extra = -Wcast-align=strict -Wconversion -Wpadded -Wshadow -Wstrict-prototypes -Wvla
w-fmt = -Wformat=2 -Wformat-overflow=2 -Wformat-truncation
w-sgst = -Wsuggest-attribute=const -Wsuggest-attribute=malloc -Wsuggest-attribute=noreturn
warn = $(w-basic) $(w-extra) $(w-arith) $(w-fmt) $(w-sgst)

CFLAGS = $(warn) -c $(optim)
LFLAGS = $(warn)


# object file names
obj_files = $(patsubst %.c, %.o, $(src_files))


# general rule: construct all .o files from
# the source files with the same file name
%.o : %.c
	$(CC) $(CFLAGS) $<


# binary rule: construct binary with given
# name from object files provided
$(bin): $(obj_files)
	$(CC) $(LFLAGS) $^ -o $@


# install target for "sudo make install"
install:
	install -m 007 $(bin) $(bindir)


# clean target: remove all object files and binary
clean:
	rm *.o $(bin)
