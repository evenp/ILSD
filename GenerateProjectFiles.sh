baseDir=`pwd`
mkdir resources 2> /dev/null
mkdir resources/captures 2> /dev/null
mkdir resources/config 2> /dev/null
mkdir resources/exports 2> /dev/null
mkdir resources/selections 2> /dev/null
mkdir resources/selections/ctracks 2> /dev/null
mkdir resources/selections/ridges 2> /dev/null
mkdir resources/selections/hollows 2> /dev/null
mkdir resources/tiles 2> /dev/null
mkdir resources/strokes 2> /dev/null
mkdir resources/tests 2> /dev/null
mkdir resources/nvm 2> /dev/null
mkdir resources/til 2> /dev/null
mkdir resources/til/top 2> /dev/null
mkdir resources/til/mid 2> /dev/null
mkdir resources/til/eco 2> /dev/null

chmod u+x ./tools/premake5.lin
chmod u+x ./tools/premake5.mac

if [[ "$OSTYPE" == darwin* ]]; then
  echo "Running premake for OSX"
  tools/premake5.mac --file=src/premakeFile.lua xcode4
else
  echo "Running premake for linux"
  tools/premake5.lin --file=src/premakeFile.lua gmake2
fi
cd $baseDir
