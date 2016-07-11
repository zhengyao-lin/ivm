#ifndef _IVM_STD_BIT_H_
#define _IVM_STD_BIT_H_

#define IVM_BIT_SET_FALSE(bit) ((bit) &= 0)
#define IVM_BIT_SET_TRUE(bit) ((bit) |= 1)

#define IVM_BIT_SET_POS_FALSE(num, pos) ((num) &= ~(1 << (pos)))
#define IVM_BIT_SET_POS_TRUE(num, pos) ((num) |= 1 << (pos))

#define IVM_BIT_GET_POS(num, pos) (((num) >> (pos)) & 1)

#endif
