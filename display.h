#ifndef _INCLUDED_DISPLAY_H_
#define _INCLUDED_DISPLAY_H_

#include "types.h"
#include "shader_program.h"

typedef struct _SIMPLE_SHADER_PROGRAM
{
	SHADER_PROGRAM_BASE base;
	GLint texture_uniform;
} SIMPLE_SHADER_PROGRAM;

typedef struct _DISPLAY_PROGRAMS
{
	SIMPLE_SHADER_PROGRAM draw_square;
} DISPLAY_PROGRAMS;

#ifdef __cplusplus
extern "C" {
#endif

EXTERN void Display(void);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_DISPLAY_H_
