/*
# This file is Copyright 2010 Dean Hall.
#
# This file is part of the Python-on-a-Chip program.
# Python-on-a-Chip is free software: you can redistribute it and/or modify
# it under the terms of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1.
#
# Python-on-a-Chip is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# A copy of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1
# is seen in the file COPYING up one directory from this.
*/

#ifndef _PLAT_H_
#define _PLAT_H_

// Note: for unit testing, no Python heap is needed, so make it tiny.
// The unit tests need lots of heap and stack, so a small Python heap
// allows that.
#ifdef UNIT_TEST
#define PM_HEAP_SIZE 100
#elif defined(__DEBUG)
#define PM_HEAP_SIZE 6800
#else
#define PM_HEAP_SIZE 6880
#endif

#define PM_FLOAT_LITTLE_ENDIAN

#define PM_PLAT_HEAP_ATTR __attribute__((far)) __attribute__((aligned ((4))))

#endif /* _PLAT_H_ */