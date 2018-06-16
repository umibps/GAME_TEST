#include <stdarg.h>
#include "script.h"

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
 ...		: printf�Ɠ���
*/
void DefaultScriptErrorMessage(
	void* dummy,
	const char* file_name,
	int line,
	const char* message,
	...
)
{
	va_list list;

	if(line > 0)
	{
		(void)fprintf(stderr, "At line %d, in %s\n", line, file_name);
	}

	va_start(list, message);

	(void)vfprintf(stderr, message, list);

	va_end(list);
}

/*
 ScriptBasicGetDefaultReservedStrings�֐�
 �f�t�H���g�̗\���̕�����z����擾����
 ����
 num_strings	: �\���̐�
 �Ԃ�l
	�f�t�H���g�̗\���̕�����z��
*/
const char** ScriptBasicGetDefaultReservedStrings(int* num_strings)
{
	static const char *reserved[] ={"if", "else", "while", "break"};
	if(num_strings != NULL)
	{
		*num_strings = sizeof(reserved) / sizeof(char*);
	}

	return reserved;
}

#ifdef __cplusplus
}
#endif
