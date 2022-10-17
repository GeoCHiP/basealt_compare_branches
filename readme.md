# About
Simple CLI program that downloads information about packages from https://rdb.altlinux.org/api//export/branch_binary_packages/{branch} for two arbitrary branches and compares them.
The program takes in two arguments: `branch1` and `branch2`, outputs a file comparison_branch1_branch2.json where `branch1` and `branch2` are branch names.

# Dependencies (from p10 repo)

- git (2.33.3)
- gcc10 (10.3.1)
- gcc10-c++ (10.3.1)
- cmake (3.20.5)
- libcurl-devel (7.85.0)
- nlohmann-json-devel (3.10.4)

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
./compare_branches sisyphus p10
```

---

Was tested on [alt-workstation-10.0-x86_64](http://ftp.altlinux.org/pub/distributions/ALTLinux/p10/images/workstation/x86_64/).

