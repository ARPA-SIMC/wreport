Summary: Library for working with (coded) weather reports
Name: wreport
Version: 2.11
Release: 4207%{dist}
License: GPL2
Group: Applications/Meteo
URL: http://www.arpa.emr.it/dettaglio_documento.asp?id=514&idlivello=64
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
BuildRequires: doxygen, libtool, libwibble-devel >= 1.1, gperf,lua-devel >= 5.1.1
%description
null

Summary: Tools for working with weather reports
Group: Applications/Meteo
Requires: lib%{name}-common

%description
 libwreport is a C++ library to read and write weather reports in BUFR and CREX
 formats.

The tools provide simple weather bulletin handling functions


%package -n lib%{name}2
Summary: shared library for working with weather reports
Group: Applications/Meteo
Requires: lib%{name}-common

%description -n lib%{name}2
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

%prep
%setup -q

%build
%configure
make

%check
make check

%install
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"
make install DESTDIR="%{buildroot}"

%clean
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"

%files
%defattr(-,root,root,-)
%{_bindir}/wrep
%{_bindir}/wrep-importtable

%files -n lib%{name}2
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
%{_libdir}/libwreport-test.a
%{_libdir}/libwreport-test.la
%{_libdir}/pkgconfig/libwreport-test.pc

%dir %{_includedir}/%{name}
%{_includedir}/%{name}/*


%files -n lib%{name}-doc
%defattr(-,root,root,-)
%doc %{_docdir}/%{name}/libwreport.doxytags
%doc %{_docdir}/%{name}/apidocs/*
%doc %{_docdir}/%{name}/examples/*

%changelog
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
