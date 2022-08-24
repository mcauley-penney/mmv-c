# mmv-c 📦
![Build Badge](https://github.com/mcauley-penney/mmv-c/actions/workflows/build.yml/badge.svg)
![Valgrind Badge](https://github.com/mcauley-penney/mmv-c/actions/workflows/run_valgrind.yml/badge.svg)

Edit file and directory names in `$EDITOR`. Inspired by [itchyny/mmv](https://github.com/itchyny/mmv).

![mmv_intro](https://user-images.githubusercontent.com/59481467/168495786-2d7900e7-50ff-4ca4-aa86-67bf9da24199.gif)


## usage
Like other command line tools, `mmv` accepts wildcards and special paths, such as `..`

![mmv_example](https://user-images.githubusercontent.com/59481467/168495798-8102f258-98a0-4ce2-b98d-2640ff6afa9c.gif)


## installation
1. Clone this repository and enter the repo directory
2. Issue `make`, then `sudo make install`
3. Feel free to remove the cloned repo

In all:
```
git clone https://github.com/mcauley-penney/mmv-c.git
cd mmv-c
make
sudo make install
cd ..
sudo rm -r mmv-c
```

## differences with itchyny/mmv
This project doesn't seek to completely duplicate the functionality of the original. It is a work-in-progress and may, in the future, incorporate functionality from the original or new abilities that I might find useful.


## credit
[itchyny/mmv](https://github.com/itchyny/mmv)

[Glenn Fowler, Landon Curt Noll, and Kiem-Phong Vo](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function)

[Mike Parker, David MacKenzie, Jim Meyering, and all of the contributors to coreutils/mv](https://github.com/coreutils/coreutils/blob/master/src/mv.c)
