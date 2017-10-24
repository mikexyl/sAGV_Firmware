/*
	Application name:	Fuzzy logic to tune both wheels
	File name:		fuzzyhuo.c
	Written by:		Huo Feng
    Last modifid:           2017-6-12 
*/

#include <math.h>
#include "fuzzyhuo.h"

#define   LIMIT(x,lmt)      ((x > lmt)? lmt : ((x > -lmt)? x : (-lmt)))  //ÏÞÖÆxÔÚ -lmt ~ lmt //huo

float  rule_strengths[49]={0};
float  fuzzy_inputs[2][8]={{0}};
float  fuzzy_outputs[8]={0};
float  in_univ_fac=1.0, in_univ_fact=1.0,out_univ_fac=1.0;

const float	inmem_points[7][4] =
{
		{ -6.000000, -6.000000, -6.000000, -4.000000 },
		{ -6.000000, -4.000000, -4.000000, -2.000000 },
		{ -4.000000, -2.000000, -2.000000, 0.000000 },
		{ -2.000000, 0.000000, 0.000000, 2.000000 },
		{ 0.000000, 2.000000, 2.000000, 4.000000 },
		{ 2.000000, 4.000000, 4.000000, 6.000000 },
		{ 4.000000, 6.000000, 6.000000, 6.000000 }
	
};

const float	outmem_points[7][4] =
{
		{ -6.000000 },
		{ -4.000000 },
		{ -2.000000 },
		{ 0.000000 },
		{ 2.000000 },
		{ 4.000000 },
		{ 6.000000 }
};

float	crisp_outputs[1] = {0};

const struct	Rule	Rules[49] =
{
	{ { 0x00, 0x01 }, { 0xb0 } }, 
	{ { 0x00, 0x09 }, { 0xb0 } },
	{ { 0x00, 0x11 }, { 0xb0 } },
	{ { 0x00, 0x19 }, { 0xb0 } },
	{ { 0x00, 0x21 }, { 0xa8 } },
	{ { 0x00, 0x29 }, { 0xa8 } },
	{ { 0x00, 0x31 }, { 0x98 } },
	{ { 0x08, 0x01 }, { 0xb0 } },
	{ { 0x08, 0x09 }, { 0xa8 } },
	{ { 0x08, 0x11 }, { 0xa8 } },
	{ { 0x08, 0x19 }, { 0xa8 } },
	{ { 0x08, 0x21 }, { 0x98 } },
	{ { 0x08, 0x29 }, { 0x90 } }, 
	{ { 0x08, 0x31 }, { 0x90 } },
	{ { 0x10, 0x01 }, { 0xb0 } }, 
	{ { 0x10, 0x09 }, { 0xa8 } },
	{ { 0x10, 0x11 }, { 0xa8 } },
	{ { 0x10, 0x19 }, { 0xa0 } }, 
	{ { 0x10, 0x21 }, { 0x98 } },
	{ { 0x10, 0x29 }, { 0x88 } }, 
	{ { 0x10, 0x31 }, { 0x80 } }, 
	{ { 0x18, 0x01 }, { 0xb0 } }, 
	{ { 0x18, 0x09 }, { 0xa8 } }, 
	{ { 0x18, 0x11 }, { 0xa0 } },
	{ { 0x18, 0x19 }, { 0x98 } },
	{ { 0x18, 0x21 }, { 0x90 } },
	{ { 0x18, 0x29 }, { 0x90 } }, 
	{ { 0x18, 0x31 }, { 0x80 } }, 
	{ { 0x20, 0x01 }, { 0xb0 } }, 
	{ { 0x20, 0x09 }, { 0xa8 } }, 
	{ { 0x20, 0x11 }, { 0x98 } },
	{ { 0x20, 0x19 }, { 0x90 } }, 
	{ { 0x20, 0x21 }, { 0x88 } },
	{ { 0x20, 0x29 }, { 0x88 } },
	{ { 0x20, 0x31 }, { 0x80 } }, 
	{ { 0x28, 0x01 }, { 0xa0 } }, 
	{ { 0x28, 0x09 }, { 0xa0 } },
	{ { 0x28, 0x11 }, { 0x98 } },
	{ { 0x28, 0x19 }, { 0x88 } },
	{ { 0x28, 0x21 }, { 0x88 } },
	{ { 0x28, 0x29 }, { 0x88 } },
	{ { 0x28, 0x31 }, { 0x80 } }, 
	{ { 0x30, 0x01 }, { 0x98 } }, 
	{ { 0x30, 0x09 }, { 0x88 } }, 
	{ { 0x30, 0x11 }, { 0x88 } },
	{ { 0x30, 0x19 }, { 0x80 } },
	{ { 0x30, 0x21 }, { 0x80 } },
	{ { 0x30, 0x29 }, { 0x80 } },
	{ { 0x30, 0x31 }, { 0x80 } }
};

void fuzzy_step(float *crisp_inputs, float *crisp_outputs)
{
  int     in_index,rule_index;

	  if(fabs(crisp_inputs[0])<=5 & fabs(crisp_inputs[1])<=0.25) //original 5, 0.5 
    {
      in_univ_fac = 0.5;  //orihinal 0.5--0.5
      in_univ_fact = 0.5;  //original 1.0
#if 0
      if(fabs(crisp_inputs[0])<=2.5)
        out_univ_fac = 0.1;  //original 0.3--0.5
      else
#endif
        out_univ_fac = 0.5;  //original 0.3--0.5
    }
    else if(fabs(crisp_inputs[0])>10) //2017-5-29
		{in_univ_fac = fabs(crisp_inputs[0])/10.0f;
		 in_univ_fact = in_univ_fac;
		 out_univ_fac = LIMIT(in_univ_fac,3);
		} 
    else if(fabs(crisp_inputs[1])>1.0) //2017-5-29
		{in_univ_fact = fabs(crisp_inputs[1])/1.0f;
		 in_univ_fac = in_univ_fact;
		 out_univ_fac = LIMIT(in_univ_fact,3);
		}
	else 
	    {in_univ_fac = 1.0;
		 in_univ_fact = 1.0;
		 out_univ_fac = 1.0;
		}
     
	crisp_inputs[0] = LIMIT(crisp_inputs[0],10.0*in_univ_fac) * 6.0/(10.0*in_univ_fac);

    crisp_inputs[1] = LIMIT(crisp_inputs[1],1.0*in_univ_fact) * 6.0/(1.0*in_univ_fact); //original 0.5

	for (in_index = 0;in_index < 2;in_index++)
	{
		fuzzify_input(in_index,crisp_inputs[in_index]);
	}
	for (rule_index = 0;rule_index < 49;rule_index++)
	{
		eval_rule(rule_index);
	}

		crisp_outputs[0] = defuzzify_output(0, crisp_inputs)/6.0*10.0*out_univ_fac;  //mm/s 
	
}

void fuzzify_input(int in_index,float in_val)
{
	int i;
	for (i = 0;i < 7;i++)
	{
		fuzzy_inputs[in_index][i] = get_membership_value(in_index,i,in_val);
		
	}
}
float get_membership_value(int in_index,int mf_index,float in_val)
{
	if (in_val < inmem_points[mf_index][0]) return 0;
	if (in_val > inmem_points[mf_index][3]) return 0;
	if (in_val <= inmem_points[mf_index][1])
	{
		if (inmem_points[mf_index][0] == inmem_points[mf_index][1])
			return 1;
		else
			return ((in_val - inmem_points[mf_index][0]) /
					(inmem_points[mf_index][1] - inmem_points[mf_index][0]));
	}
	if (in_val >= inmem_points[mf_index][2])
	{
		if (inmem_points[mf_index][2] == inmem_points[mf_index][3])
			return 1;
		else
			return ((inmem_points[mf_index][3] - in_val) /
					(inmem_points[mf_index][3] - inmem_points[mf_index][2]));
	}
	return 1;
}
void eval_rule(int rule_index)
{
	int             in_index,mf_index,ant_index,con_index;
	int     val;
	float   rule_strength = 1;
	for     (ant_index = 0;ant_index < 2;ant_index++)
	{
		val = Rules[rule_index].antecedent[ant_index];
		in_index = (val & 0x07);
		mf_index = ((val & 0x38) >> 3);
		rule_strength = MINFZ(rule_strength,fuzzy_inputs[in_index][mf_index]);
	}
	rule_strengths[rule_index] = rule_strength;
	
	for (con_index = 0;con_index < 1;con_index++)
	{
		val = Rules[rule_index].consequent[con_index];
		mf_index = ((val & 0x38) >> 3);
		fuzzy_outputs[mf_index] = MAXFZ(fuzzy_outputs[mf_index],
			rule_strengths[rule_index]);
	}
}
float defuzzify_output(int out_index,float *inputs)
{
	float           summ = 0;
	float           product = 0;
	float           temp1,temp2;
	int             mf_index;
	
	for (mf_index = 0;mf_index < 7;mf_index++)
	{
		temp1 = fuzzy_outputs[mf_index];
		temp2 = outmem_points[mf_index][0];
		summ = summ + temp1;
		product = product + (temp1 * temp2);
		
		fuzzy_outputs[mf_index] = 0;
	}
	if (summ > 0)
	{
		crisp_outputs[out_index] = product / summ;
		return crisp_outputs[out_index];
	}
	else
	{
		return crisp_outputs[out_index];
	}
}
/*****************End of the file****************************/
  
