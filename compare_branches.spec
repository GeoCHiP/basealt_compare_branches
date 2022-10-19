Name:           compare_branches
Version:        0.0.1
Release:        1%{?dist}
Summary:        Program that compares two repo branches
BuildArch:      x86_64

Group:          Unspecified

License:        GPL
URL:            https://github.com/GeoCHiP/basealt_compare_branches.git
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc gcc-c++ cmake
Requires:       libcurl-devel nlohmann-json-devel

%description
Simple CLI program that downloads information about packages from https://rdb.altlinux.org/api/export/branch_binary_packages/{branch}
for two arbitrary branches and compares them. The program takes in two
arguments: branch1 and branch2, outputs a file comparison_branch1_branch2.json
where branch1 and branch2 are branch names.


%prep
%setup -q


%build
%cmake
%cmake_build


%install
%cmake_install


%files
%{_bindir}/%{name}
%{_libdir}/libbranch_operations.so
%{_libdir}/libbranch_operations.so.1
%{_libdir}/libbranch_operations.so.1.0.0
%{_includedir}/branch_operations/branch_operations.hpp


%changelog
* Wed Oct 19 2022 alt
- 
