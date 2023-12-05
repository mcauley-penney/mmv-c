# mmv-c 📦

![Build](https://github.com/mcauley-penney/mmv-c/actions/workflows/build.yml/badge.svg)

Edit file and directory names in `$EDITOR`. Inspired by [itchyny/mmv](https://github.com/itchyny/mmv).

## usage

- `mmv` will open the arguments you pass it, files or directories, in a temporary buffer where you can give new names

![mmv-basic](https://github.com/mcauley-penney/mmv-c/assets/59481467/23b24fe1-e62e-48ea-9676-906cc1bbd745)

- It will also handle cyclical renames, should the desire arise

![mmv-cycle](https://github.com/mcauley-penney/mmv-c/assets/59481467/010fef24-fbb2-4e73-9e73-ab7fb62216a9)

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

## credit

[itchyny/mmv](https://github.com/itchyny/mmv)

[Glenn Fowler, Landon Curt Noll, and Kiem-Phong Vo](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function)

[Mike Parker, David MacKenzie, Jim Meyering, and all of the contributors to coreutils/mv](https://github.com/coreutils/coreutils/blob/master/src/mv.c)
