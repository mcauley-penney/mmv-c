# mmv-c 📦

![Build](https://github.com/mcauley-penney/mmv-c/actions/workflows/build.yml/badge.svg)

Edit file and directory names in `$EDITOR`. Inspired by [itchyny/mmv](https://github.com/itchyny/mmv).

## usage

mmv behaves like other commandline tools: it accepts a list of arguments, including patterns like wildcards. See `man mmv` for options.

## example

![mmv](https://github.com/mcauley-penney/mmv-c/assets/59481467/ecf97305-7847-4878-9ee7-5a86a287634e)

In the above example, mmv is provided the `verbose` argument so that it will list what renames it conducts and three wildcard arguments that are duplicates of each other: 
1. the set of files in the current directory that begin with `test`
2. the same set of files as before
3. the same set of files again, except using an alternate path string

mmv is capable of removing duplicate arguments even when the input strings don't match, for example `test0.txt` and `~/test_dir/test0.txt`. In the editing buffer, we see only one instance of each unique file, though three were given for each. When cycles between renames are detected, for example renaming `test0.txt` to `test1.txt` even though that destination already exists, mmv will remove the cycles by conducting intermediate renames on only those files which are detected as cycles, i.e. it avoids renaming everything when a cycle is detected. These intermediate rename operations are visible in the verbose output. cat is used here to display that the contents of the files remains the same.

## installation

1. Clone this repository and enter the repo directory

```sh
 $ git clone https://github.com/mcauley-penney/mmv-c.git
 $ cd mmv-c
```

2. Generate build file and run build

```sh
 $ cmake -Bbuild -DCMAKE_BUILD_TYPE=Release
 $ cmake --build ./build
```

3. Install

```sh
$ sudo cmake --install ./build
```

Or if you want to change prefix use `--prefix=<path(/usr, /usr/local, $HOME/.local)>` option.

4. Feel free to remove the cloned repo

```sh
 $ cd ..
 $ rm -rf mmv-c
```

```
$ sudo cmake --install ./build
```

## credit

[itchyny/mmv](https://github.com/itchyny/mmv)

[Glenn Fowler, Landon Curt Noll, and Kiem-Phong Vo](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function)

[Mike Parker, David MacKenzie, Jim Meyering, and all of the contributors to coreutils/mv](https://github.com/coreutils/coreutils/blob/master/src/mv.c)
