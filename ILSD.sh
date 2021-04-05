#/bin/bash
baseDir=`pwd`
make config="release"

if [[ $? != 0 ]]; then
	echo "compile failed"
	return
fi

echo "==== Starting ILSD ($@) ===="
cd ./resources/
../binaries/ILSD/Release/ILSD $@
cd $baseDir
