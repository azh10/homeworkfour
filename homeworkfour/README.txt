*****************************
* FIRST: compile programs   *
*****************************

run command "gcc virtualmachine.c -o virtualmachine.out"
AND
run command "gcc parser.c -o parser.out"

*****************************
* SECOND: run programs      *
*****************************

- to run the virtual machine on mcode.txt file
run command "./virtualmachine.out"

- to parse the input.txt file
run command "./parser.out"

- to parse and run the input.txt
run command "./parse.out && ./virtualmachine.out"
