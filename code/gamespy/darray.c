/* 
 *
 * File: darray.c
 * ---------------
 *	David Wright
 *	10/8/98
 * 
 *  See darray.h for function descriptions
 */
#if !defined(UNDER_CE) && !defined(__KATANA__)
#include <assert.h>
#else
#define assert(a)
#endif
#include <stdlib.h>
#include <string.h>
#include "darray.h" 

#ifdef _MFC_MEM_DEBUG
#define _CRTDBG_MAP_ALLOC 1
#include <crtdbg.h>
#endif


#define DEF_GROWBY 8

#ifdef _NO_NOPORT_H_
    #define gsimalloc malloc
    #define gsifree free
    #define gsirealloc realloc
#else
    #include "nonport.h" //for gsimalloc/realloc/free
#endif


// STRUCTURES
struct DArrayImplementation
{
    int count, capacity;
    int elemsize;
    int growby;
    ArrayElementFreeFn elemfreefn;
    void *list; //array of elements
};

// PROTOTYPES
static void *mylsearch(const void *key, void *base, int count, int size,
                     ArrayCompareFn comparator);
static void *mybsearch(const void *elem, void *base, int num, int elemsize, 
                       ArrayCompareFn comparator, int *found);
// FUNCTIONS

/* FreeElement
 * Frees the element at position N in the array
 */
static void FreeElement(DArray array, int n)
{
    if (array->elemfreefn != NULL)
        array->elemfreefn(ArrayNth(array,n));
}

/* ArrayGrow
 * Reallocates the array to a new size, incresed by growby
 */
static void ArrayGrow(DArray array)
{
    array->capacity +=  array->growby;
    array->list = gsirealloc(array->list, array->capacity * array->elemsize);
    assert(array->list);
}

/* SetElement
 * Sets the element at pos to the contents of elem
 */
static void SetElement(DArray array, const void *elem, int pos)
{
    memcpy(ArrayNth(array,pos), elem, array->elemsize);
}

DArray ArrayNew(int elemSize, int numElemsToAllocate, 
                ArrayElementFreeFn elemFreeFn)
{
    DArray array;

    array = (DArray) gsimalloc(sizeof(struct DArrayImplementation));
    assert(array);
    assert(elemSize);
    if (numElemsToAllocate == 0)
        numElemsToAllocate = DEF_GROWBY;
    array->count = 0;
    array->capacity = numElemsToAllocate;;
    array->elemsize = elemSize;
    array->growby = numElemsToAllocate;
    array->elemfreefn = elemFreeFn;
    array->list = gsimalloc(array->capacity * array->elemsize);
    assert(array->list);

    return array;
}

void ArrayFree(DArray array)
{
    int i;
    
    assert(array);
    for (i = 0; i < array->count; i++)
    {
        FreeElement(array, i);
    }
    gsifree(array->list);
    gsifree(array);
}


int ArrayLength(const DArray array)
{
    return array->count;
}

void *ArrayNth(DArray array, int n)
{
    assert( (n >= 0) && (n < array->count));
    
    return (char *)array->list + array->elemsize*n;
}

/* ArrayAppend
 * Just do an Insert at the end of the array
 */
void ArrayAppend(DArray array, const void *newElem)
{
    ArrayInsertAt(array, newElem, array->count);
}

void ArrayInsertAt(DArray array, const void *newElem, int n)
{
    assert( (n >= 0) && (n <= array->count));

    if (array->count == array->capacity)
        ArrayGrow(array);
    array->count++;
    if (n < array->count - 1) //if we aren't appending
        memmove(ArrayNth(array, n+1), ArrayNth(array,n), 
                (array->count - 1 - n) * array->elemsize);
    SetElement(array, newElem, n);
}

void ArrayInsertSorted(DArray array, const void *newElem, ArrayCompareFn comparator)
{
    int n;
    void *res;
    int found;
    assert( comparator);
    res=mybsearch(newElem, array->list,	array->count, array->elemsize, comparator, &found);
    n = (((char *)res - (char *)array->list) / array->elemsize);
    ArrayInsertAt(array, newElem, n);
}


void ArrayRemoveAt(DArray array, int n)
{
       assert( (n >= 0) && (n < array->count));

    if (n < array->count - 1) //if not last element
        memmove(ArrayNth(array,n),ArrayNth(array,n+1),
                (array->count - 1 - n) * array->elemsize);
    array->count--;
}

void ArrayDeleteAt(DArray array, int n)
{
       assert( (n >= 0) && (n < array->count));

    FreeElement(array,n);
    ArrayRemoveAt(array, n);
}


void ArrayReplaceAt(DArray array, const void *newElem, int n)
{
    assert( (n >= 0) && (n < array->count));

    FreeElement(array, n);
    SetElement(array, newElem,n);
}


void ArraySort(DArray array, ArrayCompareFn comparator)
{
    qsort(array->list, array->count, array->elemsize, comparator);
}

//assert will be raised by ArrayNth if fromindex out of range
int ArraySearch(DArray array, const void *key, ArrayCompareFn comparator, 
                  int fromIndex, int isSorted)
{
    void *res;
    int found = 1;
    if (!array || array->count == 0)
        return NOT_FOUND;

       if (isSorted)
        res=mybsearch(key, ArrayNth(array,fromIndex),
                    array->count - fromIndex, array->elemsize, comparator, &found);
    else
        res=mylsearch(key, ArrayNth(array, fromIndex), 
                      array->count - fromIndex, array->elemsize, comparator);
    if (res != NULL && found)
        return (((char *)res - (char *)array->list) / array->elemsize);
    else
        return NOT_FOUND;
}


void ArrayMap(DArray array, ArrayMapFn fn, void *clientData)
{
    int i;

    assert(fn);

    for (i = 0; i < array->count; i++)
        fn(ArrayNth(array,i), clientData);
        
}

void ArrayMapBackwards(DArray array, ArrayMapFn fn, void *clientData)
{
    int i;

    assert(fn);

    for (i = (array->count - 1) ; i >= 0 ; i--)
        fn(ArrayNth(array,i), clientData);
        
}

void * ArrayMap2(DArray array, ArrayMapFn2 fn, void *clientData)
{
    int i;
    void * pcurr;

    assert(fn);

    for (i = 0; i < array->count; i++)
    {
        pcurr = ArrayNth(array,i);
        if(!fn(pcurr, clientData))
            return pcurr;
    }

    return NULL;
}

void * ArrayMapBackwards2(DArray array, ArrayMapFn2 fn, void *clientData)
{
    int i;
    void * pcurr;

    assert(fn);

    for (i = (array->count - 1) ; i >= 0 ; i--)
    {
        pcurr = ArrayNth(array,i);
        if(!fn(pcurr, clientData))
            return pcurr;
    }

    return NULL;
}

void ArrayClear(DArray array)
{
    int i;

    // This could be more optimal!
    //////////////////////////////
    for(i = (ArrayLength(array) - 1) ; i >= 0 ; i--)
        ArrayDeleteAt(array, i);
}

/* mylsearch
 * Implementation of a standard linear search on an array, since we
 * couldn't use lfind
 */
static void *mylsearch(const void *key, void *base, int count, int size,
                     ArrayCompareFn comparator)
{
    int i;
    for (i = 0; i < count; i++)
    {
        if (comparator(key, (char *)base + size*i) == 0)
            return (char *)base + size*i;
    }
    return NULL;
}

/* mybsearch
 * Implementation of a bsearch, since its not available on all platforms
 */
static void *mybsearch(const void *elem, void *base, int num, int elemsize, ArrayCompareFn comparator, int *found)
{
    int L, H, I, C;
    
    L = 0;
    H = num - 1;
    *found = 0;
    while (L <= H)
    {
        I = (L + H) >> 1;
        C = comparator(((char *)base) + I * elemsize,elem);
        if (C == 0)
            *found = 1;
        if (C < 0) 
            L = I + 1;
        else
        {
            H = I - 1;
        }
    }
    return ((char *)base) + L * elemsize;
}