%define name yorick-ml4
%define version 0.6.0
%define release gemini2008nov20

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
mkdir -p $RPM_BUILD_ROOT/usr/lib/yorick/i
mkdir -p $RPM_BUILD_ROOT/usr/lib/yorick/i-start
mkdir -p $RPM_BUILD_ROOT/usr/share/doc/yorick-ml4
mkdir -p $RPM_BUILD_ROOT/usr/lib/yorick/packages/installed

install -m 755 ml4.so $RPM_BUILD_ROOT/usr/lib/yorick/lib
install -m 644 ml4.i $RPM_BUILD_ROOT/usr/lib/yorick/i0
install -m 644 *_check.i $RPM_BUILD_ROOT/usr/lib/yorick/i
install -m 644 *_start.i $RPM_BUILD_ROOT/usr/lib/yorick/i-start
install -m 644 LICENSE $RPM_BUILD_ROOT/usr/share/doc/yorick-ml4
install -m 644 ml4.info $RPM_BUILD_ROOT/usr/lib/yorick/packages/installed


%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/usr/lib/yorick/lib/ml4.so
/usr/lib/yorick/i0/*.i
/usr/lib/yorick/i/*.i
/usr/lib/yorick/i-start/*_start.i
/usr/share/doc/yorick-ml4/LICENSE
/usr/lib/yorick/packages/installed/*

%changelog
* Tue Jan 09 2008 <frigaut@users.sourceforge.net>
- included the info file for compat with pkg_mngr

* Mon Dec 31 2007 <frigaut@users.sourceforge.net>
- new distro directory structure
- updated cvs
