/*************************************************************
Filename: scanner.c
Compiler: MS Visual Studio 2019
Author: Matthew Leblanc 040961708
		Yuliia Topalova 040981104
Course: CST 8152 – Compilers, Lab Section: 012
Assignment: 2.
Date: March 20, 2021
Professor: Abdulah Kadri, Paulo Sousa.
Purpose: Functions implementing a Lexical Analyzer (Scanner).
Function list: int startScanner, Token tokenizer, int nextState,
			   int nextClass, Token funcAVID, Token funcSVID, 
			   Token funcIL, Token funcFPL, Token funcSL, 
			   Token funcErr, int isKeyword;
*/

 /* The #define _CRT_SECURE_NO_WARNINGS should be used in MS Visual Studio projects
  * to suppress the warnings about using "unsafe" functions like fopen()
  * and standard sting library functions defined in string.h.
  * The define does not have any effect in Borland compiler projects.
  */
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>   /* standard input / output */
#include <ctype.h>   /* conversion functions */
#include <stdlib.h>  /* standard library functions and constants */
#include <string.h>  /* string functions */
#include <limits.h>  /* integer types constants */
#include <float.h>   /* floating-point types constants */

  /*#define NDEBUG        to suppress assert() call */
#include <assert.h>  /* assert() prototype */

/* project header files */
#include "buffer.h"
#include "token.h"
#include "table.h"

#define DEBUG  /* for conditional processing */
#undef  DEBUG

/* Global objects - variables */
/* This buffer is used as a repository for string literals.
   It is defined in platy_st.c */
extern bPointer stringLiteralTable;		/* String literal table */
int line;								/* current line number of the source code */
extern int errorNumber;					/* defined in platy_st.c - run-time error number */

static char debugMode = 0;				/* optional for debugging */

/* Local(file) global objects - variables */
static bPointer lexemeBuffer;			/* pointer to temporary lexeme buffer */
static bPointer sourceBuffer;			/* pointer to input source buffer */
/* No other global variable declarations/definitiond are allowed */

/* scanner.c static(local) function  prototypes */
static int nextClass(char c);			/* character class function */
static int nextState(int, char);		/* state machine function */
static int isKeyword(char kw_lexeme[]);	/* keywords lookup function */

/************************************************************
*   Function name: startScanner
*   Purpose: to intitialize the scanner
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: bRewind, bClean
*   Parameters: bPointer psc_buf
*   Return Value: EXIT_FAILURE, EXIT_SUCCESS
*   Algorithm: This function initializes the 
			   scanner using defensive programming
**************************************************************/
int startScanner(bPointer psc_buf) {
	if (bIsEmpty(psc_buf))
		return EXIT_FAILURE; /*1*/
	/* in case the buffer has been read previously  */
	bRewind(psc_buf);
	bClean(stringLiteralTable);
	line = 1;
	sourceBuffer = psc_buf;
	return EXIT_SUCCESS; /*0*/
}

/************************************************************
*   Function name: Token tokenizer
*   Purpose: token driver scanner and transition table driver scanner
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: bGetCh, bRetract, bSetMarkOffset, bGetChOffset, bCreate, bFinish
*   Parameters: void
*   Return Value: currentToken
*   Algorithm: in the first part is reading a specific sequence from buffer, 
				token switching cases; 
			   in the second part a pattern is recognized and the appropriate 
			   function is called, token in default case.
**************************************************************/
Token tokenizer(void) {
	Token currentToken = { 0 }; /* token to return after pattern recognition. Set all structure members to 0 */
	unsigned char c;	/* input symbol */
	int state = 0;		/* initial state of the FSM */
	short lexStart;		/* start offset of a lexeme in the input char buffer (array) */
	int lexLength;
	short lexEnd;		/* end offset of a lexeme in the input char buffer (array)*/

	int i;				/* counter */
	char getChar;

	/* endless loop broken by token returns it will generate a warning */
	while (1) { 
		c = bGetCh(sourceBuffer);

		/* ------------------------------------------------------------------------
			Part 1: Implementation of token driven scanner.
			Every token is possessed by its own dedicated code
			-----------------------------------------------------------------------
		*/

		if (isspace(c)) {
			if (c == '\n') {
				line++;
			}
			continue;
		}

		switch (c) {
		case ' ': case '\t': case '\v': case '\f': case '\r': /*White-space characters*/
			continue;
		case '\n': /*New Line*/
			line++;
			continue;
		
		/*Arithmetic Operator Characters*/
		case '+':
			getChar = bGetCh(sourceBuffer);
			if (getChar == '+') {
				currentToken.code = SCC_OP_T;
				return currentToken;
			}
			else {
				bRetract(sourceBuffer);
				currentToken.code = ART_OP_T;
				currentToken.attribute.arithmeticOperator = ADD;
				return currentToken;
			}
		case '-':
			currentToken.code = ART_OP_T;
			currentToken.attribute.arithmeticOperator = SUB;
			return currentToken;
		case '*':
			currentToken.code = ART_OP_T;
			currentToken.attribute.arithmeticOperator = MUL;
			return currentToken;
		case '/':
			currentToken.code = ART_OP_T;
			currentToken.attribute.arithmeticOperator = DIV;
			return currentToken;

		/*Relational Operators*/
		case '<':
			currentToken.code = REL_OP_T;
			currentToken.attribute.relationalOperator = LT;
			return currentToken;
		case '>':
			currentToken.code = REL_OP_T;
			currentToken.attribute.relationalOperator = GT;
			return currentToken;
		case '=':
			if (bGetCh(sourceBuffer) == '=') {
				currentToken.code = REL_OP_T;
				currentToken.attribute.relationalOperator = EQ;
				return currentToken;
			}
			else {
				bRetract(sourceBuffer);
				currentToken.code = ASS_OP_T;
				return currentToken;
			}

		/*Logical Operators*/
		case '.':
			bSetMarkOffset(sourceBuffer, bGetChOffset(sourceBuffer));
			getChar = bGetCh(sourceBuffer); // A or N or O 

			if (getChar == 'A' && bGetCh(sourceBuffer) == 'N' && bGetCh(sourceBuffer) == 'D' && bGetCh(sourceBuffer) == '.') {
				currentToken.code = LOG_OP_T;
				currentToken.attribute.logicalOperator = AND;
				return currentToken;
			}
			else if (getChar == 'N' && bGetCh(sourceBuffer) == 'O' && bGetCh(sourceBuffer) == 'T' && bGetCh(sourceBuffer) == '.') {
				currentToken.code = LOG_OP_T;
				currentToken.attribute.logicalOperator = NOT;
				return currentToken;
			}
			else if (getChar == 'O' && bGetCh(sourceBuffer) == 'R' && bGetCh(sourceBuffer) == '.') {
				currentToken.code = LOG_OP_T;
				currentToken.attribute.logicalOperator = OR;
				return currentToken;
			}
			else {
				bRestore(sourceBuffer);
				currentToken.code = ERR_T;
				currentToken.attribute.errLexeme[0] = '.';
				currentToken.attribute.errLexeme[1] = CHARSEOF0;
				return currentToken;
			}

		/*Seperators*/
		case '(':
			currentToken.code = LPR_T;
			return currentToken;
		case ')':
			currentToken.code = RPR_T;
			return currentToken;
		case '{':
			currentToken.code = LBR_T;
			return currentToken;
		case '}':
			currentToken.code = RBR_T;
			return currentToken;
		case ',':
			currentToken.code = COM_T;
			return currentToken;
		case ';':
			currentToken.code = EOS_T;
			return currentToken;

		case CHARSEOF0:
			currentToken.code = SEOF_T;
			currentToken.attribute.seofType = CHARSEOF0;
			return currentToken;

		case CHARSEOF255:
			currentToken.code = SEOF_T;
			currentToken.attribute.seofType = CHARSEOF255;
			return currentToken;

		case '!':
			getChar = bGetCh(sourceBuffer);
			if (getChar == '=') {
				currentToken.code = REL_OP_T;
				currentToken.attribute.relationalOperator = NE;
				return currentToken;
			}
			bRetract(sourceBuffer);
			currentToken.code = ERR_T;
			currentToken.attribute.errLexeme[0] = '!';
			currentToken.attribute.errLexeme[1] = '\0';
			return currentToken;

		/*Comments*/
		case '%':
			getChar = bGetCh(sourceBuffer);

			/*Check if it's a comment*/
			if (getChar == '%') {
				while (getChar != CHARSEOF0 && getChar != CHARSEOF255 && getChar != '\n') {
					getChar = bGetCh(sourceBuffer);
				}
				bRetract(sourceBuffer);
				continue;
			}
			else {
				bRetract(sourceBuffer);
				currentToken.code = ERR_T;
				currentToken.attribute.errLexeme[0] = '%';
				currentToken.attribute.errLexeme[1] = CHARSEOF0;
				return currentToken;
			}

		/*Error cases*/
		case RT_FAIL_2:
			errorNumber = 1;
			currentToken.code = RTE_T;
			strcpy(currentToken.attribute.errLexeme, "RUN TIME ERROR: ");
			return currentToken;

		default:
			/* ------------------------------------------------------------------------
				Part 2: Implementation of Finite State Machine (DFA)
						   or Transition Table driven Scanner
						   Note: Part 2 must follow Part 1 to catch the illegal symbols
				-----------------------------------------------------------------------*/

			state = nextState(state, c);
			lexStart = bGetChOffset(sourceBuffer) - 1;
			bSetMarkOffset(sourceBuffer, lexStart);

			/*Check for run-time error*/
			if (lexStart == RT_FAIL_1) {
				errorNumber = 1;
				strcpy(currentToken.attribute.errLexeme, "RUN TIME ERROR: ");
				currentToken.code = RTE_T;
				return currentToken;
			}

			/*FSM0. Start with state = 0 and input c. Can be Letter, Digit or String opening double quote*/
			/*FSM1. Get next transition table state*/
			/*FSM2. Use the transitionTable to get the type of state (NOAS, ASWR, ASNR)*/
			while (stateType[state] == NOAS) {
				getChar = bGetCh(sourceBuffer);
				state = nextState(state, getChar);
			}

			/*F3M3. If state is accepting state then found a token, therefore leave the machine and call function*/
			/*If the state is ASWR, retract function is called beforehand*/
			if (stateType[state] == ASWR) {
				bRetract(sourceBuffer);
			}

			/*Set lexEnd*/
			lexEnd = bGetChOffset(sourceBuffer);
			lexLength = lexEnd - lexStart;

			/*Create a temp buffer for lexeme*/
			lexemeBuffer = bCreate((short)lexLength, (short)state, 'f');

			/*Check for run-time error*/
			/*if (lexemeBuffer == NULL) {
				errorNumber = 1;
				strcpy(currentToken.attribute.errLexeme, "RUN TIME ERROR: ");
				currentToken.code = RTE_T;
				return currentToken;
			}*/

			/*Restore getchoffset*/
			bRestore(sourceBuffer);

			/*Loop between lexStart and lexEnd from the input buffer*/
			for (i = 0; i < lexLength; i++) {
				getChar = bGetCh(sourceBuffer);
				bAddCh(lexemeBuffer, getChar);
				/*if (bAddCh(lexemeBuffer, c) == NULL) {
					currentToken.code = RTE_T;
					strcpy(currentToken.attribute.errLexeme, "RUN TIME ERROR: ");
					errorNumber = 1;
				}*/
			}

			/*Add SEOF to the lexeme buffer end*/
			bFinish(lexemeBuffer, CHARSEOF0);

			/*Call accepting function using finalStateTable[]*/
			currentToken = (*finalStateTable[state])(bGetContent(lexemeBuffer, 0));

			bFree(lexemeBuffer);
			return currentToken;
		}
	}
}

/* DO NOT MODIFY THE CODE / COMMENT OF THIS FUNCTION */
/*************************************************************
 * Get Next State
	The assert(int test) macro can be used to add run-time diagnostic to programs
	and to "defend" from producing unexpected results.
	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	(*) assert() is a macro that expands to an if statement;
	if test evaluates to false (zero) , assert aborts the program
	(by calling abort()) and sends the following message on stderr:
	(*) Assertion failed: test, file filename, line linenum.
	The filename and linenum listed in the message are the source file name
	and line number where the assert macro appears.
	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	If you place the #define NDEBUG directive ("no debugging")
	in the source code before the #include <assert.h> directive,
	the effect is to comment out the assert statement.
	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	The other way to include diagnostics in a program is to use
	conditional preprocessing as shown bellow. It allows the programmer
	to send more details describing the run-time problem.
	Once the program is tested thoroughly #define DEBUG is commented out
	or #undef DEBUF is used - see the top of the file.
 ************************************************************/

int nextState(int state, char c) {
	int col;
	int next;
	col = nextClass(c);
	next = transitionTable[state][col];
#ifdef DEBUG
	printf("Input symbol: %c Row: %d Column: %d Next: %d \n", c, state, col, next);
#endif
	assert(next != IS);
#ifdef DEBUG
	if (next == IS) {
		printf("Scanner Error: Illegal state:\n");
		printf("Input symbol: %c Row: %d Column: %d\n", c, state, col);
		exit(1);
	}
#endif
	return next;
}

/************************************************************
*   Function name: nextClass
*   Purpose: to identify the column number in the transition table
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: isalpha, isdigit
*   Parameters: char c
*   Return Value: val
*   Algorithm: checks each column of the array and the assigned value for it;
*			   return specific value for each character.			
**************************************************************/
int nextClass(char c) {
	int val = -1;

	if (isalpha(c))
		val = 0;
	else if (isdigit(c))
		val = 1;
	else if (c == '.')
		val = 2;
	else if (c == '$')
		val = 3;
	else if (c == '\'')
		val = 4;
	else if (c == CHARSEOF0 || c == CHARSEOF255)
		val = 5;
	else
		val = 6;

	return val;
}
/************************************************************
*   Function name: funcAVID
*   Purpose: to recognize the pattern for AVID
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: isKeyword, strlen, strcpy
*   Parameters: char lexeme[]
*   Return Value: currentToken
*   Algorithm: checks if the current lexeme matches with KW from language;
*			   checks the limit defined for lexemes (VID_LEN); 
*			   sets the lexeme to the corresponding attribute (vidLexeme).
**************************************************************/
Token funcAVID(char lexeme[]) {
	Token currentToken = { 0 };
	
	if (isKeyword(lexeme) != RT_FAIL_1) {
		currentToken.code = KW_T;
		currentToken.attribute.keywordIndex = isKeyword(lexeme);
		return currentToken;
	}
	else { 
		currentToken.code = AVID_T; 
	}

	if (strlen(lexeme) > VID_LEN) {
		for (int i = 0; i < VID_LEN; i++) {
			currentToken.attribute.vidLexeme[i] = lexeme[i];
		}
		currentToken.attribute.vidLexeme[VID_LEN] = CHARSEOF0;
	}

	else {
		strcpy(currentToken.attribute.vidLexeme, lexeme);
		currentToken.attribute.vidLexeme[strlen(lexeme)] = CHARSEOF0;
	}

	return currentToken;
}

/************************************************************
*   Function name: funcSVID
*   Purpose: to recognize the pattern for SVID
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: strlen, strcpy
*   Parameters: char lexeme[]
*   Return Value: currentToken
*   Algorithm: checks the limit defined for lexemes (VID_LEN);
*			   sets the lexeme to the corresponding attribute (vidLexeme).
**************************************************************/
Token funcSVID(char lexeme[]) {
	Token currentToken = { 0 };
	currentToken.code = SVID_T;

	if (strlen(lexeme) > VID_LEN) {
		for (int i = 0; i < VID_LEN; i++) {
			currentToken.attribute.vidLexeme[i] = lexeme[i];
		}
		currentToken.attribute.vidLexeme[VID_LEN - 1] = '$';
		currentToken.attribute.vidLexeme[VID_LEN] = CHARSEOF0;
	} else {
		strcpy(currentToken.attribute.vidLexeme, lexeme);
		currentToken.attribute.vidLexeme[strlen(lexeme)] = CHARSEOF0;
	}

	return currentToken;
}

/************************************************************
*   Function name: funcIL
*   Purpose: to identify the IL
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: none
*   Parameters: char lexeme[]
*   Return Value: currentToken
*   Algorithm: checks the limit, returns error if lexemeslarger;
*			   sets the lexeme to the corresponding attribute (vidLexeme).
**************************************************************/
Token funcIL(char lexeme[]) {
	Token currentToken = { 0 };
	int sToInt = atoi(lexeme);
	
	if (sToInt > SHRT_MAX || sToInt < SHRT_MIN) {
		currentToken = funcErr(lexeme);
	}
	else {
		currentToken.code = INL_T;
		currentToken.attribute.intValue = sToInt;
	}

	return currentToken;
}

/************************************************************
*   Function name: funcFPL
*   Purpose: to identify FPL (float point literals)
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: strlen, funcErr
*   Parameters: char lexeme[]
*   Return Value: currentToken
*   Algorithm: checks the limit, if it is large returns the error;
*			   accepts only first ERR_LEN characters.
**************************************************************/
Token funcFPL(char lexeme[]) { //Might need reworking
	Token currentToken = { 0 };
	float sToFPL = atof(lexeme);
	if (((sToFPL >= 0 && strlen(lexeme) > 7) && (sToFPL < FLT_MIN || sToFPL > FLT_MAX)) || (sToFPL < 0)) { 
		currentToken = funcErr(lexeme);
	}
	else {
		currentToken.code = FPL_T;
		currentToken.attribute.floatValue = (float)sToFPL;

	}
	return currentToken;
}

/************************************************************
*   Function name: funcSL
*   Purpose: to identify SL (string literals)
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: strlen, bAddCh, bGetAddChOffset
*   Parameters: char lexeme[]
*   Return Value: currentToken
*   Algorithm: checks if lexeme is stored in the String Literal Table;
*			   includes  literals in this structure, using offsets;
*			   separates the lexeme and incrementes the line.
**************************************************************/
Token funcSL(char lexeme[]) {
	Token currentToken = { 0 };
	int i;

	currentToken.code = STR_T;
	currentToken.attribute.contentString = bGetAddChOffset(stringLiteralTable);

	for (i = 0; i < (signed)strlen(lexeme); i++) {
		if (!(lexeme[i] == '\''))
			bAddCh(stringLiteralTable, lexeme[i]);
		if (lexeme[i] == '\n')
			line++;
	}

	bAddCh(stringLiteralTable, CHARSEOF0);

	return currentToken;
}

/************************************************************
*   Function name: funcErr
*   Purpose: to deal with ERR token
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: strcat
*   Parameters: char lexeme[]
*   Return Value: currentToken
*   Algorithm: uses the errLexeme, checks the limit given by ERR_LEN;
*			   uses three dots (...) to find the limit is defined;
*			   and incrementes the line.
**************************************************************/
Token funcErr(char lexeme[]) {
	Token currentToken = { 0 };
	unsigned int i;

	if (strlen(lexeme) > ERR_LEN) {
		for (i = 0; i < (ERR_LEN - 3); i++) {
			if (lexeme[i] == '\n')
				line++;
			currentToken.attribute.errLexeme[i] = lexeme[i];
		}
		strcat(currentToken.attribute.errLexeme, "...");
	}
	else {
		for (i = 0; i < (signed)strlen(lexeme); i++) {
			if (lexeme[i] == '\n')
				line++;
			currentToken.attribute.errLexeme[i] = lexeme[i];
		}
	}

	currentToken.code = ERR_T;

	return currentToken;
}

/************************************************************
*   Function name: isKeyword
*   Purpose: to checks if one specific lexeme is a keyword
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: strlen, strcmp
*   Parameters: char lexeme[]
*   Return Value: 1, RT_FAIL_1
*   Algorithm: uses the keywordTable to check the keywords
**************************************************************/

int isKeyword(char lexeme[]) {
	int i;

	if (!strlen(lexeme)) {
		return RT_FAIL_1;
	}	
	for (i = 0; i < KWT_SIZE; i++) {
		if (strcmp(lexeme, keywordTable[i]) == 0)
			return i;
	}
	return RT_FAIL_1;
}