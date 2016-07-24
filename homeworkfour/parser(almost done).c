// Homework Three: Parser
// Ashton Ansag
// Harold Marcial

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**//* NOTE: this is for me to find reminders to be edited at a later time*/

// this is the defined max table size
#define MAX_SYMTABLE_SIZE 1000
#define MAX_CODE_LENGTH 500


// this is the enum and structs for the instruction opcodes
enum {
	LIT = 1, OPR, LOD, STO, CAL,
	INC, JMP, JPC, SIO
};

typedef struct ins{
	int op, l, m;
}ins;

int codeLine = 0;
ins code[MAX_CODE_LENGTH];
int space = 0;
int op, currentM, prevSym,jumpBack;

// enum with token values for the lexemes
enum {
	nulsym = 1, identsym, numbersym, plussym, minussym,
	multsym, slashsym, oddsym, eqlsym, neqsym,
	lessym, leqsym, gtrsym, geqsym, lparentsym,
	rparentsym, commasym, semicolonsym, periodsym, becomessym,
	beginsym, endsym, ifsym, thensym, whilesym,
	dosym, callsym, constsym, varsym, procsym,
	writesym, readsym, elsesym
};

// enum to show what kind the symbol is
enum { CONST = 1, VAR, PROC };

// this is the structure that defines a symbol
typedef struct symbol{
	int kind, val, level, addr;
	char name[12];
}symbol;

// this is the symbol table that will store all of the important symbols
symbol symbol_table[MAX_SYMTABLE_SIZE];

// a global symbol variable that will be used as a temp to be added to table
symbol s;

// a counter to show the amount of items in the symbol table
int symCounter = 0;

// this is to store the current token being evaulated
int currentToken = 0;

// variable use to keep track of the current lex depth in the code
int currLevel = 0;

// variable to keep the index of

// array of all the reserved word/symbols
char table[34][12] = {
	"\0","\0","var","num","+","-",
	"*","/","odd","=","!=",
	"<","<=",">",">=","(",
	")",",",";",".",":=",
	"begin","end","if","then","while",
	"do","call","Const","Var","Proc",
	"write","read","else"
};

// the printable error messages stored in an array
char ERROR_MESSAGE[26][60] = {
	"end expected.\n",
	"Use of = instead of :=.\n",
	"= must be followed by a number.\n",
	"Identifier must be followed by =.\n",
	"const, var, procedure must be followed by identifier.\n",
	"Semicolon or comma missing.\n",
	"Incorrect symbol after procedure declaration.\n",
	"Statement expected.\n",
	"Incorrect symbol after statement part in block.\n",
	"Period expected.\n",
	"Semicolon between statements missing.\n",
	"Undeclared identifier.\n",
	"Assignment to constant or procedure is not allowed.\n",
	"Assignment operator expected.\n",
	"call must be followed by an identifier.\n",
	"Call of a constant or variable is meaningless.\n",
	"then expected.\n",
	"Semicolon or } expected.\n",
	"do expected.\n",
	"Incorrect symbol following statement.\n",//19??
	"Relational operator expected.\n",
	"Expression must not contain a procedure identifier.\n",
	"Right parenthesis missing.\n",
	"The preceding factor cannot begin with this symbol.\n",
	"An expression cannot begin with this symbol.\n",
	"This number is too large.\n"
};

/**/// debugging string array
char identable[6][6] = { "\0", "const\0", "var\0", "proc\0" };

// temp char array to store identifer names
char *buffer;

// this will be the filepointer to lexemelist.txt
FILE *fp;


void gen( int op, int lex, int mcode ){

    if (codeLine > MAX_CODE_LENGTH){

        printf("Error! Can't have more than 500 instructions.");
        exit(EXIT_FAILURE);
    }

    printf("op: %d, lex: %d, m: %d\n", op, lex, mcode);
    // make the instruction with its code and add to code array
	ins instruction = { op, lex, mcode };
	code[codeLine++] = instruction;
}

void outputmCode(){
	FILE *mcodeTXT = fopen( "mcode.txt", "w+" );

	int i = 0;
	for( ; i < codeLine; i++ ){
		fprintf( mcodeTXT, "%i %i %i\n", code[i].op, code[i].l, code[i].m );
	}
	fclose( mcodeTXT );
}


// simple error printer that returns 0 which represents an error
int error( int number ){
	printf( "Error number %i, %s", number, ERROR_MESSAGE[number] );
	exit(EXIT_FAILURE);
	return 0;
}

// this table checks if the symbol is in the table, if not add to table
void addTo( symbol s ){
	int temp = 0;

	while( temp<symCounter ){
		if( !strcmp(s.name, symbol_table[temp++].name) ) return;
	}
	symbol_table[symCounter++] = s;

}

/* Returns 0 if not found, else s.kind
 * 1 = CONST
 * 2 = VAR
 * 3 = PROC
*/
int lookup( symbol s ){
	int temp = 0;

	while( temp < symCounter){
        if( !strcmp(s.name, symbol_table[temp++].name) ){
            return symbol_table[temp - 1].kind;
        }

    }

	return 0;
}

/* Returns the location of the symbol in the table
 * return -1 if not found
 */
int find( symbol s ){
	int temp = symCounter;
	while( temp > 0 ){
		if( !strcmp(s.name, symbol_table[--temp].name) ) return temp;
	}
	return -1;
}

// functional prototypes because some of them depend on each other
int program(), block(), condition(), relation();
int constdec(), vardec(), procdec(), statement();
int expression(), term(), factor();

// this is a function to grab the next token from the lexemelist.txt
void getToken(){
	fscanf( fp, "%s ", buffer );
	currentToken = atoi(buffer);
	printf( "(%s,%i)\n", buffer,currentToken );
}

/* checks program syntax:
 *  - 1 block
 *  - 1 '.'
 */
int program(){
	getToken();
	if( block() ){
		if( currentToken != periodsym )
            return error(9);
		else
            printf( "No errors, program is syntactically correct." );
    }
	return 1;
}

/* checks block syntax:
 *  - 1 constant  declaration (or none)
 *  - 1 variable  declaration (or none)
 *  - 1 procedure declaration (or none)
 *  - 1 statement             (or none)
 */
int block(){


	int jmpAddr = codeLine;
	int tempPos;
	currentM = 0;
	space = 4;
	gen(JMP,0,0);

    do{
	if( currentToken == constsym ){
		// the token is a constant symbol, test this declaration
		if( !constdec() ) return 0;
	}

	if( currentToken == varsym ){
		// the token is a variable symbol, test this declaration
		if( !vardec() ) return 0;
	}
	}while(currentToken == constsym || currentToken == varsym);

	tempPos = currentM;

	while( currentToken == procsym ){
		// the token is a procedure symbol, test this declaration
		if( !procdec() ) return 0;
	}



	code[jmpAddr].m = codeLine;

	gen(INC, 0, tempPos + 4);

	// now test the statement at the end of the block
	statement();

	if(currentToken == semicolonsym){
        gen(OPR, 0, 0);
        return 1;
	}
	else if(currentToken == periodsym){
        gen(11, 0, 3);
        return 1;
	}
	else{
       return error(6);
	}




	return 1;
}
/* checks constant declaration syntax:
 *  - 1 "constsym" token (already found before function call)
 *  - 1 "identsym" token (or more seperated by commas)
 *    containing:
 *    - 1 "eqlsym" token
 *    - 1 "numbersym" token (which is not more than 5 digits)
 *  - 1 "semicolonsym" token
 */
int constdec(){

	// set the temp symbol to have CONST kind value and clear fields
	//   the fields will be refilled in as the information is parsed
	s.kind = CONST;
	s.val = s.level = s.addr = 0;

	// loop till "ident eql number semicolon" pattern is found
	//   note: commas can replace a semicolon and restart the pattern without error
	do{

		// get token and test for first pattern piece "identsym"
		getToken();
		if( currentToken != identsym ) return error(4);     // no ident error

		// an identifier was found the next token is its name
		getToken();
		strcpy( s.name, buffer );
		s.level = currLevel;

		// get token and test for second pattern piece "eqlsym"
		getToken();
		if( currentToken != eqlsym ){
			if( currentToken == becomessym )
                return error(1); // := not = error

			else
                return error(3);                             // anyother error
		}


		// get token and test for third pattern piece "numbersym"
		getToken();
		if( currentToken != numbersym ) return error(2);    // not a num error

		// a number was found the next token is its value
		getToken();
		if( currentToken >= 100000 ) return error(25);      // num too large error
		s.val = currentToken;

		// we reached the end of one constant declaration (try to) add to table
		addTo( s );

		// get the next token to decide if the declaration statement continues
		getToken();

	}while( currentToken == commasym ); // seperated by commas

	if( currentToken != semicolonsym ) return error(5); // expected ; error

	// done with declaration statement get the next token
	getToken();
	return 1;
}

/* checks variable declaration syntax:
 *  - 1 "varsym" token (already found before function call)
 *  - 1 "identsym" token (or more seperated by commas)
 *  - 1 "semicolonsym" token
 */
int vardec(){


	// set the temp symbol to have VAR kind value and clear fields
	//   the fields will be refilled in as the information is parsed
	s.kind = VAR;
	s.val = s.level = s.addr = 0;

	// loop till "ident semicolon" pattern is found
	//   note: commas can replace a semicolon and restart the patter without error
	do{

		// get token and test for the first pattern piece "identsym"
		getToken();
		if( currentToken != identsym ) return error(4);     // no ident error



		// an identifier was found the next token is its name
		getToken();
		strcpy( s.name, buffer );
		s.level = currLevel;
		s.addr = currentM+4;


		// we reached the end of one variable declaration (try to) add to table
		addTo( s );
		currentM++;

		// get the next token to decide if the declaration statement continues
		getToken();

	}while( currentToken == commasym ); // seperated by commas

	if(currentToken == identsym){
        error(5);       //semicolon between statements
	}

	if( currentToken != semicolonsym ) return error(5); // expected ; error

	getToken();
	return 1;
}

/* checks procedure declaration syntax:
 *  - 1 "procsym" token (already found before function call)
 *  - 1 "identsym" token
 *  - 1 "semicolonsym" token
 *  - 1 block
 *  - 1 "semicolonsym" token
 */
int procdec(){

	// set the temp symbol to have PROC kind value and clear fields
	//   the fields will be refilled as the information is parsed
	s.kind = PROC;
	s.val = s.level = s.addr = 0;

	printf("PROCEDURE!! YUUSSS\n\n");



    // get token and test for first pattern piece "identsym"
    getToken();
    if( currentToken != identsym ) return error(4);     // no ident error

    // an identifier was found the next token is its name
    getToken();
    strcpy( s.name, buffer );


    // get token and test for second pattern piece "semicolonsym"
    getToken();
    if( currentToken != semicolonsym ) return error(5); // expect ; error

    // we move inside a procedure and down a level
    s.level = currLevel;
    s.addr = codeLine;


    // add to table
    addTo( s );

    // get token and test this block
    getToken();

    // checking if the next token is the right symbol
    if(
        currentToken != constsym &&
        currentToken != varsym &&
        currentToken != procsym &&
        currentToken != identsym &&
        currentToken != callsym &&
        currentToken != beginsym &&
        currentToken != ifsym &&
        currentToken != whilesym &&
        currentToken != readsym &&
        currentToken != writesym)
        return error(6);   //wrong symbol after procedure declaration error

    currLevel++;
    if( !block() ) return 0;

    // the procedure must end with a semicolon
    if( currentToken != semicolonsym ) return error(5); // expect ; error

    //we finished parsing in the procedure move back up a level
    currLevel--;

    // get token decided if there is another procedure to be declared
    getToken();


	return 1;
}

/* checks the statement syntax:
 *  - multiple patterns:
 *    i.   "ident becomes expression()"
 *    ii.  "call ident"
 *    iii. "begin statement()" (any number of: "semicolon statement()") "end"
 *    iv.  "if condition() then statement()" (optional: "else statement()")
 *    v.   "while condition() do statement()"
 *    vi.  "read ident"
 *    vii. "write ident"
 */
int statement(){

	int lookUp;
	int jumpIndex1 = 0;
	int jumpIndex2 = 0;

	// test for first pattern piece "identsym"
	if( currentToken == identsym ){

		// an identifier was found the next token is its name
		getToken();
		strcpy(s.name, buffer);

		lookUp = lookup(s);
		int location = find(s);
		if( location == -1 ) return error(11);  //Undeclared identifier

		if( symbol_table[location].kind != VAR ) return error(12); //not a var
/*
		switch(lookUp){
			case 0:
				error(11);      //Undeclared identifier.
				break;
			case 1:
				error(12);      //Assignment is a constant.
				break;
			case 3:
				error(12);      //Assignment is a procedure.
				break;
		}
*/

		// get token and test for second pattern piece "becomessym"
		getToken();
		if( currentToken != becomessym ) return error(13);   // Assignment operator.


		// get token and test the expression
		getToken();
		if( !expression() ) return 0;

		gen( STO, currLevel - symbol_table[location].level, symbol_table[location].addr );


	// test for first pattern piece "callsym"
	}else if( currentToken == callsym ){

		// get token and test for second pattern piece "identsym"
		getToken();
		if( currentToken != identsym ) return error(14);	  // call must be followed by identifier

        // check if we are calling a procedure.
        getToken();

		strcpy(s.name, buffer);
		lookUp = lookup(s);
		int location;

		switch(lookUp){
            case 0:
                error(11);      //Undeclared identifier.
                break;
            case 1:
                error(15);      //Calling a constant
                break;
            case 2:
                error(15);      //Calling a variable
                break;
            case 3:
                location = find(s);
                gen(CAL, currLevel - symbol_table[location].level, symbol_table[location].addr);
                break;
       }

       getToken();


	// test for first pattern piece "beginsym"
	}else if( currentToken == beginsym ){



		// get token and test the statement
		getToken();
		if( !statement() ) return error(7);  // a correct statement was expected


		// while optional "semicolonsym" is found then keep testing statements
		while( currentToken	== semicolonsym ){

			// get token and test the statement
			getToken();
			if( !statement() ) return error(7);	// a correct statement was expected
		}

		 //test for the last pattern piece "endsym"
		if( currentToken != endsym ){
            return error(17);       // no end error
		}

		printf("done with begin/end\n");

		// get token and move on
		getToken();


	// test for first pattern piece "ifsym"
	}else if( currentToken == ifsym ){

		// get token and test the condition
		getToken();
		if( !condition() ) return 0;	                      // error

		// test for the third pattern piece "thensym"
		if (currentToken != thensym)
            error(16);

        getToken();



		jumpIndex1 = codeLine;
		gen(JPC, 0, 0);
		if( !statement() ) return error(7);	                      // error statement expected

        printf("if/else currentToken %d\n", currentToken);

		// get to adding something for an else
		if( currentToken == elsesym){

            getToken();

            code[jumpIndex1].m = codeLine+1;
            jumpIndex1 = codeLine;
            gen(JMP,0,0);

            if(!statement()) return error(7);                      //error statement expected

		}

        code[jumpIndex1].m = codeLine;


	// test for first pattern piece "whilesym"
	}else if( currentToken == whilesym){

	    jumpIndex1 = codeLine;

		// get token and test the condition
		getToken();
		if( !condition() ) return 0;	                      // error

        jumpIndex2 = codeLine;
        gen(JPC,0,0);
		// test for the third pattern piece "dosym"
		if( currentToken != dosym ) return error(18);	      // no do error

		// get token and test the statement
		getToken();
		if( !statement() ) return 0;	                      // error

		gen(JMP, 0, jumpIndex1);
		code[jumpIndex2].m = codeLine;

	// test for first pattern piece "readsym"
	}else if( currentToken == readsym ){

		// get token and test for second pattern piece "identsym"
		getToken();
		if( currentToken != identsym ) return error(4);	  // no ident error
		getToken();

		//SIO - read
		gen(10, 0, 2);

		//find the variable and STO it
        strcpy(s.name, buffer);

        int location = find(s);

        if( location == -1 ) return error(11);  //Undeclared identifier

		if( symbol_table[location].kind != VAR ) return error(12); //not a var

        gen(STO, currLevel - symbol_table[location].level, symbol_table[location].addr);

		getToken();

	// test for first pattern piece "writesym"
	}else if( currentToken == writesym ){

		// get token and test for second pattern piece "identsym"
		getToken();
		if( currentToken != identsym ) return error(4);	  // no ident error

        //STO the variable
       if(!expression()) return 0; //error

		//SIO - write
		gen(9, 0, 1);

	}

	return 1;
}

/* checks expression syntax:
 *  - (optional: "(plussym or minussym)" )
 *    - 1 "term"
 *    - (any number of: "(plus or minus) term" )
 */
int expression(){

    int type = 0;
	// test for optional "plussym" or "minussym"
	if( currentToken ==	plussym || currentToken == minussym ) {
            type = currentToken;
            getToken();

	}

	if( currentToken != numbersym && currentToken != identsym && currentToken != lparentsym){
        error(24); //expression cannot begin with this symbol error
	}

	// test this term
	if( !term() ) return 0;                               // error

	// while option "plussym" or "minussym" is found keep testing terms
	while( currentToken == plussym || currentToken == minussym ){
        int type = currentToken;
		getToken();
		if( !term() ) return 0;                             // error

		if(type == plussym){
            //ADD
            gen(OPR, 0, 2);
		}
		else if(type == minussym){
            //SUB
            gen(OPR, 0, 3);
		}
	}

	//This generates when the expression starts with + or -.
	if(type != 0){
        if(currentToken == plussym){
            //ADD
            gen(OPR, 0, 2);
        }
        else{
            //SUB
            gen(OPR, 0, 1);
        }
	}

	return 1;
}

/* checks condition syntax:
 *  - multiple patterns
 *    i.    "odd expression()"
 *    ii.   "expression() relation() expression()"
 */
int condition(){

	// test for first pattern piecet "oddsym"
	if( currentToken == oddsym ){

		// get token and test the expression()
		getToken();
		if( !expression() ) return 0;                       // error

		//ODD
		gen(OPR, 0, 6);

	}else{

		// test the expression
		if( !expression() ) return 0;                       // error

        op = currentToken;

		// get token and test the expression
		getToken();
		if( !expression() ) return 0;                       // error

		// test the relation
		if( op != relation() ) return error(20);	// rel-op error
	}
	return 1;
}

/* checks the term syntax:
 *  - 1 factor
 *  - (any number of: "("multsym" or "slashsym") factor" )
 */
int term(){

    int mulop;
	// test the factor
	if( !factor() ) return 0;                             // error

	// while optional "multsym" or "slashsym" found test the factor
	while( currentToken == multsym || currentToken == slashsym ){
        mulop = currentToken;

		// get token and test
		getToken();
		if( !factor() ) return 0;                           // error

		if(mulop == multsym){
            //MUL
            gen(OPR, 0, 4);
		}
		else{
            //DIV
            gen(OPR, 0, 5);
		}

	}
	return 1;
}

/* checks the factor syntax:
 *  - multiple patterns:
 *    i.   "ident"
 *    ii.  "number"
 *    iii. "lparent expression() rparent"
 */
int factor(){

    int location = 0;


	// test for first patterm piece "identsym"
	if( currentToken == identsym ){
		getToken();
		strcpy(s.name, buffer);
        int lookUp = lookup(s);

        switch(lookUp){

            case 0:
                error(11);      //Undeclared identifier.
                break;
            case 1:
                //What to do when it's a const - LIT
                location = find(s);
                gen(LIT, 0, symbol_table[location].val);
                break;
            case 2:
                //What to do when it's a var - LOD
                location = find(s);
                gen(LOD, currLevel  - symbol_table[location].level, symbol_table[location].addr);
                break;
            case 3:
                error(21);      //Assignment is a procedure.

           }

	// test for first pattern piece "number"
	}else if( currentToken == numbersym ){

		// a number is found the next number is its value
		getToken();
		if( currentToken >= 100000 ) return error(25);      // num too large error

        //LIT
	    gen(LIT, 0, currentToken);

	// test for first pattern piece "lparentsym"
	}else if( currentToken == lparentsym ){

		// get token and test the expression
		getToken();
		if( !expression() ) return 0;                       // error

		// test for last pattern piece "rparentsym"
		if( currentToken != rparentsym ) return error(22);  // missing ) error

		// get token carry on
		//getToken();

	}else	return error(23);                               // bad factor error

	// get token carry on
	getToken();
	return 1;
}

// test if currentToken is a relational operator
int relation(){
	switch(op){
	case eqlsym: // equal
	    gen(OPR, 0, 8);
	    break;
	case neqsym: // not equal
	    gen(OPR, 0, 9);
	    break;
	case lessym: // less than
	    gen(OPR, 0, 10);
	    break;
	case leqsym: // less than equal to
	    gen(OPR, 0, 11);
	    break;
	case gtrsym: // greater than
	    gen(OPR, 0, 12);
	    break;
	case geqsym: // greater than equal to
	    gen(OPR, 0, 13);
	    break;
	default:
		return error(20);                                   // rel-op error
	}

	return op;
}

int main( int argc, char *argv[] ){

	/**/// simply way to print lexemelist if run with -l*
	if( argc > 1 ){
		if( argv[1][0] == '-' && argv[1][1] == 'l' ){
			fp = fopen( "lexemelist.txt","r" );
			buffer = (char *)malloc( 10000 );
			fscanf(fp, "%[^\n]\n", buffer );
			printf("Lexemelist:\n%s\n\n", buffer );
			fclose( fp );
		}
	}

	fp = fopen( "lexemelist.txt","r" );
	int i;
	buffer = (char *)malloc(12);

	program();

    //Printing the symbol list
    FILE *ofp = fopen("symlist.txt", "w");
	fprintf( ofp, "Name\tType\tLevel\tValue\n" );
	for( i=0; i<symCounter; i++ ){
		s = symbol_table[i];
	fprintf( ofp, "%s\t%s\t%i\t%i\n", s.name, identable[s.kind], s.level, s.addr );
	}
	printf( "\n" );

    //closure of the files.
	fclose( fp );
	fclose ( ofp );
	outputmCode();


	return 0;
}
