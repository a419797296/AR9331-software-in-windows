#!/bin/sh

ftp -niv <<- EOF  
open 192.168.31.121  
user blue willtech  
ascii
put ./kill.sh  
bye  
EOF 
