echo "Cleaning Visual Studio project files"
del *.sln
del *.vcxproj*
rmdir /Q /S .vs

echo "Cleaning build directory"
rmdir /Q /S build

echo "Cleaning makefile"
del Makefile
del *.make
