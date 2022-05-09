/*************************************************************
* COMPILERS COURSE - Algonquin College
* Code version: Fall, 2020
* This Code is EXCLUSIVE for professors (must not be shared)
* Author: Svillen Ranev - Paulo Sousa - Abdulah.
*************************************************************
* File name: buffer.c
* Compiler: MS Visual Studio 2019
* Author: Paulo Sousa
* Course: CST 8152 – Compilers, Lab Section: [011, 012, 013, 014]
* Assignment: A1.
* Date: Jan 01 2021
* Professor: Paulo Sousa / Haider Miraj
* Purpose: This file is the main code for Buffeer (A1)
* Function list: (...).
*************************************************************/

#include "buffer.h"

/************************************************************
* Function name: bCreate
* Purpose: Creates the buffer according to capacity, increment 
	factor and operational mode (f, a, m)
* Author: Svillen Ranev
* History/Versions: Ver 2.0
* Called functions: calloc(), malloc()
* Parameters: 
*   size = initial capacity
*   increment = increment factor
*   mode = operational mode
* Return value: bPointer (pointer to Buffer)
* Algorithm: Allocation of memory according to inicial (default) values.
**************************************************************/

bPointer bCreate(short size, char increment, char mode) {
	bPointer b;
	char bMode;
	if (size<0 || size>MAX_SIZE)
		return NULL;
	if (!size) {
		size = DEFAULT_SIZE;
		increment = DEFAULT_INCREMENT;
	}
	if (!increment)
		mode = 'f';
	switch (mode) {
	case 'a':
		bMode = ADDMODE;
		break;
	case 'f':
		bMode = FIXMODE;
		increment = 0;
		break;
	case 'm':
		if ((unsigned char)increment > MAX_INCREMENT)
			return NULL;
		bMode = MULMODE;
		break;
	default:
		return NULL;
	}
	b = (bPointer)calloc(1, sizeof(bStructure));
	if (!b)
		return NULL;
	b->content = (char*)malloc(size);
	if (!b->content) {
		free(b);
		return NULL;
	}
	b->mode = bMode;
	b->size = size;
	b->increment = increment;
	b->flags = DEFAULT_FLAGS;
	return b;
}


/************************************************************
* Function name: bAddCh
* Purpose: Adds a char to buffer
* Parameters: 
*   pBuffer = pointer to Buffer Entity
*   ch = char to be added
* Return value: bPointer (pointer to Buffer)
* 2DO: Follow spec.
**************************************************************/

bPointer bAddCh(bPointer const pBuffer, char ch) {
	char* tempbuf;
	short availSpace, newSize, newInc;
	if (!pBuffer)
		return NULL;
	pBuffer->flags = pBuffer->flags & RESET_R_FLAG;
	if (pBuffer->addCOffset * sizeof(char) == MAX_SIZE ||
		pBuffer->addCOffset * sizeof(char) == SHRT_MAX)
		return NULL;
	if (pBuffer->size == pBuffer->addCOffset * (int)sizeof(char)) {
		switch (pBuffer->mode) {
		case ADDMODE:
			newSize = pBuffer->size + (unsigned char)pBuffer->increment;
			if (newSize > 0)
				if (newSize == SHRT_MAX)
					newSize = MAX_SIZE;
			break;
		case FIXMODE:
			return NULL;
		case MULMODE:
			availSpace = MAX_SIZE - pBuffer->size;
			newInc = (short int)(availSpace * (pBuffer->increment / 100.0F));
			newSize = pBuffer->size + newInc;
			if (newSize <= pBuffer->size && pBuffer->size < MAX_SIZE)
				newSize = MAX_SIZE;
			break;
		default:
			return NULL;
		}
		tempbuf = (char*)realloc(pBuffer->content, newSize);
		if (!tempbuf)
			return NULL;
		if (tempbuf != pBuffer->content) {
			pBuffer->flags = pBuffer->flags | SET_R_FLAG;
			pBuffer->content = tempbuf;
		}
		pBuffer->size = newSize;
	}
	pBuffer->content[pBuffer->addCOffset++] = ch;
	return pBuffer;
}

/************************************************************
* Function name: bClean
* Purpose: Clears the buffer
* Parameters:
*   bPointer = pointer to Buffer Entity
* Return value: operation status
* 2DO: Follow spec.
**************************************************************/
int bClean(bPointer const pBuffer) {
	if (!pBuffer)
		return RT_FAIL_1;
	pBuffer->addCOffset = pBuffer->markOffset = pBuffer->getCOffset = 0;
	pBuffer->flags = pBuffer->flags & RESET_EOB;
	pBuffer->flags = pBuffer->flags & RESET_R_FLAG;
	return 0;
}

/************************************************************
* Function name: bFree
* Purpose: Releases the buffer address
* Parameters:
*   bPointer = pointer to Buffer Entity
* Return value: int (boolean)
* 2DO: Follow spec.
**************************************************************/
int bFree(bPointer const pBuffer) {
	if (pBuffer) {
		free(pBuffer->content);
		free(pBuffer);
		return 1;
	}
	return 0;
}

/************************************************************
* Function name: bIsFull
* Purpose: Checks if buffer is null
* Parameters:
*   bPointer = pointer to Buffer Entity
* Return value: boolean value
* 2DO: Follow spec.
**************************************************************/
int bIsFull(bPointer const pBuffer) {
	if (!pBuffer)
		return RT_FAIL_1;
	return (pBuffer->size == pBuffer->addCOffset * (int)sizeof(char));
}

/************************************************************
* Function name: bGetAddChOffset
* Purpose: Returns the position of char to be got
* Parameters:
*   bPointer = pointer to Buffer Entity
* Return value: addcPosition value
* 2DO: Follow spec.
**************************************************************/
short bGetAddChOffset(bPointer const pBuffer) {
	if (!pBuffer)
		return RT_FAIL_1;
	return pBuffer->addCOffset;
}

/************************************************************
* Function name: bGetSize
* Purpose: Returns the current buffer capacity
* Parameters:
*   bPointer = pointer to Buffer Entity
* Return value: capacity of buffer.
* 2DO: Follow spec.
**************************************************************/
short bGetSize(bPointer const pBuffer) {
	if (!pBuffer)
		return RT_FAIL_1;
	return pBuffer->size;
}

/************************************************************
* Function name: bGetMode
* Purpose: Returns the operational mode
* Parameters:
*   bPointer = pointer to Buffer Entity
* Return value: operational mode.
* 2DO: Follow spec.
**************************************************************/
int bGetMode(bPointer const pBuffer) {
	if (!pBuffer)
		return RT_FAIL_2;
	return pBuffer->mode;
}


/************************************************************
* Function name: bGetMarkOffset
* Purpose: Returns the position of mark in the buffer
* Parameters:
*   bPointer = pointer to Buffer Entity
* Return value: markC position.
* 2DO: Follow spec.
**************************************************************/

short bGetMarkOffset(bPointer const pBuffer) {
	if (!pBuffer)
		return RT_FAIL_1;
	return pBuffer->markOffset;
}


/************************************************************
* Function name: bSetMarkOffset
* Purpose: Adjust the position of mark in the buffer
* Parameters:
*   bPointer = pointer to Buffer Entity
*   mark = mark position for char
* Return value: buffer pointer.
* 2DO: Follow spec.
**************************************************************/
short bSetMarkOffset(bPointer const pBuffer, short mark) {
	if (!pBuffer || mark<0 || mark > pBuffer->addCOffset)
		return RT_FAIL_1;
	pBuffer->markOffset = mark;
	return 0;
}

/************************************************************
* Function name: bFinish
* Purpose: Ends buffer with an specific char
* Parameters:
*   bPointer = pointer to Buffer Entity
* Return value: bPointer = buffer pointer.
* 2DO: Follow spec.
**************************************************************/
bPointer bFinish(bPointer const pBuffer, char ch) {
	char* tempbuf;
	short size;
	if (!pBuffer || pBuffer->addCOffset * sizeof(char) == SHRT_MAX)
		return NULL;
	pBuffer->flags = pBuffer->flags & RESET_R_FLAG;
	if (pBuffer->size - pBuffer->addCOffset * sizeof(char) == 1) {
		pBuffer->content[pBuffer->addCOffset++] = ch;
		return pBuffer;
	}
	size = (pBuffer->addCOffset + 1) * sizeof(char);
	tempbuf = (char*)realloc(pBuffer->content, size);
	if (!tempbuf)
		return NULL;
	if (pBuffer->content != tempbuf) {
		pBuffer->flags = pBuffer->flags | SET_R_FLAG;
		pBuffer->content = tempbuf;
	}
	pBuffer->size = size;
	pBuffer->content[pBuffer->addCOffset++] = ch;
	return pBuffer;
}

/************************************************************
* Function name: bDisplay
* Purpose: Prints the string in the buffer.
* Parameters:
*   bPointer = pointer to Buffer Entity
*   nl = Char to put in the end of printing
* Return value: Number of chars printed.
* 2DO: Follow spec.
**************************************************************/
int bDisplay(bPointer const pBuffer, char nl) {
	int cont = 0;
	char c;
	if (!pBuffer || !(pBuffer->content))
		return RT_FAIL_1;
	c = bGetCh(pBuffer);
	while (!(pBuffer->flags & CHECK_EOB)) {
		cont++;
		printf("%c", c);
		c = bGetCh(pBuffer);
	}
	//for (c = bufferGetChar(pBE); !(pBE->flags & CHECK_EOB); cont++) {
	//	printf("%c", c);
	//	c = bufferGetChar(pBE);
	//}
	if (nl)
		printf("\n");
	return cont;
}

/************************************************************
* Function name: bLoad
* Purpose: Loads the string in the buffer with the content of 
	an specific file.
* Parameters:
*   bPointer = pointer to Buffer Entity
*   fi = pointer to file descriptor
* Return value: Number of chars read and put in buffer.
* 2DO: Follow spec.
**************************************************************/
int bLoad(bPointer const pBuffer, FILE* const fi) {
	int size = 0;
	char c;
	if (!fi || !pBuffer)
		return RT_FAIL_1;
	c = (char)fgetc(fi);
	while (!feof(fi)) {
		if (!bAddCh(pBuffer, c)) {
			ungetc(c, fi);
			return LOAD_FAIL;
		}
		c = (char)fgetc(fi);
		size++;
	}
	if (ferror(fi))
		return RT_FAIL_1;
	return size;
}

/************************************************************
* Function name: bIsEmpty
* Purpose: Checks if buffer is empty.
* Parameters:
*   bPointer = pointer to Buffer Entity
* Return value: Boolean value.
* 2DO: Follow spec.
**************************************************************/
int bIsEmpty(bPointer const pBuffer) {
	if (!pBuffer)
		return RT_FAIL_1;
	return (pBuffer->addCOffset == 0);
}

/************************************************************
* Function name: bGetCh
* Purpose: Returns the char in the getC position.
* Parameters:
*   bPointer = pointer to Buffer Entity
* Return value: Char in the getC position.
* 2DO: Follow spec.
**************************************************************/
char bGetCh(bPointer const pBuffer) {
	if (!pBuffer)
		return RT_FAIL_1;
	if (pBuffer->getCOffset == pBuffer->addCOffset) {
		pBuffer->flags = pBuffer->flags | SET_EOB;
		return '\0';
	}
	pBuffer->flags = pBuffer->flags & RESET_EOB;
	return pBuffer->content[pBuffer->getCOffset++];
}


/************************************************************
* Function name: bRewind
* Purpose: Rewinds the buffer.
* Parameters:
*   bPointer = pointer to Buffer Entity
* Return value: Success in rewind.
* 2DO: Follow spec.
**************************************************************/
int bRewind(bPointer const pBuffer) {
	if (!pBuffer)
		return RT_FAIL_1;
	pBuffer->getCOffset = 0;
	pBuffer->markOffset = 0;
	return 0;
}


/************************************************************
* Function name: bRetract
* Purpose: Retracts the buffer to put back the char in buffer.
* Parameters:
*   bPointer = pointer to Buffer Entity
* Return value: The getCPosition updated.
* 2DO: Follow spec.
**************************************************************/
bPointer bRetract(bPointer const pBuffer) {
	if (!pBuffer || pBuffer->getCOffset == 0)
		return NULL;
	pBuffer->getCOffset--;
	return pBuffer;
}
/*
short bRetract(bPointer const pBuffer) {
	if (!pBuffer || pBuffer->getCOffset == 0)
		return RT_FAIL_1;
	return (pBuffer->getCOffset--);
}
*/


/************************************************************
* Function name: bufferReset
* Purpose: Resets the buffer.
* Parameters:
*   pBE = pointer to Buffer Entity
* Return value: The getCPosition.
* 2DO: Follow spec.
**************************************************************/
short bRestore(bPointer const pBuffer) {
	if (!pBuffer)
		return RT_FAIL_1;
	pBuffer->getCOffset = pBuffer->markOffset;
	return pBuffer->getCOffset;
}


/************************************************************
* Function name: bufferGetCPosition
* Purpose: Returns the value of getCPosition.
* Parameters:
*   pBE = pointer to Buffer Entity
* Return value: The getCPosition.
* 2DO: Follow spec.
**************************************************************/
short bGetChOffset(bPointer const pBuffer) {
	if (!pBuffer)
		return RT_FAIL_1;
	//pBD->getc_offset = pBD->markc_offset;
	return pBuffer->getCOffset;
}


/************************************************************
* Function name: bufferGetIncrement
* Purpose: Returns the buffer increment.
* Parameters:
*   pBE = pointer to Buffer Entity
* Return value: The Buffer increment.
* 2DO: Follow spec.
**************************************************************/
size_t bGetIncrement(bPointer const pBuffer) {
	if (!pBuffer)
		return RT_INC_FAIL;
	return (unsigned char)pBuffer->increment;
}


/************************************************************
* Function name: bufferGetString
* Purpose: Returns the pointer to String.
* Parameters:
*   pBE = pointer to Buffer Entity
*   charPosition = position to get the pointer
* Return value: Position of string char.
* 2DO: Follow spec.
**************************************************************/
char* bGetContent(bPointer const pBuffer, short pos) {
	if (!pBuffer || pos < 0 || pos > pBuffer->addCOffset)
		return NULL;
	return pBuffer->content + pos;
}


/************************************************************
* Function name: bufferAddCPosition
* Purpose: Returns the position wher new chars are added.
* Parameters:
*   pBE = pointer to Buffer Entity
* Return value: AddCPosition.
* 2DO: Follow spec.
**************************************************************/
short bufferAddCPosition(bPointer const pBuffer) {
	if (!pBuffer)
		return RT_FAIL_1;
	return pBuffer->addCOffset;
}


/************************************************************
* Function name: bufferGetFlags
* Purpose: Returns the entire flags of Buffer.
* Parameters:
*   pBE = pointer to Buffer Entity
* Return value: Flags of Buffer.
* 2DO: Follow spec.
**************************************************************/
#define FLAGS_
#undef FLAGS_
#ifndef FLAGS_
unsigned short bGetFlags(bPointer const pBuffer) {
	if (!pBuffer)
		return (unsigned short)RT_FAIL_1;
	return pBuffer->flags;
}
#else
#define bGetFlags(pBuffer) ((pBuffer)?(pBuffer->flags):(RT_FAIL_1))
#endif
