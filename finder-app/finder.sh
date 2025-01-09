#!/bin/sh
#Accepts the following runtime arguments: the first argument is a path to a 
#directory on the filesystem, referred to below as filesdir; the second argument
# is a text string which will be searched within these files, referred to below
# as searchstr

#Exits with return value 1 error and print statements if any of the parameters a
#bove were not specified

#Exits with return value 1 error and print statements if filesdir does not 
#represent a directory on the filesystem

#Prints a message "The number of files are X and the number of matching lines
# are Y" where X is the number of files in the directory and all subdirectories
#and Y is the number of matching lines found in respective files, where a 
#matching line refers to a line which contains searchstr (and may also contain 
#additional content). 

if [ $# -ne 2 ]
then
        echo "Two arguments are expected, Path and String_to_search"
        exit 1
else
        SEARCHPATH=$1
        SEARCHSTR=$2
        if  [ ! -d $SEARCHPATH ]
        then
            echo "Path provided '$SEARCHPATH' is not valid"
            exit 1
        fi
fi

NUM_FILES=`find $SEARCHPATH -type f| wc -l`
MATCHES=`grep -r $SEARCHSTR $SEARCHPATH|wc -l`
echo "The number of files are $NUM_FILES and the number of matching lines are $MATCHES"
