#ifndef _INCLUDED_TASK_COMMON_H_
#define _INCLUDED_TASK_COMMON_H_

#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 TaskTop�֐�
 �^�X�N���X�g�̐擪�Ŏ��s�����^�X�N
 ��ʂ̃N���A�Ɠ��͏����������s
*/
EXTERN void TaskTop(TASK* task);

/*
 TaskBottom�֐�
 �^�X�N���X�g�̍Ō�Ŏ��s�����^�X�N
 ��ʂ̍X�V���s��
*/
EXTERN void TaskBottom(TASK* task);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_TASK_COMMON_H_
