 #!/usr/bin/sh
baseDir=`pwd`
echo BUILDING GLFW...
cd ../deps/glfw/
mkdir build 2> /dev/null
cd build
cmake ../ -DBUILD_SHARED_LIBS=ON
sudo make install
echo DONE!

echo BUILDING SHAPELIB...
cd ../../shapelib/
mkdir build 2> /dev/null
cd build
cmake ../ -Wno-dev
sudo make install
cd "$baseDir"
echo DONE!
