 # mmv
# GCC Options: https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html

# -------------------------------------------------------------------
# files and directories
# -------------------------------------------------------------------
bin_name = mmv
test_bin_name = test_$(bin_name)

install_dir = /usr/bin
src_dir = src
inc_dir = inc
test_dir = test
build_dir = build

src_fnames := $(notdir $(wildcard $(src_dir)/*.c))
obj_files := $(addprefix $(build_dir)/, $(src_fnames:.c=.o))

test_src_fnames := $(notdir $(wildcard $(test_dir)/*.c))
test_obj_files := $(addprefix $(build_dir)/, $(test_src_fnames:.c=.o))


# -------------------------------------------------------------------
# flags
# -------------------------------------------------------------------
optim = -O2
w-arith = -Wdouble-promotion -Wfloat-equal
w-basic = -pedantic -Wall -Wextra
w-extra = -Wcast-align=strict -Wconversion -Wpadded -Wshadow -Wstrict-prototypes -Wvla
w-fmt = -Wformat=2 -Wformat-overflow=2 -Wformat-truncation
w-sgst = -Wsuggest-attribute=const -Wsuggest-attribute=malloc -Wsuggest-attribute=noreturn
warn = $(w-basic) $(w-extra) $(w-arith) $(w-fmt) $(w-sgst)

CFLAGS = $(warn) $(optim)
LFLAGS = $(warn)


# -------------------------------------------------------------------
#  targets
# -------------------------------------------------------------------
# tell make not to create files for these target names
.PHONY: all check clean install test test_clean uninstall


all: $(bin_name)
$(bin_name): $(obj_files)
	$(CC) $(LFLAGS) $^ -o $@

$(build_dir)/%.o: $(src_dir)/%.c $(inc_dir)/mmv.h
	mkdir -p $(build_dir)
	$(CC) $(CFLAGS) -c $< -o $@


test: $(test_bin_name)
$(test_bin_name): $(test_obj_files) $(build_dir)/mmv.o
	$(CC) $(LFLAGS) $^ -o $@

$(build_dir)/%.o: $(test_dir)/%.c $(inc_dir)/mmv.h $(test_dir)/test_mmv.h
	mkdir -p $(build_dir)
	$(CC) $(CFLAGS) -c $< -o $@

$(build_dir)/mmv.o: $(src_dir)/mmv.c $(inc_dir)/mmv.h
	mkdir -p $(build_dir)
	$(CC) $(CFLAGS) -c $< -o $@


# clean target: remove all object files and binary
clean:
	rm -rf $(build_dir)
	rm $(bin_name)


test_clean:
	rm -rf $(build_dir)
	rm $(test_bin_name)


# install target for "sudo make install"
install:
	$(NORMAL_INSTALL)
	install -m 007 $(bin_name) $(install_dir)/


uninstall:
	$(NORMAL_UNINSTALL)
	rm $(install_dir)/$(bin_name)
