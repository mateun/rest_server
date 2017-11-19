echo "Building RestService"
pushd build
cl ../main.cpp /Zi ws2_32.lib
popd
