#ifndef _INCLUDED_TASK_H_
#define _INCLUDED_TASK_H_

#include <stdio.h>
#include "types.h"

#define MAX_TASK 128	// �쐬�ł���^�X�N���̍ő�l

typedef struct _TASK
{
	void (*execute)(struct _TASK*);		// ���s�֐�
	void (*delete_func)(struct _TASK*);	// �^�X�N�폜���̃R�[���o�b�N�֐�
	void *data;							// �^�X�N���ۗL����f�[�^
	unsigned int priority;				// �^�X�N�̗D��x
	struct _TASK *previous;				// �^�X�N���X�g�̑O
	struct _TASK *next;					// �^�X�N���X�g�̌�
	struct _TASKS *tasks;				// �^�X�N�S�̂��Ǘ�����f�[�^
} TASK;

typedef struct _TASKS
{
	TASK task[MAX_TASK];	// �^�X�N�̎���
	TASK *top;				// �擪�Ɏ��s����^�X�N
	int search_start;		// �����J�n�ʒu
	GAME_DATA *game_data;	// �Q�[���S�̂��Ǘ�����f�[�^
} TASKS;

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeTasks�֐�
 �^�X�N�S�̂��Ǘ�����f�[�^������������
 ����
 tasks		: �^�X�N�S�̂��Ǘ�����f�[�^
 game_data	: �Q�[���S�̂��Ǘ�����f�[�^
*/
EXTERN void InitializeTasks(TASKS* tasks, GAME_DATA* game_data);

/*
 TaskNew�֐�
 �V���Ƀ^�X�N���쐬����
 ����
 tasks			: �^�X�N�S�̂��Ǘ�����f�[�^
 execute		: �^�X�N�̓��e�����̊֐��|�C���^
 delete_func	: �^�X�N�폜���Ɏ��s����R�[���o�b�N�֐�
 priority		: �^�X�N�̎��s�D��x
 data_size		: �^�X�N���ۗL����f�[�^�̃o�C�g��
 �Ԃ�l
	�쐬�����^�X�N�f�[�^
*/
EXTERN TASK* TaskNew(
	TASKS* tasks,
	void (*execute)(TASK*),
	void (*delete_func)(TASK*),
	unsigned int priority,
	size_t data_size
);

/*
 DeleteTask�֐�
 �^�X�N���폜����
 ����
 task	: �폜����^�X�N
*/
EXTERN void DeleteTask(TASK* task);

/*
 TaskChangePriroity�֐�
 �^�X�N�̎��s�D��x��ύX����
 ����
 task			: 
 new_priority	: �V�����D��x
*/
EXTERN void TaskChangePriority(TASK* task, unsigned int new_priority);

/*
 TaskChangeExecution�֐�
 �^�X�N�̎��s������ύX����
 ����
 task			: ���s������ύX����^�X�N
 new_execution	: �V���Ȏ��s�����֐��̃|�C���^
*/
EXTERN void TaskChangeExecution(TASK* task, void (*new_execution)(TASK*));

/*
 ExecuteTasks�֐�
 �^�X�N���X�g��1�������s����
 ����
 tasks	: �^�X�N�S�̂��Ǘ�����f�[�^
*/
EXTERN void ExecuteTasks(TASKS* tasks);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_TASK_H_
