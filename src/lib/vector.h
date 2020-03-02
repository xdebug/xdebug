/*
The MIT License (MIT)

Copyright (c) 2015 Evan Teran

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef vector_H_
#define vector_H_

#include <stddef.h> /* for size_t */
#include <stdlib.h> /* for malloc/realloc/free */

/**
 * @brief XDEBUG_VECTOR_OF_TYPE - The vector type used in this library
 */
#define XDEBUG_VECTOR_OF_TYPE(type) type *

/**
 * @brief XDEBUG_VECTOR_SET_CAPACITY - For internal use, sets the capacity variable of the vector
 * @param vec - the vector
 * @param size - the new capacity to set
 * @return void
 */
#define XDEBUG_VECTOR_SET_CAPACITY(vec, size)     \
	if (vec) {                          \
		((size_t *)(vec))[-1] = (size); \
	}

/**
 * @brief XDEBUG_VECTOR_SET_SIZE - For internal use, sets the size variable of the vector
 * @param vec - the vector
 * @param size - the new capacity to set
 * @return void
 */
#define XDEBUG_VECTOR_SET_SIZE(vec, size)         \
	if (vec) {                          \
		((size_t *)(vec))[-2] = (size); \
	}

/**
 * @brief XDEBUG_VECTOR_CAPACITY - gets the current capacity of the vector
 * @param vec - the vector
 * @return the capacity as a size_t
 */
#define XDEBUG_VECTOR_CAPACITY(vec) \
	((vec) ? ((size_t *)(vec))[-1] : (size_t)0)

/**
 * @brief XDEBUG_VECTOR_SIZE - gets the current size of the vector
 * @param vec - the vector
 * @return the size as a size_t
 */
#define XDEBUG_VECTOR_SIZE(vec) \
	((vec) ? ((size_t *)(vec))[-2] : (size_t)0)

/**
 * @brief XDEBUG_VECTOR_NOT_EMPTY - returns non-zero if the vector is not empty
 * @param vec - the vector
 * @return zero if empty, non-zero if non-empty
 */
#define XDEBUG_VECTOR_NOT_EMPTY(vec) \
	(XDEBUG_VECTOR_SIZE(vec) != 0)

/**
 * @brief XDEBUG_VECTOR_GROW - For internal use, ensures that the vector is at least <count> elements big
 * @param vec - the vector
 * @param size - the new capacity to set
 * @return void
 */
#define XDEBUG_VECTOR_GROW(vec, count)                                             \
	const size_t __sz = (count) * sizeof(*(vec)) + (sizeof(size_t) * 2); \
	if (!(vec)) {                                                        \
		size_t *__p = malloc(__sz);                                      \
		(vec) = (void *)(&__p[2]);                                       \
		XDEBUG_VECTOR_SET_CAPACITY((vec), (count));                            \
		XDEBUG_VECTOR_SET_SIZE((vec), 0);                                      \
	} else {                                                             \
		size_t *__p1 = &((size_t *)(vec))[-2];                           \
		size_t *__p2 = realloc(__p1, (__sz));                            \
		(vec) = (void *)(&__p2[2]);                                      \
		XDEBUG_VECTOR_SET_CAPACITY((vec), (count));                            \
	}

/**
 * @brief XDEBUG_VECTOR_POP - removes the last element from the vector
 * @param vec - the vector
 * @return void
 */
#define XDEBUG_VECTOR_POP(vec)                           \
	XDEBUG_VECTOR_SET_SIZE((vec), XDEBUG_VECTOR_SIZE(vec) - 1);

/**
 * @brief XDEBUG_VECTOR_REMOVE - removes the element at index i from the vector
 * @param vec - the vector
 * @param i - index of element to remove
 * @return void
 */
#define XDEBUG_VECTOR_REMOVE(vec, i)                              \
	if (vec) {                                         \
		const size_t __sz = XDEBUG_VECTOR_SIZE(vec);         \
		if ((i) < __sz) {                              \
			size_t __x;                                \
			XDEBUG_VECTOR_SET_SIZE((vec), __sz - 1);         \
			for (__x = (i); __x < (__sz - 1); ++__x) { \
				(vec)[__x] = (vec)[__x + 1];           \
			}                                          \
		}                                              \
	}

/**
 * @brief XDEBUG_VECTOR_FREE - frees all memory associated with the vector
 * @param vec - the vector
 * @return void
 */
#define XDEBUG_VECTOR_FREE(vec)                        \
	if (vec) {                               \
		size_t *p1 = &((size_t *)(vec))[-2]; \
		free(p1);                            \
	}

/**
 * @brief XDEBUG_VECTOR_START - returns an iterator to first element of the vector
 * @param vec - the vector
 * @return a pointer to the first element (or NULL)
 */
#define XDEBUG_VECTOR_START(vec) \
	(vec)

/**
 * @brief XDEBUG_VECTOR_END - returns an iterator to the last element of the vector
 * @param vec - the vector
 * @return a pointer to the last element (or NULL)
 */
#define XDEBUG_VECTOR_END(vec) \
	((vec) ? &((vec)[XDEBUG_VECTOR_SIZE(vec) - 1]) : NULL)

/**
 * @brief XDEBUG_VECTOR_PREV - returns a pointer to the previous element of a given element if it exists
 * @param vec - the vector
 * @param elem - pointer to the element
 * @return a pointer to the previous element (or NULL)
 */
#define XDEBUG_VECTOR_PREV(vec, elem) \
	((vec && elem != vec) ? elem - 1 : NULL)

/**
 * @brief XDEBUG_VECTOR_NEXT - returns a pointer to the next element of a given element if it exists
 * @param vec - the vector
 * @param elem - pointer to the element
 * @return a pointer to the previous element (or NULL)
 */
#define XDEBUG_VECTOR_NEXT(vec, elem) \
	((vec && elem != XDEBUG_VECTOR_END(vec)) ? elem + 1 : NULL)

/**
 * @brief XDEBUG_VECTOR_ELEMENT_AT - returns a pointer to the element at a certain position
 * @param vec - the vector
 * @param position - the position
 * @return a pointer to the element at position (or NULL)
 */
#define XDEBUG_VECTOR_ELEMENT_AT(vec, position) \
	((vec) ? &((vec)[position]) : NULL)

/**
 * @brief XDEBUG_VECTOR_PUSH - adds an element to the end of the vector
 * @param vec - the vector
 * @param value - the value to add
 * @return void
 */
#define XDEBUG_VECTOR_PUSH(vec, value)                   \
	do {                                                \
		size_t __cap = XDEBUG_VECTOR_CAPACITY(vec);           \
		if (__cap <= XDEBUG_VECTOR_SIZE(vec)) {               \
			XDEBUG_VECTOR_GROW((vec), __cap + 1);             \
		}                                               \
		vec[XDEBUG_VECTOR_SIZE(vec)] = (value);               \
		XDEBUG_VECTOR_SET_SIZE((vec), XDEBUG_VECTOR_SIZE(vec) + 1); \
	} while (0)


/**
 * @brief XDEBUG_VECTOR_PUSH_EMPTY - adds an empty element to the end of the vector
 * @param vec - the vector
 * @return void
 */
#define XDEBUG_VECTOR_PUSH_EMPTY(vec)                       \
	do {                                                \
		size_t __cap = XDEBUG_VECTOR_CAPACITY(vec);           \
		if (__cap <= XDEBUG_VECTOR_SIZE(vec)) {               \
			XDEBUG_VECTOR_GROW((vec), __cap + 1);             \
		}                                               \
		XDEBUG_VECTOR_SET_SIZE((vec), XDEBUG_VECTOR_SIZE(vec) + 1); \
	} while (0)

#endif /* vector_H_ */
