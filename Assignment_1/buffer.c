/*************************************************************
* File name: buffer.c
Compiler: MS Visual Studio 2019
Author: Matthew Leblanc 040961708
        Yuliia Topalova 040981104  
Course: CST 8152 – Compilers, Lab Section: 012
Assignment: 1
Date: February 5, 2021
Professor: Abdulah Kadri, Paulo Sousa.
Purpose: This file is the main code for Buffer (A1)
         it includes constant definitions and functions declaration.
Function list: bCreate, bAddCh, bClean, bFree, bIsFull, bGetAddChOffset,
               bGetSize, bGetMode, bGetMarkOffset, bSetMarkOffset, bFinish
               bDisplay, bLoad, bIsEmpty, bGetCh, bRewind, bRetract,
               bRestore, bGetChOffset, bGetIncrement, bGetContent,
               bufferAddCPosition, bGetFlags
*************************************************************/

#include "buffer.h"

/************************************************************
*   Function name: bCreate
*   Purpose: to create a new buffer in memory
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: calloc(), sizeof(), malloc(), free()
*   Parameters: short size, char increment, char mod
*   Return Value: bPointer, NULL
*   Algorithm: - allocate memory for one buffer structure;
*              - allocate memory for one dynamic character, based on the initial capacity;
*              - copies the given size value into the Buffer size;
*              - return the pointer to Buffer if succeeded, else free dynamically allocated 
*                memory and return NULL. 
**************************************************************/
bPointer bCreate(short size, char increment, char mode) {
    bPointer b = NULL;
    char tempMode;

    /* checks range of parameter size (between 0 and MAX_SIZE) */
    if (size < 0 || size > MAX_SIZE)
        return NULL;
    /* if size = 0, set it DEFAULT_SIZE, and increment to DEFAULT_INCREMENT */
    else if (size == 0) {
        size = DEFAULT_SIZE;
        increment = DEFAULT_INCREMENT;
    }

    /* increment value according to the mode */
    switch (mode) {
    case 'f':
        tempMode = FIXMODE;
        increment = 0;
        break;
    case 'a':
        if (increment == 0) {
            tempMode = FIXMODE;
            /* only 0 for FIXMODE */
            increment = 0;
            break;
        }
        else {
            tempMode = ADDMODE;
            /* range from 1 to 255 for ADDMODE */
            if ((unsigned char)increment < 1 || (unsigned char)increment > 255)
                return NULL; 
            break;
        }
    case 'm':
        tempMode = MULMODE;
        /* range from 1 to MAXINCREMENT for MULMODE */
        if ((unsigned char)increment < 1 || (unsigned char)increment > MAXINCREMENT)
            return NULL; 
        break;
    default:
        return NULL;
    }

    /* create new buffer using calloc */
    b = (bPointer)calloc(1, sizeof(bStructure));
    if (!b)
        return NULL;

    /* content call malloc (with given size) */
    b->content = (char*)malloc(size);

    /* checks content */
    if (!b->content) {
        free(b);
        return NULL;
    }

    b->mode = tempMode;
    b->size = size;
    b->increment = increment;
    b->flags = DEFAULT_FLAGS;

    return b;
}

/************************************************************
*   Function name: bAddCh
*   Purpose: to include a char in the buffer
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: realloc()
*   Parameters: bPointer const pBuffer, char ch
*   Return Value: pBuffer, NULL
*   Algorithm: - set the r_flag bit to 0;
*		       - if the buffer is not full, add the character and increment addCOffset by 1;
*		       - if the buffer is full, try to resize;
*		       - return pBuffer if succeeded, or return NULL and free dynamically allocated memory
**************************************************************/
bPointer bAddCh(bPointer const pBuffer, char ch) {
    int newSize = 0;
    int availableSpace = 0;
    int newInc = 0;
    char* newContent;

    if (!pBuffer) 
        return NULL;
    pBuffer->flags = pBuffer->flags & RESET_R_FLAG;

    /* if Buffer not full store the symbol, is not try resize */
    if (pBuffer->addCOffset == pBuffer->size) {
        /* impossible to increase Buffer in FIXMODE */
        if (pBuffer->mode == FIXMODE) {
            return NULL;
        }
        /* ADDMODE try to increase to current size adding increament */
        if (pBuffer->mode == ADDMODE) {
            newSize = bGetSize(pBuffer) + bGetIncrement(pBuffer);

            if (newSize < 0 || newSize > MAX_VALUE) {
                return NULL;
            }
            else if (newSize == MAX_VALUE) {
                newSize = MAX_SIZE;
            }
            else {
                pBuffer->size = bGetSize(pBuffer) + (short)bGetIncrement(pBuffer);
            }   
        }
        /* MULMODE can not increament it is already full */
        if (pBuffer->mode == MULMODE) {
            if (bGetSize(pBuffer) == MAX_SIZE) {
                return NULL;
            }
            /* available space = maximum buffer size – current size */
            availableSpace = MAX_SIZE - pBuffer->size;
            /* new increment = available space * inc_factor / 100 */
            (int)newInc = (short int)(availableSpace * ((pBuffer->increment) / 100.0));
            /* new size = current size + new increment */
            newSize = bGetSize(pBuffer) + newInc;
            
            if (newSize < MAX_SIZE && newInc == 0) {
                newSize = MAX_SIZE;
            }
            else if (newSize < 0 || newSize > MAX_VALUE) {
                return NULL;
            }
        }
        /* using realloc() with the new size to expand the buffer */
        newContent = (char*)realloc(pBuffer->content, newSize);
        if (!newContent) {
            return NULL; 
        }
        /* set r_flag if memory has been changed */
        if (newContent != pBuffer->content) {
            pBuffer->flags = pBuffer->flags | SET_R_FLAG;
        }
        pBuffer->content = newContent;
        pBuffer->size = newSize;
    }
    /* change the value of addCOffset by 1 */
    pBuffer->content[pBuffer->addCOffset] = ch;
    pBuffer->addCOffset++;
    return pBuffer;
}

/************************************************************
*   Function name: bClean
*   Purpose: to retains the memory space currently allocated to the buffer
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: none
*   Parameters: bPointer const pBuffer
*   Return Value: RT_FAIL_1 on a failure
*   Algorithm: - check the pointers of the Buffer;
*              - reset the value of offsets. 
**************************************************************/
int bClean(bPointer const pBuffer) {
        if (pBuffer == NULL) {
            return RT_FAIL_1;
        }
        /*Reset the value of offsets*/
        pBuffer->getCOffset = 0;
        pBuffer->markOffset = 0;
        pBuffer->addCOffset = 0;

        return 0;
}

/************************************************************
*   Function name: bFree
*   Purpose: to de-allocates the memory occupied by the buffer
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: none
*   Parameters: bPointer const pBuffer
*   Return Value: 1 if the character buffer is full, 0 if not and RT_FAIL_1 on a failure
*   Algorithm: - check the pointers of the Buffer is NULL;
*              - free the dynamically allocate memory if it is not
**************************************************************/
int bFree(bPointer const pBuffer) {
    if (!pBuffer) {
        return RT_FAIL_1;
    }
    if (pBuffer->size == MAX_SIZE) {
        return 1;
    }
    /* checks the content and srt it to NULL */
    if (pBuffer->content != NULL) {
        free(pBuffer->content);
        pBuffer->content = NULL;
    }
    free(pBuffer);
    return 0;
}

/************************************************************
*   Function name: bIsFull
*   Purpose: to check the size of the buffer
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: none
*   Parameters: bPointer const pBuffer
*   Return Value: 1 if the character buffer is full, 0 if not and RT_FAIL_1 on a failure
*   Algorithm: - check the value of the Buffer size;
**************************************************************/
int bIsFull(bPointer const pBuffer) {
    if (!pBuffer) {
        return RT_FAIL_1;
    }
    /* return 1 if the buffer is full */
    if (pBuffer->addCOffset == pBuffer->size){
        return 1;
        }
       return 0;
    }
	
/************************************************************
*   Function name: bGetAddChOffset
*   Purpose: to return current addCOffset
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: none
*   Parameters: bPointer const pBuffer
*   Return Value: addCOffset
*   Algorithm: - check the pBuffer;
*              - check the addCOffset
**************************************************************/
short bGetAddChOffset(bPointer const pBuffer) {
    if (pBuffer == NULL) {
        return RT_FAIL_1;
    }
    return pBuffer->addCOffset;
}

/************************************************************
*   Function name: bGetSize
*   Purpose: to return the current size of the character buffer
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: none
*   Parameters: bPointer const pBuffer
*   Return Value: size
*   Algorithm: - check the pBuffer;
*              - check the size 
**************************************************************/
short bGetSize(bPointer const pBuffer) {
    if (pBuffer == NULL) {
        return RT_FAIL_1;
    }
    return pBuffer->size;
}

/************************************************************
*   Function name: bGetMode
*   Purpose: to return the value of mode to the calling function
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: none
*   Parameters: bPointer const pBuffer
*   Return Value: mode
*   Algorithm: - check the pBuffer;
*              - check the value of mode
**************************************************************/
int bGetMode(bPointer const pBuffer) {
    if (pBuffer == NULL) {
        return RT_FAIL_1;
    }
    return (int)pBuffer->mode; 
}

/************************************************************
*   Function name: bGetMarkOffset
*   Purpose: to return the value of markOffset to the calling function
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: none
*   Parameters: bPointer const pBuffer
*   Return Value: markOffset
*   Algorithm: - check the pBuffer;
*              - check the value ofmarkOffset
**************************************************************/
short bGetMarkOffset(bPointer const pBuffer) {
    if (pBuffer == NULL) {
        return RT_FAIL_1;
    }
    return pBuffer->markOffset;
}

/************************************************************
*   Function name: bSetMarkOffset
*   Purpose: to set the mark to markOffset
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: none
*   Parameters: bPointer const pBuffer, short mark
*   Return Value: markOffset
*   Algorithm: - check the pBuffer;
*              - check if the mark within the current limit of the buffer
**************************************************************/
short bSetMarkOffset(bPointer const pBuffer, short mark) {
    if (pBuffer == NULL) {
        return RT_FAIL_1;
    }
    mark = pBuffer->markOffset;
    /* checks the current limit of the buffer*/
    if (mark < 0 || mark > pBuffer->addCOffset) {
        return RT_FAIL_1;
    }
    return 0;
}

/************************************************************
*   Function name: bFinish
*   Purpose: to decrease the buffer to a new size
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: realloc()
*   Parameters: bPointer const pBuffer, char ch
*   Return Value: pBuffer
*   Algorithm: - check the pBuffer;
*              - adjust the new size with realloc();
*              - adds the ch to the end of the character buffer;
*              - increments addCOffset
**************************************************************/
bPointer bFinish(bPointer const pBuffer, char ch) {
    char* temp;
    if (pBuffer == NULL) {
        return NULL;
    }
    /* new size is current size addCOffset + 1 */
    pBuffer->size = (pBuffer->addCOffset + 1);
    /* adjusting new size with realloc */
    temp = (char*)realloc(pBuffer->content, pBuffer->size);
    if (temp == NULL) {
        return NULL;
    }
    if (pBuffer->content != temp) {
        pBuffer->flags |= SET_R_FLAG;
    }
    pBuffer->content = temp;
    /* increment addCOffset and add ch to the end */
    pBuffer->content[pBuffer->addCOffset++] = ch;
    return pBuffer;
}

/************************************************************
*   Function name: bDisplay
*   Purpose: to print the content of buffer
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: bGetCh()
*   Parameters: bPointer const pBuffer, char nl
*   Return Value: the number of characters printed
*   Algorithm: - check the pBuffer;
*              - using loop print the content of the buffer calling bGetCh();
*              - check the flags to detect the end of the buffer; 
*              - in the end of the loop checks the nl  is not 0 and print new line;
*              - then returns the number of characters printed. 
**************************************************************/
int bDisplay(bPointer const pBuffer, char nl) {
    char cH;
    int i = 0;
    if (pBuffer == NULL) {
        return RT_FAIL_1;
    }
    while (1) {
        cH = bGetCh(pBuffer);
        /* check flags to setect the end of the buffer */
        if ((pBuffer->flags & CHECK_EOB) != 0) {
            break;
        }
        printf("%c", cH);
        i++;
    }
    if (nl != 0) {
        printf("\n");
    }
    return i;
}
/************************************************************
*   Function name: bLoad
*   Purpose: to load an open input file specified by fi into a buffer
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: bAddCh()
*   Parameters: bPointer const pBuffer, FILE* const fi
*   Return Value: number of characters added to the buffer
*   Algorithm: - check the pBuffer;
*              - use fgetc to read one character at a time;
*              - bAddCh() function add the character to the buffer;
*              - return LOAD_FAIL if character cannot be added to the buffer
                 and using ungetc() function returns the character to the file stream
**************************************************************/
int bLoad(bPointer const pBuffer, FILE* const fi) {
    if (pBuffer == NULL) {
        return RT_FAIL_1;
    }
    char c;
    int charsAdded = 0;
    if (!pBuffer || !fi) {
        return RT_FAIL_1;
    }
    while (1) {
        /*read one char at a time*/
        c = (char)fgetc(fi);
        if (feof(fi)) {
            break;
        }
        if (!bAddCh(pBuffer, c)) {
            /* returns the character to the file stream */
            ungetc(c, fi);
            return LOAD_FAIL;
        }
        charsAdded++;
    }
    return charsAdded;
}

/************************************************************
*   Function name: bIsEmpty
*   Purpose: to return the empty state of the Buffer
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: none
*   Parameters: bPointer const pBuffer
*   Return Value: 1 if COffset is empty and  0 if COffset isn't empty
*   Algorithm: - check the pBuffer;
*              - check the value of the getCOffset;
*              - returen 1, otherwise 0
**************************************************************/
int bIsEmpty(bPointer const pBuffer) {
    if (pBuffer == NULL) {
        return RT_FAIL_1;
    }
    if (pBuffer->getCOffset == 0) {
        return 1;
    }
    return 0;
}

/************************************************************
*   Function name: bGetCh
*   Purpose: to read the buffer
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: none
*   Parameters: bPointer const pBuffer
*   Return Value: character located at getCOffset
*   Algorithm: - check the pBuffer;
*              - if getCOffset = addCOffset set the eob_flag bit to 1;
*              - increments getCOffset by 1 
**************************************************************/
char bGetCh(bPointer const pBuffer) {
    if (pBuffer == NULL) {
        return RT_FAIL_1;
    }
    if (pBuffer->getCOffset == pBuffer->addCOffset) {
        pBuffer->flags |= SET_EOB;
        return 0;
    }
    else
        pBuffer->flags &= RESET_EOB;

    return pBuffer->content[pBuffer->getCOffset++];
}


/************************************************************
*   Function name: bRewind
*   Purpose: to re-read the buffer
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: none
*   Parameters: bPointer const pBuffer
*   Return Value: returns RT_FAIL_1 on failure
*   Algorithm: - check the pBuffer;
*              - set the getCOffset and markOffset to 0
**************************************************************/
int bRewind(bPointer const pBuffer) {
    if (pBuffer == NULL) {
        return RT_FAIL_1;
    }
    pBuffer->getCOffset = 0;
    pBuffer->markOffset = 0;
    return 0;
}

/************************************************************
*   Function name: bRetract
*   Purpose: to return the buffer once it decremented
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: none
*   Parameters: bPointer const pBuffer
*   Return Value: pBuffer
*   Algorithm: - check the pBuffer;
*              - decrements getCOffset by 1
**************************************************************/
bPointer bRetract(bPointer const pBuffer) {
    if (pBuffer == NULL) {
        return NULL;
    }
    /* decrements by 1 */
    pBuffer->getCOffset--; 

    return pBuffer;
}

/************************************************************
*   Function name: bRestore
*   Purpose: to return the buffer once it decremented
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: none
*   Parameters: bPointer const pBuffer
*   Return Value: getCOffset
*   Algorithm: - check the pBuffer;
*              - sets getCOffset to the value of the markOffset
**************************************************************/
short bRestore(bPointer const pBuffer) {
    if (pBuffer == NULL) {
        return RT_FAIL_1;
    }
    pBuffer->getCOffset = pBuffer->markOffset;

    return 0;
}

/************************************************************
*   Function name: bGetChOffset
*   Purpose: to return getCOffset to the calling function
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: none
*   Parameters: bPointer const pBuffer
*   Return Value: getCOffset
*   Algorithm: - check the pBuffer;
*              - return getCOffset
**************************************************************/
short bGetChOffset(bPointer const pBuffer) {
    if (pBuffer == NULL || pBuffer->addCOffset || pBuffer->addCOffset < 0) {
        return RT_FAIL_1;
    }
    return pBuffer->getCOffset;
}

/************************************************************
*   Function name: bGetIncrement
*   Purpose: to return the non-negative value of the increment to the function
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: none
*   Parameters: bPointer const pBuffer
*   Return Value: size_t
*   Algorithm: - check the pBuffer;
*              - increment with non-negative value
**************************************************************/
size_t bGetIncrement(bPointer const pBuffer) {
    if (pBuffer == NULL)
        return (size_t)RT_INC_FAIL;
    /* return non-negative value of increment*/
    return (unsigned char)pBuffer->increment;
}

/************************************************************
*   Function name: bGetContent
*   Purpose: to return the pointer to the location of the buffer rom the beginning of the character array
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: none
*   Parameters: bPointer const pBuffer, short chPosition
*   Return Value: chPosition
*   Algorithm: - check the pBuffer;
*              - return a pointer to the location of the character buffer 
*                from the begging of the content   
**************************************************************/
char* bGetContent(bPointer const pBuffer, short chPosition) {
    if (pBuffer == NULL) {
        return NULL;
    }
    return pBuffer->content + chPosition;
}

/************************************************************
*   Function name: bufferAddCPosition
*   Purpose: to return the getCOffset to the calling function
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: none
*   Parameters: bStructure* const pBuffer
*   Return Value: getCOffset
*   Algorithm: - check the pBuffer;
*              - return getCOffset 
**************************************************************/
short bufferAddCPosition(bStructure* const pBuffer) {
    if (pBuffer == NULL) {
        return RT_FAIL_1;
    }
    return pBuffer->getCOffset;
}

/************************************************************
*   Function name: bufferAddCPosition
*   Purpose: to return the flag field from buffer
*   Author:  Matthew Leblanc, Yuliia Topalova
*   History/Versions: 1.0
*   Called functions: none
*   Parameters: bPointer const pBuffer
*   Return Value: flags
*   Algorithm: - check the pBuffer;
*              - return flags
**************************************************************/
#define FLAGS_
#undef FLAGS_
#ifndef FLAGS_
unsigned short bGetFlags(bPointer const pBuffer) {
    if (pBuffer == NULL) {
        return RT_FAIL_1;
    }
    return pBuffer->flags;
}
#else
#define
#endif
