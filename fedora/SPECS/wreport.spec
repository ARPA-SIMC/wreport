Name: wreport
Version: 3.9
Release: 1
License: GPL2
URL: https://github.com/arpa-simc/%{name}
Source0: https://github.com/arpa-simc/%{name}/archive/v%{version}-%{release}.tar.gz#/%{name}-%{version}-%{release}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
%if 0%{?rhel} == 7
%define python3_vers python34
%else
%define python3_vers python3
%endif
BuildRequires: gcc-c++, doxygen, libtool, lua-devel >= 5.1.1, python-devel, %{python3_vers}-devel
Summary: Tools for working with weather reports
Group: Applications/Meteo
Requires: lib%{name}-common

%description
 libwreport is a C++ library to read and write weather reports in BUFR and CREX
 formats.

The tools provide simple weather bulletin handling functions


%package -n lib%{name}3
Summary: shared library for working with weather reports
Group: Applications/Meteo
Requires: lib%{name}-common

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
BuildRequires: python-docutils

%description -n lib%{name}-doc
libwreport is a C++ library to read and write weather reports in BUFR and CREX
 formats.

 This is the documentation for the library.

%package -n lib%{name}-devel
Summary:  Library for working with (coded) weather reports
Group: Applications/Meteo
Requires: lib%{name}3 = %{version}

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

%package -n python-%{name}3
Summary: shared library for working with weather reports
Group: Applications/Meteo
Requires: lib%{name}3

%description -n python-%{name}3
libwreport is a C++ library to read and write weather reports in BUFR and CREX
 formats.

 This is the Python library

%package -n %{python3_vers}-%{name}3
Summary: shared library for working with weather reports
Group: Applications/Meteo
Requires: lib%{name}3

%description -n %{python3_vers}-%{name}3
libwreport is a C++ library to read and write weather reports in BUFR and CREX
 formats.

 This is the Python library


%prep
%setup -q -n %{name}-%{version}-%{release}

rm -rf %{py3dir}
cp -a . %{py3dir}

%build

autoreconf -ifv

%configure
make

pushd %{py3dir}
autoreconf -ifv
%configure PYTHON=%{__python3}
make
popd

%check
make check
pushd %{py3dir}
make check
popd


%install
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"

pushd %{py3dir}
make install DESTDIR="%{buildroot}"
popd

make install DESTDIR="%{buildroot}"

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
%{_libdir}/libwreport.a
%{_libdir}/libwreport.la
%{_libdir}/pkgconfig/libwreport.pc
%{_libdir}/libwreport.so

%dir %{_includedir}/%{name}
%{_includedir}/%{name}/*


%files -n lib%{name}-doc
%defattr(-,root,root,-)
%doc %{_docdir}/%{name}/libwreport.doxytags
%doc %{_docdir}/%{name}/apidocs/*
%doc %{_docdir}/%{name}/examples/*

%files -n python-%{name}3
%defattr(-,root,root,-)
%dir %{python2_sitelib}/wreport
%{python2_sitelib}/wreport/*
%dir %{python2_sitearch}
%{python2_sitearch}/*.a
%{python2_sitearch}/*.la
%{python2_sitearch}/*.so*

%doc %{_docdir}/wreport/python-wreport.html
%doc %{_docdir}/wreport/python-wreport.rst


%files -n %{python3_vers}-%{name}3
%defattr(-,root,root,-)
%dir %{python3_sitelib}/wreport
%{python3_sitelib}/wreport/*
%dir %{python3_sitearch}
%{python3_sitearch}/*.a
%{python3_sitearch}/*.la
%{python3_sitearch}/*.so*


%changelog
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
