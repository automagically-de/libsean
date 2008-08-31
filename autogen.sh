#!/bin/sh

mkdir -p config

echo "++ aclocal -I m4/ $ACLOCAL_FLAGS $INCLUDES"
aclocal -I m4/ $ACLOCAL_FLAGS $INCLUDES || exit 1 

echo "++ autoheader"
autoheader || exit 1

echo "++ libtoolize --automake --copy --force"
libtoolize --automake --copy --force || exit 1

# echo "++ gtkdocize --copy"
# gtkdocize --copy || exit 1

echo "++ automake --gnu --add-missing --copy"
automake --gnu --add-missing --copy || exit 1

echo "++ autoconf"
autoconf || exit 1

# configure_options="--enable-debug --enable-experimental --enable-gtk-doc"
configure_options="--enable-debug"

if [ "$1" = "-conf" ]; then
	shift
	echo "++ ./configure $configure_options $@"
	./configure $configure_options "$@"
fi
