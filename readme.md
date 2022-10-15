# About
Simple CLI program that downloads information about packages from https://rdb.altlinux.org/api//export/branch_binary_packages/{branch} for two arbitrary branches and compares them.
The program takes in two arguments: `branch1` and `branch2`, outputs a file comparison_branch1_branch2.json where `branch1` and `branch2` are branch names.

# Build & Run

- Clone the repo.
```
git clone https://github.com/GeoCHiP/basealt_compare_branches.git
```
- Build the project.
```
cd basealt_compare_branches/
mkdir build/
cd build/
cmake ..
cmake --build .
```

- Run the binary.

```
./bin/compare_branches sisyphus p10
```

