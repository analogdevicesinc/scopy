#!/bin/bash

#get all .rst files
files=$(find . -type f -name "*.rst")

# files="./user_guide/index.rst"

# echo $files

# Check if the correct number of arguments is provided
if [ -z "${files}" ]; then
    echo "No .rst files found "
    exit 1
fi


for FILE in $files; do
    echo "Formating $FILE"
    # format file to have max 80 cols per row
    fold -sw 80 "${FILE}" >tmp.rst && mv tmp.rst "${FILE}"
    # replace tabs with spaces ( each tab is replaced with 2 spaces )
    sed -i 's/\t/  /g' "${FILE}"

    # prev_line=""

    # regex_link_start='([^`]+)'

    # regex_complete_link='`([^`]+)`'

    # while ISF= read -r line; do
       
    #     if [[ ! -z "${prev_line}" ]]; then
    #         echo "$prev_line  $line"
    #         $prev_line=""
    #     fi

    #     # echo "$line"

    #     # in this line a link started 
    #     if [[ $line =~ $regex_link_start ]]; then
    #         if [[ ! $line =~ $regex_complete_link ]]; then 
    #             prev_line="$line"
    #         fi
    #     fi
        
       
    #     if [ -z "${prev_line}"]; then 
    #         if [[ $line == *\ * ]]; then
    #             echo "$line" # Print the original line
    #             echo ""      # Add an empty line after the line starting with *
    #         else
    #             echo "$line" # Print the line unchanged
    #         fi
    #     fi
    # done < "${FILE}" >tmp.rst && mv tmp.rst "${FILE}" 
done
