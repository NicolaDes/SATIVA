if [ ! -d "docs" ]; then
       mkdir docs
fi       
echo "Generating doxygen in docs..."
doxygen Doxygen.cfg 
echo "Ended generation!"
if ! type firefox > /dev/null; then
	echo  -e "\e[31mFirefox not installed! open < docs/html/index.html > with your browser\e[39m"
else
	firefox docs/html/index.html
fi
