/*
 * accessor_comp.h
 *
 *  Created on: 13 juil. 2015
 *      Author: gorinje
 */

#ifndef ACCESSOR_COMP_H_
#define ACCESSOR_COMP_H_

typedef struct Acc_Comp_s Acc_Comp;

int compile(struct Acc_Comp_s** comp, const char* llvm_code, unsigned int length, const char* cachedir);
void* getFn(struct Acc_Comp_s* comp, const char * name);

#endif /* ACCESSOR_COMP_H_ */
