/*
	Application name:	FUzzy logic to tune the right wheel
	File name:		fuzzyhuo.c
	Written by:		Huo Feng
        Last modifid:           2017-3-29 
*/
#ifndef _FUZZYHUO_H
#define _FUZZYHUO_H

#define MINFZ(A,B)       (A < B) ? A : B
#define MAXFZ(A,B)       (A > B) ? A : B

struct Rule {
	int             antecedent[2];
	int             consequent[1];
};

extern float           fuzzy_inputs[2][8];
extern float           fuzzy_outputs[8];
extern float           rule_strengths[49];

extern void    fuzzy_step(float *crisp_inputs, float *crisp_outputs);
extern void    fuzzify_input(int in_index,float in_val);
extern float   get_membership_value(int in_index,int mf_index,float in_val);
extern void    eval_rule(int rule_index);
extern float   defuzzify_output(int out_index,float *inputs);

#endif
/***********************END Line************************/

