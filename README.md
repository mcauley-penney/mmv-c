# mmv-c 📦

Edit file and directory names in `$EDITOR`

![mmv_example](https://user-images.githubusercontent.com/59481467/168492957-ad35e23d-a5d7-4620-8551-1842101ceb73.gif)


#### about
I like and frequently use the functionality of itchyny/mmv but wanted to implement it for myself.

This implementation forgoes some of the features of the original that I don't want, such as cyclical renaming. See it for a more featureful experience.


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


##### credit
[itchyny/mmv](https://github.com/itchyny/mmv)

[Glenn Fowler, Landon Curt Noll, and Kiem-Phong Vo](https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function)

[Mike Parker, David MacKenzie, Jim Meyering, and all of the contributors to coreutils/mv](https://github.com/coreutils/coreutils/blob/master/src/mv.c)
