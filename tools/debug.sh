cd tools
./premake5 gmake
cd ..
cd build
rm -rf bin/
make config=debug_win32 -j10