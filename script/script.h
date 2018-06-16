#ifndef _INCLUDED_SCRIPT_H_
#define _INCLUDED_SCRIPT_H_

#include "rule.h"
#include "parser.h"
#include "lexical_analyser.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 DefaultScriptErrorMessage�֐�
 �f�t�H���g�̃X�N���v�g�������̃G���[�\���֐�
 ����
 dummy
 file_name	: �G���[�̂������t�@�C����
 line		: �G���[�̔��������s�� (0�̏ꍇ�̓X�N���v�g�̓��e�ȊO�̃G���[)
 message	: �G���[���b�Z�[�W
 ... : printf�Ɠ���
*/
EXTERN void DefaultScriptErrorMessage(
	void* dummy,
	const char* file_name,
	int line,
	const char* message,
	...
);

/*
 ScriptBasicGetDefaultReservedStrings�֐�
 �f�t�H���g�̗\���̕�����z����擾����
 ����
 num_strings	: �\���̐�
 �Ԃ�l
	�f�t�H���g�̗\���̕�����z��
*/
EXTERN const char** ScriptBasicGetDefaultReservedStrings(int* num_strings);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_SCRIPT_H_
