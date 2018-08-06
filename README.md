# File_substring_search
The multithread program takes in 4 args from the command promt: The directory path, The output file path, The substring file path and the file mask. 

Multithreading is done in such fashion so as to configure the thread handeling to the implementation of the specific machine(No more threads than what the hardware allows).

The directory path - the directory in which to search for the substring file path specified substring.
The substring file path - substring file.
The output file path - text file into which the programm will output the names of the files in which the substrng was found.
The file mask - a specification that the programm uses to check whether or not to search for the substring.
An example of a cmd input - TheProg.exe C:\Users\ output.txt subs.txt *.*
