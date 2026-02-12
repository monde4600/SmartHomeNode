#ifndef BITMAT_H
#define BITMAT_H
#define GETBIT(variable,bit_number)   (variable>>bit_number) &1
#define WRITEBIT(variable,bit_number,value)  variable =  ((variable & ~(1<<bit_number))|(value)<<(bit_number))
#define TOGGLEBIT(variable,bit_number)    variable ^= (1<<(bit_number))
#endif