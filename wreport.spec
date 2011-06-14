Summary: Library for working with (coded) weather reports
Name: wreport
Version: 1.9
Release: 3335
License: GPL2
Group: Applications/Meteo
URL: http://www.arpa.emr.it/dettaglio_documento.asp?id=514&idlivello=64
Source0: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
BuildRequires: doxygen, libtool, libwibble-devel, gperf,lua-devel >= 5.1.1
%description
null

%package -n lib%{name}1
Summary: shared library for working with weather reports
Group: Applications/Meteo
Requires: lib%{name}-common

%description -n lib%{name}1
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
make check

%install
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"
make install DESTDIR="%{buildroot}"

%clean
[ "%{buildroot}" != / ] && rm -rf "%{buildroot}"


%files -n lib%{name}1
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

%changelog
* Tue Sep 28 2010 root <ppatruno@arpa.emr.it> - 1.0
- Initial build.
