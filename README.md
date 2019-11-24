# CBug

CBug is a Read Evaluate Print Loop (REPL) program for C programming as an interactive shell.
CBug allows for quick development and prototyping of C code.

## Usage
Enter the build directory and run the executable `/CBUG`

The `-save` flag will save the tempory file where input is written to, note it will be a hidden file

## Examples
### Single Line Statements
```c
CBug : press CTRL+C+ENTER to exit
> int x = 2;
> printf("%d\n", x);
2
>
```
### Multi Line Statements
```c
CBug : press CTRL+C+ENTER to exit
> for(int i = 0; i < 5; ++i){
>   int tmp = i + 2;
>   printf("%d\n", tmp);
> }
2
3
4
5
6
>
```

To exit the program enter CTRL + C and then the ENTER key

## Development
To build the program, enter the build directory and run `make` then `./CBUG`

## License
CBug is licensed under the MIT library
