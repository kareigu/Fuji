echo "Cleaning Visual Studio project files"
rm *.sln
rm *.vcxproj*
rm -rf .vs

echo "Cleaning build directory"
rm -rf build

echo "Cleaning makefile"
rm Makefile
rm *.make
