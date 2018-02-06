if [ ! -d "build" ]; then
	mkdir build
fi
cd build
cmake ..
if [ -d "inputs" ]; then
	echo "Inputs link already exist!"
else
	echo "Linking inputs..."
	make inputs
	echo "Inputs link created!"
fi
make -j
