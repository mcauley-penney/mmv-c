# files
bin = mmv # output binary name
bindir = /usr/bin/
src_dir = src
src_files = $(notdir $(wildcard $(src_dir)/*.c)) # src files without dir prefix
VPATH = $(src_dir) # where to look for src files

# flags
general = -pipe -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wsuggest-attribute=malloc
optim = -O2
warn_basic = -Wall -Wextra
warn_extra = -pedantic -Wdouble-promotion -Wfloat-equal -Wpadded
warn_format = -Wformat=2 -Wformat-overflow=2 -Wformat-truncation
warn = $(warn_basic) $(warn_extra) $(warn_format)

CFLAGS = $(general) $(warn) -c $(optim) # compile flags
LFLAGS = $(warn) # linking flags


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
	install -m 557 $(bin) $(bindir)


# clean target: remove all object files and binary
clean:
	rm *.o $(bin)
