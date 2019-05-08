#!/bin/bash
mkdir -p release
#!/bin/bash
mkdir -p release
make clean builddir=release
rm release/*.o release/*.d
echo "Compilation output will be in file compil_out.txt"

MACHINE_TYPE=`uname -m`
if [ ${MACHINE_TYPE} == 'x86_64' ]; then
    echo "Building for architecture x64 in folder release";
    make -j4 default CXXFLAGS="-m64 -std=c++11" CFLAGS=-m64 LDFLAGS="-m64 -Wl,-rpath,'\$\$ORIGIN'/../../libs/FTDI/linux/x64,-rpath,'\$\$ORIGIN'/../../libs/MPSSE/linux/x64" builddir=release > compil_out.txt 2>&1
elif [[ ${MACHINE_TYPE} =~ arm.* ]]; then
    echo "Building for architecture ARM in folder release";
    make -j4 default LDFLAGS="-Wl,-rpath,'\$\$ORIGIN'/../../libs/FTDI/linux/ARM,-rpath,'\$\$ORIGIN'/../../libs/MPSSE/linux/ARM" builddir=release > compil_out.txt 2>&1
else
    echo "Building for architecture x86 in folder release";
    make -j4 default CXXFLAGS=-m32 CFLAGS=-m32 LDFLAGS="-m32 -Wl,-rpath,'\$\$ORIGIN'/../../libs/FTDI/linux/x86,-rpath,'\$\$ORIGIN'/../../libs/MPSSE/linux/x86" builddir=release > compil_out.txt 2>&1
fi

if [[ $(tail -n 1 compil_out.txt) =~ .*Err.* ]] || !( ls ./release/libLeddar* 1> /dev/null 2>&1) ; then
    echo "Error, check compil_out.txt for more information";
else
    echo "Done";
fi
