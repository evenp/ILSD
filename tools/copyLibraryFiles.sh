baseDir=`pwd`
mkdir ../src/Libs/ 2> /dev/null

echo -------IMGUI-------
mkdir ../src/Libs/imgui/ 2> /dev/null
cp ../deps/imgui/*.h ../src/Libs/imgui
cp ../deps/imgui/*.cpp ../src/Libs/imgui
cp ../deps/imgui/examples/imgui_impl_glfw.cpp ../src/Libs/imgui
cp ../deps/imgui/examples/imgui_impl_glfw.h ../src/Libs/imgui
cp ../deps/imgui/examples/imgui_impl_opengl3.cpp ../src/Libs/imgui
cp ../deps/imgui/examples/imgui_impl_opengl3.h ../src/Libs/imgui

if [[ "$OSTYPE" != darwin* ]]; then
	echo -------GLAD-------
	rmdir ../src/Libs/glad/ /s /q 2> /dev/null
	mkdir ../src/Libs/glad/ 2> /dev/null
	cp -r ../deps/glad/* ../src/Libs/glad
fi

echo -------STBI-------
rmdir ../src/Libs/stbi/ /s /q 2> /dev/null
mkdir ../src/Libs/stbi/ 2> /dev/null
cp ../deps/stb/stb_image.h ../src/Libs/stbi
cp ../deps/stb/stb_image_write.h ../src/Libs/stbi
cd "$baseDir"
