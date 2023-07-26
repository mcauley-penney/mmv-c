# Make manual: https://www.gnu.org/software/make/manual/make.html
# GCC Options: https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html

# mmv

# -------------------------------------------------------------------
# files and directories
# -------------------------------------------------------------------
bin_name = mmv
man_name = $(bin_name).1.gz

prefix = /usr/local
bin_dir = $(prefix)/bin
man_dir = $(prefix)/man/man1

src_dir = src
test_dir = test
debug_dir = debug
build_dir = build

src_files		:= $(wildcard $(src_dir)/*)
def_files 		:= $(wildcard $(src_dir)/*.c)
test_def_files  := $(patsubst $(src_dir)/%.c, $(test_dir)/test_%.c, $(def_files))


# -------------------------------------------------------------------
# flags
# -------------------------------------------------------------------
optim_flags = -O2
w-arith = -Wdouble-promotion -Wfloat-equal
w-basic = -pedantic -Wall -Wextra
w-extra = -Wcast-align=strict -Wconversion -Wpadded -Wshadow -Wstrict-prototypes -Wvla
w-fmt = -Wformat=2 -Wformat-overflow=2 -Wformat-truncation
warn_flags = $(w-basic) $(w-extra) $(w-arith) $(w-fmt)

CFLAGS = $(warn_flags) $(optim_flags)


# -------------------------------------------------------------------
#  targets
# -------------------------------------------------------------------
.PHONY: all test debug clean test_clean install uninstall


all:
	$(CC) $(CFLAGS) main.c $(src_files) -o $(bin_name)


test:
	mkdir -p ./test/bin

	$(CC) ./test/test_utils.c ./test/unity.c -o ./test/bin/test_utils
	$(CC) ./test/test_set.c  ./test/unity.c -o ./test/bin/test_set
	$(CC) -D DEBUG=1 ./test/test_mmv.c  ./test/unity.c -o ./test/bin/test_mmv

	./test/bin/test_utils
	./test/bin/test_set
	./test/bin/test_mmv


debug:
	$(CC) $(CFLAGS) -D DEBUG=1 main.c $(src_files) -o debug_$(bin_name)


# clean target: remove all object files and binary
clean:
	rm $(bin_name)


test_clean:
	rm -rf ./test/bin


debug_clean:
	rm debug_$(bin_name)


# install target for "sudo make install"
install:
	$(NORMAL_INSTALL)
	install -m 007 $(bin_name) $(bin_dir)
	cp ./man/$(man_name) $(man_dir)/$(man_name)


uninstall:
	$(NORMAL_UNINSTALL)
	rm $(bin_dir)/$(bin_name)
	rm $(man_dir)/$(man_name)
