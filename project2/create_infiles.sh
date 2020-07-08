#!/bin/bash
#http://tldp.org/LDP/abs/html/sha-bang.html
echo
E_WRONG_ARGS=85
script_parameters="diseaseFile countriesFile input_dir numFilesPerDirectory numRecordsPerFile"

if [ ! $# == 5 ]
then
  echo "Usage: `basename $0` $script_parameters"
  echo
  #`basename $0` is the script's filename.
  exit $E_WRONG_ARGS
fi

diseaseFile="$1"
while read line; do
   # reading each line
    i="0"
    j="0"
    mkdir -p "${PWD}/$3/"$line"";
    cp -p $diseaseFile "${PWD}/$3/"$line""
    cd "${PWD}/$3/"$line""
        while [ $i -lt $4 ]; do
            while [ $j -lt $5 ]; do
                echo -e  $(shuf -i 0-10000 -n 1; shuf -n 1 -e ENTER EXIT; head /dev/urandom | tr -dc A-Za-z | head -c `shuf -i 3-12 -n 1` ; echo ; head /dev/urandom | tr -dc A-Za-z | head -c `shuf -i 3-12 -n 1`; echo;  shuf -n 1 $diseaseFile; echo ; shuf -i 1-120 -n 1; )  >> data
                j=$[$j+1]
                done
                cat data >`date -d "$((RANDOM%30+1990))-$((RANDOM%12+1))-$((RANDOM%28+1))" '+%d-%m-%Y'`
        i=$[$i+1]
        done
    rm data
    rm $diseaseFile
cd -
done < "$2"
echo "$3 folder created"
exit 0
