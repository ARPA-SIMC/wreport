#!/bin/bash
set -ex

image=$1

if [[ $image =~ ^centos: ]]
then
    pkgcmd="yum"
    builddep="yum-builddep"
    yum install -y yum-utils
elif [[ $image =~ ^fedora: ]]
then
    pkgcmd="dnf"
    builddep="dnf builddep"
    dnf install -y 'dnf-command(builddep)' fedora-packager
fi

$builddep -y fedora/SPECS/wreport.spec

if [[ $image =~ "^fedora:" ]]
then
    pkgname="$(rpmspec -q --qf="wreport-%{version}-%{release}\n" fedora/SPECS/wreport.spec | head -n1)"
    rpmdev-setuptree
    git archive --prefix=$pkgname/ --format=tar HEAD | gzip -c > ~/rpmbuild/SOURCES/$pkgname.tar.gz
    rpmbuild -ba fedora/SPECS/wreport.spec
    find ~/rpmbuild/{RPMS,SRPMS}/ -name "${pkgname}*rpm" -exec cp -v {} . \;
    # TODO upload ${pkgname}*.rpm to github release on deploy stage
else
    autoreconf -ifv
    ./configure
    make check
fi
