#include <string.h>
#include "token.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeToken�֐�
 �X�N���v�g�̎����͂ɂ���ē�����g�[�N���f�[�^������������
 ����
 token	: �g�[�N���f�[�^
 line	: �g�[�N���̑��݂���s��
*/
void InitializeToken(TOKEN* token, int line)
{
	(void)memset(token, 0, sizeof(*token));
	token->line = (uint16)line;
}

#ifdef __cplusplus
}
#endif
