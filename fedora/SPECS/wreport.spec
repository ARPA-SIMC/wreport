%global releaseno 1

# Note: define srcarchivename in CI build only.
%{!?srcarchivename: %global srcarchivename %{name}-%{version}-%{releaseno}}

Name: wreport
Version: 3.40
Release: %{releaseno}%{?dist}
License: GPL2
URL: https://github.com/arpa-simc/%{name}
Source0: https://github.com/arpa-simc/%{name}/archive/v%{version}-%{releaseno}.tar.gz#/%{srcarchivename}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot

BuildRequires: doxygen
BuildRequires: meson
BuildRequires: gcc-c++
BuildRequires: pkgconfig(lua) >= 5.1.1
BuildRequires: python3-devel
BuildRequires: python3-sphinx
BuildRequires: python3-breathe
BuildRequires: /usr/bin/rst2html

Summary: Tools for working with weather reports
Group: Applications/Meteo
Requires: lib%{name}-common
Requires: lib%{name}3 = %{?epoch:%epoch:}%{version}-%{release}

%description
 libwreport is a C++ library to read and write weather reports in BUFR and CREX
 formats.

The tools provide simple weather bulletin handling functions


%package -n lib%{name}3
Summary: shared library for working with weather reports
Group: Applications/Meteo
Requires: lib%{name}-common = %{?epoch:%epoch:}%{version}-%{release}

%description -n lib%{name}3
 libwreport is a C++ library to read and write weather reports in BUFR and CREX
 formats.
 
 This is the shared library for C programs.

%package -n lib%{name}-common
Summary: shared library for working with weather reports
Group: Applications/Meteo

%description -n lib%{name}-common
 libwreport is a C++ library to read and write weather reports in BUFR and CREX
 formats.
 
 This is the shared library for C programs.


%package -n lib%{name}-doc
Summary: documentation for libwreport
Group: Applications/Meteo

%description -n lib%{name}-doc
libwreport is a C++ library to read and write weather reports in BUFR and CREX
 formats.

 This is the documentation for the library.

%package -n lib%{name}-devel
Summary:  Library for working with (coded) weather reports
Group: Applications/Meteo
Requires: lib%{name}3 = %{?epoch:%epoch:}%{version}-%{release}

%description -n lib%{name}-devel
libwreport is a C++ library to read and write weather reports in BUFR and CREX
 formats.
 .
 It also provides a useful abstraction to handle values found in weather
 reports, with awareness of significant digits, measurement units, variable
 descriptions, unit conversion and attributes on variables.
 .
 Features provided:
 .
  * Unit conversion
  * Handling of physical variables
  * Read and write BUFR version 2, 3, and 4
  * Read and write CREX

%package -n python3-%{name}3
Summary: shared library for working with weather reports
Group: Applications/Meteo
Requires: lib%{name}3 = %{?epoch:%epoch:}%{version}-%{release}

%description -n python3-%{name}3
libwreport is a C++ library to read and write weather reports in BUFR and CREX
 formats.

 This is the Python library

%prep
%setup -q -n %{srcarchivename}

%build
%meson
%meson_build

%check
%meson_test

%install
%meson_install

%clean
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"

%files
%defattr(-,root,root,-)
%{_bindir}/wrep
%{_bindir}/wrep-importtable

%files -n lib%{name}3
%defattr(-,root,root,-)
%{_libdir}/libwreport.so.*

%files -n lib%{name}-common
%defattr(-,root,root,-)
%{_datadir}/wreport/[BD]*


%files -n lib%{name}-devel
%defattr(-,root,root,-)
%{_libdir}/pkgconfig/libwreport.pc
%exclude %{_libdir}/libwreport.a
%{_libdir}/libwreport.so

%dir %{_includedir}/%{name}
%{_includedir}/%{name}/*


%files -n lib%{name}-doc
%defattr(-,root,root,-)
%doc %{_docdir}/%{name}/libwreport.doxytags
%exclude %{_docdir}/%{name}/html/.doctrees/*
%doc %{_docdir}/%{name}/html/*
%doc %{_docdir}/%{name}/examples/*
%exclude %{_docdir}/%{name}/xml/*
%exclude %{_docdir}/%{name}/html/.buildinfo

%files -n python3-%{name}3
%defattr(-,root,root,-)
%dir %{python3_sitearch}/wreport/
%{python3_sitearch}/wreport/*
%{python3_sitearch}/_wreport*.so

%changelog
* Wed May 14 2025 Daniele Branchini <dbranchini@Branchini-LP-SMR.smr.arpa.emr.net> - 3.40-1
- Fixed an accidental API/ABI break introduced in 3.39 (#61)
- Added `varinfo_create_bufr` and `varinfo_delete_bufr` to create `Varinfo`entries outside of tables. (#61)
- Moved those internal functions that are not used by reverse dependencies to wreport/internals
- Dropped spec support for CentOS7

* Thu May 25 2023 Emanuele Di Giacomo <edigiacomo@arpe.it> - 3.36-1
- Include cstdint in wreport/utils/string.cc (#55)
- Updated wobble

* Wed May 17 2023 Daniele Branchini <dbranchini@arpae.it> - 3.35-2
- Include cstdint in wreport/utils/string.cc (#55)

* Tue Oct 18 2022 Daniele Branchini <dbranchini@arpae.it> - 3.35-1
- Fixed a corner case in decoding of associated fields (#52)

* Fri Jun  3 2022 Daniele Branchini <dbranchini@arpae.it> - 3.34-1
- Fixed build on Fedora 36 (#49)

* Tue Mar 15 2022 Daniele Branchini <dbranchini@arpae.it> - 3.33-1
- Fixed decoding of missing values in compressed MODES messages (#48)

* Wed Feb 23 2022 Emanuele Di Giacomo <edigiacomo@arpae.it> - 3.32-1
- Conversion V <-> mV and NTU <-> LM

* Thu Dec 16 2021 Daniele Branchini <dbranchini@arpae.it> - 3.31-3
- Enabled python binding doc for CentOS8 (#31)

* Thu Dec  2 2021 Daniele Branchini <dbranchini@arpae.it> - 3.31-1
- Fixed decoding of associated fields in compressed messages (#47)

* Sun Mar 28 2021 Daniele Branchini <dbranchini@arpae.it> - 3.30-1
- Fixed dtable parser (#43)
- Improved documentation (#42)

* Mon Jan 25 2021 Daniele Branchini <dbranchini@arpae.it> - 3.29-1
- Implemented `var_hook_domain_errors` option, required by dballe:#241

* Fri Dec 11 2020 Daniele Branchini <dbranchini@arpae.it> - 3.28-1
- Implemented var_clamp_domain_errors option, required by dballe#241

* Wed Sep 30 2020 Emanuele Di Giacomo <edigiacomo@arpae.it> - 3.27-1
- Support Lua 5.4

* Fri Sep 11 2020 Daniele Branchini <dbranchini@arpae.it> - 3.26-1
- Decode compressed string with non-empty reference values (#38)
- Updated wobble and wobblepy
- Always include wreport/version.h so that code that needs it can be compatibile with old versions

* Fri Mar 13 2020 Daniele Branchini <dbranchini@arpae.it> - 3.25-1
- Added NEWS.md (#34)
- Updated lookup/guessing of reference year century on BUFR ed.3 (#36)
- Fix bug with C table modifier 207YYY (#37)
- Improved documentation

* Tue Oct  1 2019 Daniele Branchini <dbranchini@arpae.it> - 3.24-1
- Added python bindings for convert_units (#30)
- Added sphinx infrastructure for documentation

* Fri Aug 30 2019 Emanuele Di Giacomo <edigiacomo@arpae.it> - 3.23-1
- Fix 32 bit builds

* Mon Aug 26 2019 Emanuele Di Giacomo <edigiacomo@arpae.it> - 3.22-1
- Ported python bindings to wobblepy

* Wed Jun 5 2019 Daniele Branchini <dbranchini@arpae.it> - 3.21-1
- Implemented python bindings to iterate over attributes of variables (#27)
- removed python2 bindings and package

* Tue Apr 16 2019 Daniele Branchini <dbranchini@arpae.it> - 3.20-1
- Implemented decoding and encoding of C03 reference value change modifiers (#26)
- moving to python 3.6 on Centos7

* Wed Dec 19 2018 Daniele Branchini <dbranchini@arpae.it> - 3.19-1
- Correct decoding of a BUFR containing an invalid unicode sequence

* Fri Oct 26 2018 Emanuele Di Giacomo <edigiacomo@arpae.it> - 3.18-1
- Version python C API and export type objects

* Mon Oct 22 2018 Daniele Branchini <dbranchini@arpae.it> - 3.17-1
- Added new WMO tables
- Ported to python3
- Removed .la and .a files

* Thu Sep 20 2018 Daniele Branchini <dbranchini@arpae.it> - 3.16-2
- Too many 3.16-1 releases... bogus release to work around copr cache

* Wed Sep 19 2018 Emanuele Di Giacomo <edigiacomo@arpae.it> - 3.16-1
- Fixed typo

* Wed Sep 19 2018 Emanuele Di Giacomo <edigiacomo@arpae.it> - 3.15-1
- Fixed luaX.Y check in configure.ac

* Wed Sep 19 2018 Emanuele Di Giacomo <edigiacomo@arpae.it> - 3.14-1
- Fixed #19
- Fixed parallel build
- Fixed off by one bound checkings

* Thu Jul 26 2018 Emanuele Di Giacomo <edigiacomo@arpae.it> - 3.13-1
- Added stricter and more straightforward message tests

* Thu Jun 7 2018 Daniele Branchini <dbranchini@arpae.it> - 3.12-1
- fixed #17

* Mon Jun 4 2018 Daniele Branchini <dbranchini@arpae.it> - 3.11-1
- fixed #16

* Wed Apr 18 2018 Daniele Branchini <dbranchini@arpae.it> - 3.10-2
- Deallocate storage for Var objects

* Thu Feb 22 2018 Daniele Branchini <dbranchini@arpae.it> - 3.10-1
- Updated wobble
- Added missing newline ad the end of verbose output

* Tue Feb 14 2017 Daniele Branchini <dbranchini@arpae.it> - 3.9-1
- Added ppt (part per thousand) conversion

* Mon Jan 9 2017 Daniele Branchini <dbranchini@arpae.it> - 3.8-2
- Managing python namespace for CentOs 7

* Mon Dec 19 2016 Daniele Branchini <dbranchini@arpae.it> - 3.8-1
- Added documentation for table file formats (fixes #11)
- Splitted python2 and python3 packages (fixes #12)

* Wed Oct 5 2016 Daniele Branchini <dbranchini@arpae.it> - 3.7-1
- fixed duplicate varaible in table
- check tables ad build times (fixes #10)
- implemented test cases with American Fuzzy Loop
- updated wobble

* Tue Sep 15 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 3.2-1%{dist}
- gcc 4.8.3 support (lambdas and variadic templates)
- Removed every reference to libwreport-test.pc.in
- version 3.2 and version-info 3:2:0
- Normalise CODE TABLE and FLAG TABLE units
-  Removed dependency on wibble in favour of new utils/ code in sync with wobble

* Mon Aug 31 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 3.1-1%{dist}
- Fixed FLAGTABLE and CODETABLE conversion errors

* Wed Jul 29 2015 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 3.0-0.1%{dist}
- wreport 3.0 pre-release

* Tue Aug 05 2014 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 2.13-4298%{dist}
- Updated wrep-importtable to deal with new zipfiles and XML files published by
  WMO
- Ship more tables (CREX table 17 is a copy of table 18 as a workaround, since
  I could not find a parseable version of table 17)
- Added more unit conversions to deal with the changed unit names in new BUFR
  tables

* Wed Nov 13 2013 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 2.10-4116%{dist}
- Fixed linking bug.

* Tue Nov 12 2013 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 2.10-4104%{dist}
- No changes, but bumped the version number to succeed an internally released 2.9

* Fri Aug 23 2013 Emanuele Di Giacomo <edigiacomo@arpa.emr.it> - 2.9-3968%{dist}
- Aggiunta conversione ug/m**3->KG/M**3 (e viceversa)

* Fri Nov 23 2012 root <dbranchini@arpa.emr.it> - 2.5-3724%{dist}
- Aggiornamento sorgenti

* Tue Jun 12 2012 root <dbranchini@arpa.emr.it> - 2.5-3633%{dist}
- Aggiornamento sorgenti

* Tue May 8 2012 root <dbranchini@arpa.emr.it> - 2.4-3621%{dist}
- Aggiunta conversione da minuti (oracle) a S

* Tue Sep 28 2010 root <ppatruno@arpa.emr.it> - 1.0
- Initial build.
