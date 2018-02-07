if [ ! -d "docs" ]; then
       mkdir docs
fi       
echo "Generating doxygen in docs..."
doxygen Doxygen.cfg 
echo "Ended generation!"
firefox docs/html/index.html
