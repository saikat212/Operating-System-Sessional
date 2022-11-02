#!/bin/bash

working_directory=`pwd`

inputfile_name=""
current_directory=`pwd`

if [ $# = "0" ]; then
	echo "no cmd line arg: working directory name (optional) and input file name must be given"
elif [ $# = "1" ]; then 
	if [[ $1 == *".txt"* ]]; then 
		find ./ -type f -name $1 | grep -q $1 > /dev/null && inputfile_name=$1
	else
		echo "input file name must be given"
		find ./ -type d -name $1 | grep -q $1 > /dev/null && working_directory=`find ./ -type d -name $1`
	fi

elif [ $# = "2" ];then
	if [[ $2 == *".txt"* ]];then 
		find ./ -type f -name $2 | grep -q $2 > /dev/null && inputfile_name=$2
	else
		echo "input file name must be given"
	fi
	find ./ -type d -name $1 | grep -q $1 > /dev/null && working_directory=`find ./ -type d -name $1`

else 
	echo " too much cmd ling arg:"

fi	

until [ "$inputfile_name" != "" ]
do
	echo -n "give valid input file name: "
	read temp
	if [[ $temp == *".txt"* ]]; then
		find ./ -type f -name $temp | grep -q $temp > /dev/null && inputfile_name=$temp
	fi
done


IFS=$'\n'
inputfile_content=( `cat $inputfile_name` )

cd $working_directory

output_directory=`find ./ -type d -name "1705116_output_dir"`

rm -f -r $output_directory

csv_directory=`find ./ -type d - name "1705116_output.csv"`
rm -f $csv_directory



echo "d1:$1"
#readable_textfiles=( `find ./ -type f -exec file {} \; | grep -i 'ascii\|unicode' | awk -F':' '{print $1}'` )

expectedfile=(`find \( -not -name "*.${inputfile_content[0]}" -type f \) -and  \( -not -name "*.${inputfile_content[1]}" -type f \) -and \( -not -name "*.${inputfile_content[2]}" -type f \) -and \( -not -name "*.${inputfile_content[3]}" -type f \)` )

#expectedfile=( `find \( -name "*.${inputfile_content[3]}" -type f \) -or \( -name "chart*" -type f \)` )
#echo "find \( -name "*.${inputfile_content[3]}" -type f \)"

<<com
allfile="find \( -not -name "\"*.${inputfile_content[3]}\"" -type f \) "

 for((i=1;i<${#inputfile_content[*]};i++))
do
	allfile+=" -and \( -not -name "\"*.${inputfile_content[$i]}\"" -type f \)"
	
done



x=`$s`
count=0
for i in "$x"
do 
	expectedfile[$count]="$i"
	count=$(($count+1))
done

command="find -not -name "*.html" -type f"
x=`$command`
count=0
for i in "$x"
do
	array[$count]="$i"
	count=$(($count+1))
done

echo "${array[*]}"

com




cd $current_directory
cd $working_directory
cd ..

rm -f -r 1705116_output_dir
mkdir 1705116_output_dir
cd 1705116_output_dir
output_directory=`pwd`
cd ..
cd $working_directory




counter=0
extensionGet(){
	echo "$1" | sed 's/^.*\.//'
}
for((i=0;i<${#expectedfile[*]};i++))
do
	echo "${expectedfile[$i]}"
	extension="$(extensionGet "${expectedfile[$i]}")"
	mkdir -p "$output_directory/$extension"
	cp "${expectedfile[$i]}" "$output_directory/$extension"
	
	
	
done


for((i=0;i<${#inputfile_content[*]};i++))
do

	echo "${inputfile_content[$i]}"
done


csv_file_name="output.csv"
echo "file_type,no_of_files">$csv_file_name
cd ..

cd "1705116_output_dir"
z=`pwd`
echo "$z"



for i in *
do
	cnt=`ls $i | wc -l`
	

        echo "$i,${cnt}">>$csv_file_name

done
echo "ignored","4">>$csv_file_name
