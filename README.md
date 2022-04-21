# mmv.c 📦

Edit file and directory names in `$EDITOR`


#### motivation
I was introduced to this modern mmv alternative by [itchyny/mmv](https://github.com/itchyny/mmv). Prior to finding it, I had wanted something like it but didn't know how to go about starting. I primitively reimplemented itchyny's Go implementation in Bash and use it often. I started this because I wanted to try my hand at writing it in C and all that comes with that, such as rolling my own hashing/set/map functionality.


#### research

Hashing algorithms:

1. [Introductory resource discussing various hashing algorithms](https://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed)
2. [FNV Hash calculator, used for verifying correctness of algorithm](https://fnvhash.github.io/fnv-calculator-online/)

Temporary file functionality:
1. [GNU resource on temporary files](https://www.gnu.org/software/libc/manual/html_node/Temporary-Files.html)
