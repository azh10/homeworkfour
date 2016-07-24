#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAX_STACK_HEIGHT 2000
#define MAX_CODE_LENGTH  500
#define MAX_LEXI_LEVELS  3

// a temp, basepointer, stackpointer, programcounter, num of activation records
int tmp, bp = 1, sp = 0, pc = 0, ar = 0;

// a stack, and a marker to place |'s for activation records
int stack[MAX_STACK_HEIGHT], marker[4];

// labels for the instructions
char ops[13][4] = {
	"\0",    "LIT\0", "OPR\0", "LOD\0", 
	"STO\0", "CAL\0", "INC\0", "JMP\0", 
	"JPC\0", "SIO\0", "SIO\0", "SIO\0"
	};

// labels for the operators
char oprs[14][6] = {
	"(RET)\0", "(NEG)\0", "(ADD)\0", "(SUB)\0", 
	"(MUL)\0", "(DIV)\0", "(ODD)\0", "(MOD)\0", 
	"(EQL)\0", "(NEQ)\0", "(LSS)\0", "(LEQ)\0", 
	"(GTR)\0", "(GEQ)\0"
};

// struction for how instructions are broken-up
typedef struct instruction{
	int op, l, m;
}instruction;

// store all instructions in an array, and store current instruction
instruction ins[MAX_CODE_LENGTH], ir;

// read and load the instuctions into an instruciion array
void read(){

	// set needed zeros
	for( tmp = 1; tmp < 4; tmp++ ){
		stack[tmp] = 0; 
		marker[tmp] = 0;
	}
	FILE *fp = fopen( "mcode.txt", "r" );

	// read and output each instruction as it is read
	tmp = -1;
	printf( "\nLine\tOP\tL\tM\n" );
	while( !feof(fp) ){
		fscanf( fp, "%i ", &ins[++tmp].op );
		fscanf( fp, "%i ", &ins[tmp].l  );
		fscanf( fp, "%i ", &ins[tmp].m  );
		ir = ins[tmp];
		printf( "%3i\t%s\t%i\t%i\%s\n", tmp, ops[ir.op], ir.l, ir.m, ((ir.op == 2)?oprs[ir.m]:" ") );
	}
	fclose(fp);	
}

// print the pc, bp, sp, and the stack
void print(){
	printf( " %3i  %3i %3i\t", pc, bp, sp );
	for( tmp = 1; tmp <= sp; tmp++ )
		if( tmp!=1 && (marker[1]==tmp-1 || marker[2]==tmp-1 || marker[3]==tmp-1) )
			printf( "| %i ", stack[tmp] );
		else printf( "%i ", stack[tmp] );

	// if this opr needs user input take it
	if( ir.op == 10 ){
		printf( ">> " );
		scanf(  "%i", &stack[tmp] );
		sp++;
	} else printf( "\n" );
}

// given way to find the base at a level
int base( int l ){
	int b1 = bp;
	while( l-- > 0 )
		b1 = stack[b1 + 1];
	
	return b1;
}

// take the instruction and execute it
void execute( int op, int l, int m ){

	printf( "%3i  %s %3i %3i", pc++, ops[op], l, m );
	switch( op ){
	case 1: // lit
		stack[++sp] = m;
		break;
	case 2: // opr
		switch( m ){
		case 0: // ret
			sp = bp - 1;
			pc = stack[sp + 4];
			bp = stack[sp + 3];
			marker[ar--] = 0;
			break;
		case 1: // neg
			stack[sp] = -stack[sp];
			break;
		case 2: // add
			stack[--sp] = stack[sp] + stack[sp + 1];
			break;
		case 3: // sub
			stack[--sp] = stack[sp] - stack[sp + 1];
			break;
		case 4: // mul
			stack[--sp] = stack[sp] * stack[sp + 1];
			break;
		case 5: // div 
			stack[--sp] = stack[sp] / stack[sp + 1];
			break;
		case 6: // odd
			stack[sp] = stack[sp] % 2;
			break;
		case 7: // mod
			stack[--sp] = stack[sp] % stack[sp + 1];
			break;
		case 8: // eql
			stack[--sp] = stack[sp] == stack[sp + 1];
			break;
		case 9: // neq 
			stack[--sp] = stack[sp] != stack[sp + 1];
			break;
		case 10: // lss 
			stack[--sp] = stack[sp] < stack[sp + 1];
			break;
		case 11: // leq
			stack[--sp] = stack[sp] <= stack[sp + 1];
			break; 
		case 12: // gtr
			stack[--sp] = stack[sp] > stack[sp + 1];
			break;
		case 13: // geq
			stack[--sp] = stack[sp] >= stack[sp + 1];
			break;
		}
		break;
	case 3: // lod
		stack[++sp] = stack[base( l ) + m];
		break;
	case 4: // sto
		stack[ base( l ) + m ] = stack[sp--];
		break;
	case 5: // cal
		marker[++ar] = sp;
		stack[sp + 1] = 0;
		stack[sp + 2] = base( l );
		stack[sp + 3] = bp;
		stack[sp + 4] = pc;
		bp = sp + 1;
		pc = m;
		break;
	case 6: // inc
		sp += m;
		break;
	case 7: // jmp
		pc = m;
		break;
	case 8: // jpc
		if( stack[sp--] == 0 )	pc = m;
		break;
	case 9: // sio 
		stack[sp--];
		break;
	case 10: // sio
		break;
	case 11: // halt
		// set everything to 0 
		printf( "\nSuccessfully halted.\n" );
		bp = sp = pc = ir.op = ir.l = ir.m = 0;
		return;
	}
	print();
}

// read input, make headers, execute each instruction, until halt or return
int main(void){

	read();
	printf( "\n\t\t  pc   bp  sp\tstack\n" );
	printf( "Initial values    %2i  %3i %3i\n", pc, bp, sp );
	while( bp != 0 ){
		ir = ins[pc];
		execute( ir.op, ir.l, ir.m );
	}
	return 0;
}
