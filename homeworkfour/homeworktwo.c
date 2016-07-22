// Homeworktwo: Lexer
// Ashton Ansag
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// enum with token values for the lexemes
typedef enum{
	nulsym = 1, identsym, numbersym, plussym, minussym, 
	multsym, slashsym, oddsym, eqlsym, neqsym, 
	lessym, leqsym, gtrsym, geqsym, lparentsym, 
	rparentsym, commasym, semicolonsym, periodsym, becomessym,
	beginsym, endsym, ifsym, thensym, whilesym,
	dosym, callsym, constsym, varsym, procsym, 
	writesym, readsym, elsesym
}token;

/* I decided instead of making arrays of digits/letter/symbols to test the 
   ASCII value to determine what the char is. */

// ASCII Value Range for Digits
#define DIGIT_MIN 48
#define DIGIT_MAX 57

// ASCII Value Range for Uppercase Letters
#define ULETTER_MIN 65
#define ULETTER_MAX 90

// ASCII Value Range for Lowercase Letters
#define LLETTER_MIN 97
#define LLETTER_MAX 122

// ASCII Values for Invisible Characters
#define TAB 9
#define WHITE_SPACE 32
#define NEWLINE 10

// ASCII Values for Special Symbols
//    THIS WILL NOT INCLUDE DIGITS
#define SYM_MIN 40
#define SYM_MAX 62

// just made a global counter
int temp;

// global reading buffer
char *buffer;
// global length of the buffer
int bufferLen = 0;

// Array to store the reserved words for PL/0
char resword[15][12] = {
	"const","var","procedure","call","begin","end",
	"if","then","else","while","do","read","write","odd"
};
// token values for the reserved words
int restoken[15] = {
  constsym, varsym, procsym, callsym, beginsym, endsym,
  ifsym, thensym, elsesym, whilesym, dosym, readsym, writesym, oddsym
};

// a lex table has a lexeme and a token
typedef struct{
  char *lexeme;
  int token;
}lex;

// resizeable lexTable
lex *lexTable;

// TableSize for the lexTable
int ts = 0;

// Error messages for the list that are to be detected
char lexError[4][22] = {
  "Var started with num\n", "Number too long\n", "Name too long\n", "Invalid symbol\n"
};

// This method checks if the char is a digit by referencing the ASCII Values
int isDigit( char test ){
	// get the integer(ASCII) value of the char and test if it is in digit bounds
	return( (int)test <= DIGIT_MAX && (int)test >= DIGIT_MIN );
}

// This method checks if the char is a letter by referencing the ASCII Values
int isLetter( char test ){
	// get the integer(ASCII) value of the char and test if it is in letter bounds
	if( (int)test <= ULETTER_MAX && (int)test >= ULETTER_MIN ) return 1;
	if( (int)test <= LLETTER_MAX && (int)test >= LLETTER_MIN ) return 1;
	return 0;
}

// This method checks if the char is a symbol by referencing the ASCII Values
int isSymbol( char test ){
	// get the integer(ASCII) value of the char and test if it is in symbol bounds
	if( (int)test <= SYM_MAX && (int)test >= SYM_MIN ) return 1;
	return 0;
}

// This method checks if the char is an invisible character with ASCII Values
int isInvis( char test ){
	if( (int)test == TAB ) return 1;
	if( (int)test == WHITE_SPACE ) return 1;
	if( (int)test == NEWLINE ) return 1;
	return 0;
}

// This method will add items to the lexemetable array and return if added 
int addToTable( FILE *clean, char *str, int token ){
  // reallocate space for a new item in the lex array
  lexTable = realloc( lexTable, ++ts * sizeof(lex) );

  // malloc and store the lexeme string
  lexTable[ts -1].lexeme = (char*)malloc( 12 );
  strcpy( lexTable[ts -1].lexeme, str );

  // store the token
  lexTable[ts -1].token = token;

  // add string to cleaninput.txt
	fprintf( clean, "%s", str );
  return 1;
}

// This method checks through all the reserved list for the str
int isReserved( FILE *clean, char *str ){
	for( temp = 0; temp < 15; temp++ )
    // if the str is found add it to the lexTable and cleaninput.txt
		if( !strcmp( resword[temp], str ) ) 
      return addToTable( clean, resword[temp], restoken[temp] );
  
	return 0;
}

// prints error message and cleans the rest of the file of comments
int error( FILE *input, FILE *clean, char c, char *buffer, int i ){
  printf( "%s", lexError[i] );
  fprintf( clean, "%s", buffer );
  while( c != EOF ){
    switch(c){

    case '/': // still clean input of comments 
      c = getc( input );
      switch(c){
      case '*': // '/*' multiline comment
        do{ // waits for another '/' after a '*'
          do{ // waits for another '*'
						c = getc( input ); 
					}while( c != '*' && c != EOF );
					c = getc( input );
				}while( c != '/' && c != EOF );
        break;

      case '/': // '//' singleline comment
        do{ // waits for a newline character
          c = getc( input );
        }while( c != NEWLINE && c != EOF );
        break;

      default: // '/' as in 'divides' add this one to the table
        addToTable( clean, "/", slashsym );
        ungetc( c, input );
      }
			break;
    default: // not a comment so just print away
      fprintf( clean, "%c", c );
    }
    c = getc( input );
  }
 
  return 0;
}

// This method will read the input file char by char
//   and will clean the input and build the lexTable
int execute( ){
  // open input.txt and cleaninput.txt
	FILE *input = fopen( "input.txt","r" );
	FILE *clean = fopen( "cleaninput.txt","w+" );

  // begin grabbing char by char
	char c = getc( input );
  buffer = (char *)malloc(13);

  // while there are characters left parse the file
	while( c != EOF ){    
		// makes sure the buffer is clean
    for( temp = 0; temp < 13; temp++ )
      buffer[temp] = '\0';
		
    // start building a word
    buffer[bufferLen] = c;

		if( isLetter(c) ){	// word starts with a letter

      // keep adding letters or digits to the word
      while( isLetter(c) || isDigit(c) ){
				buffer[bufferLen++] = c;
        // if the buffer word is too large give error
        if( bufferLen > 11 ) return error( input, clean, c, buffer, 2 );

				c = getc( input );
			}
      // now check if the found word is a reserved word
      if( !isReserved( clean, buffer ) )
        // add to lexTable as an identsym
        addToTable( clean, buffer, identsym );

      // unget one char
			ungetc( c, input );

		}else if( isDigit(c) ){ // word start with a digit 
      
      // keep adding digits to the word (number)
			while( isDigit(c) ){
				buffer[bufferLen++] = c;
        // if the buffer word (number) is no large give error
        if( bufferLen > 5 ) return error( input, clean, c, buffer, 1 );

				c = getc( input );
        // if the word starts with a digit and has a letter give error
        if( isLetter(c) ) return error( input, clean, c, buffer, 0 );

			}
      // add to lexTable as an numbersym
      addToTable( clean, buffer, numbersym );
 
      // unget one char and add buffer to cleaninput.txt
			ungetc( c, input );
			
		}else if( isSymbol(c) ){ // word is a symbol
			// use a switch to add the corresponding symbol to the lexTable	
      //   also add the symbol to cleaninput.txt
			switch(c){
      case '+':
        addToTable( clean, "+", plussym );
        break;
      case '-':
        addToTable( clean, "-", minussym );
        break;
      case '*':
        addToTable( clean, "*", multsym );
        break;   
      case '/': // two comment formatts that are left out of cleaninput.txt
				c = getc( input );
        switch(c){
        case '*': // '/*' multiline comment
          do{ // waits for another '/' after a '*'
						do{ // waits for another '*'
							c = getc( input ); 
						}while( c != '*' );
						c = getc( input );
					}while( c != '/' );
          break;

        case '/': // '//' singleline comment
          do{ // waits for a newline character
            c = getc( input );
          }while( c != NEWLINE );
          break;

        default: // '/' as in 'divides' add this one to the table
          addToTable( clean, "/", slashsym );
          ungetc( c, input );
        }
				break;
      case '(':
        addToTable( clean, "(", lparentsym );
        break;
      case ')':
        addToTable( clean, ")", rparentsym );
        break;
      case '=':
        addToTable( clean, "=", eqlsym );
        break;
      case ',':
        addToTable( clean, ",", commasym );
        break;
	    case '.':
        addToTable( clean, ".", periodsym );
        break;
	    case '<': 
        c = buffer[++bufferLen] = getc( input );
        switch( c ){
        case '=': // less than or equal to
          addToTable( clean, "<=", leqsym );
          break;
        case '>': // '<>' neqsym
          addToTable( clean, "<>", neqsym );
          break;
        default: // just less than
          addToTable( clean, "<", lessym );
        } 
        ungetc( c, input );
        break;
	    case '>': 
        c = buffer[++bufferLen] = getc( input );
        switch( c ){
        case '=': // greater than or equal to
          addToTable( clean, ">=", leqsym );
          break;
        default: // just greater than
          addToTable( clean, ">", lessym );
        } 
        ungetc( c, input );
        break;
      case ';':
        addToTable( clean, ";", semicolonsym );
        break;
	    case ':': // a colon is found check for an '=' 
        // if not bleed into the default case to send an error
        buffer[++bufferLen] = getc( input );
        if( !strncmp( buffer, ":=", 2 ) ){
          addToTable( clean, ":=", becomessym );
          break;
        }
			default:
       return error( input, clean, c, buffer, 3 );
			}
		}else if( isInvis(c) ){ // word is a white space
			// add the char to the buffer
      while( isInvis(c) ){
        buffer[0] = c;
        c = getc( input ); 
      }
      ungetc( c, input );
			fprintf( clean, "%s", buffer );
		}else
      return error( input, clean, c, buffer, 3 );

    // reset bufferLen and continue building char by char
		bufferLen = 0;
		c = getc( input );
	}
	fclose( input );
  fclose( clean );
  return 1;
}

// This method uses the lexTable to fill in the lexeme* files
void lexOutput(){
  FILE *tableP = fopen( "lexemetable.txt","w+" );
  FILE *listP = fopen( "lexemelist.txt","w+" );
  int token;

  fprintf( tableP, "lexeme \ttoken type\n" );
  // go through the whole table adding all items
  for( temp = 0; temp < ts; temp++ ){
    token = lexTable[temp].token;
    fprintf( tableP, "%s\t%i\n", lexTable[temp].lexeme, token );
    fprintf( listP, "%i ", token );
   
    // if the token is an identifier or number add the lexeme to the file
    if( token == identsym || token == numbersym )
      fprintf( listP, "%s ", lexTable[temp].lexeme );
  }
  fclose( tableP );
  fclose( listP );
}

int main(){
	execute(); 
  lexOutput();
	return 0;
}
