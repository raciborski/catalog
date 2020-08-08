# catalog
This *CLI* tool helps tracking file system changes by storing the directory structure along with basic metadata and **SHA-3** hashes of individual files in an **SQLite** database.
It may be useful to notice unwanted or malicious modifications, so the files can be restored from an intact backup in time.
Additionally, it is possible to force the program to compare the hashes regardless of modification times in order to detect data degradation.

## Compilation
This project has the following dependencies:
  * **gcc** or **clang**
  * **glibc** >= 2.10
  * **sqlite3**

If the environment meets the requirements, set the desired location of the database in *main.c* by assigning the correct value to the `DB_PATH` variable, then you can compile the program with one of the commands below:
```bash
gcc -O2 -s -Wall main.c filesystem.c keccak.c node.c print.c -lsqlite3 -o catalog
clang -O2 -s -Wall main.c filesystem.c keccak.c node.c print.c -lsqlite3 -o catalog
```
