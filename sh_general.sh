#!/bin/bash
rm tags
rm cscope.* 

ctags -R .
find ./ -iname "*.[ch]" > cscope.files
find ./ -iname "*.cpp" >> cscope.files

cscope -Rkbqi cscope.files

