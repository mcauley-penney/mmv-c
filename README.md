# mmv-c 📦

Edit file and directory names in `$EDITOR`. Inspired by [itchyny/mmv](https://github.com/itchyny/mmv).


#### usage

Like other command line tools, `mmv` accepts wildcards and special paths, such as `..`


#### installation

1. Clone this repository and enter the repo directory
2. Issue `make`, then `sudo make install`
3. Feel free to remove the cloned repo

In all:
```
git clone https://github.com/mcauley-penney/mmv.c.git
cd mmv.c
make
sudo make install
cd ..
sudo rm -r mmv.c
```

#### about
I like and frequently use the base functionality of the original, linked above. I've implemented something like it before [in shell](https://github.com/mcauley-penney/mmv.sh) and wanted to try my hand at a C implementation.

This version forgoes some of the features of the original that I don't use and don't need, such as cyclical renaming. See it for a more featureful experience.


##### credit
[itchyny/mmv](https://github.com/itchyny/mmv)

[Glenn Fowler, Landon Curt Noll, and Kiem-Phong Vo](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function)

[Mike Parker, David MacKenzie, Jim Meyering, and all of the contributors to coreutils/mv](https://github.com/coreutils/coreutils/blob/master/src/mv.c)
