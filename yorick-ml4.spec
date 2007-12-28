%define name yorick-ml4
%define version 0.5.01
%define release gemini2007dec07

Summary: yorick library for matlab files I/O
Name: %{name}
Version: %{version}
Release: %{release}
Source0: %{name}-%{version}.tar.bz2
License: BSD
Group: Development/Languages
Packager: Francois Rigaut <frigaut@gemini.edu>
Url: http://www.maumae.net/yorick/doc/plugins.php
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
Requires: yorick >= 2.1


%description
Matlab4 I/O

This package was develop for the Gemini MCAO (RTC file exchange)

Available functions:
 * ml4scan(file[,maxvar])
 * ml4search(file,varname)
 * ml4read(file,varname,leave_open)
 * ml4close,file
 * ml4write(file,data,varname,mode,endian=)

%prep
%setup -q

%build
yorick -batch make.i
make
if [ -f check.i ] ; then
   mv check.i %{name}_check.i
fi;

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/lib/yorick/lib
mkdir -p $RPM_BUILD_ROOT/usr/lib/yorick/i0
mkdir -p $RPM_BUILD_ROOT/usr/lib/yorick/i-start

install -m 755 ml4.so $RPM_BUILD_ROOT/usr/lib/yorick/lib
install -m 644 *.i $RPM_BUILD_ROOT/usr/lib/yorick/i0
install -m 644 *_start.i $RPM_BUILD_ROOT/usr/lib/yorick/i-start

rm $RPM_BUILD_ROOT/usr/lib/yorick/i0/*_start.i


%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/lib/yorick/lib/ml4.so
/usr/lib/yorick/i0/*.i
/usr/lib/yorick/i-start/*_start.i

%changelog
