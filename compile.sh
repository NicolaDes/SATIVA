if [ -d "build" ]; then
	echo "Cleaning build directory..."
	rm -rf build
	echo "Build directory cleaned!"
fi
mkdir build
cd build
if [ "$1" ==  "proof" ]; then
	echo -e "\e[33m############### Generating sativa with proof generator...\e[39m"
	cmake .. -DP=1
	echo -e "\e[33m############### sativa with proof generated!\e[39m"
else
	echo -e "\e[33m############### Generating sativa without proof generator...\e[39m"
	cmake ..
	echo -e "\e[33m############### sativa without proof generated!\e[39m"
fi

if [ -d "inputs" ]; then
	echo -e "\e[31m############## Inputs link already exist!\e[39m"
else
	echo -e "\e[32m############## Linking inputs...\e[39m"
	make inputs
	echo -e "\e[32m############## Inputs link created!\e[39m"
fi
make -j
