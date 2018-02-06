if [ ! -d "build" ]; then
	mkdir build
fi
cd build
if [ "$1" ==  "proof" ]; then
	cmake .. -DP=1
else
	cmake ..
fi

if [ -d "inputs" ]; then
	echo "Inputs link already exist!"
else
	echo "Linking inputs..."
	make inputs
	echo "Inputs link created!"
fi
make -j
