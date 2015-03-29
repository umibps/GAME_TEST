#include <string.h>
#include "task.h"

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
void InitializeTasks(TASKS* tasks, GAME_DATA* game_data)
{
	(void)memset(tasks, 0, sizeof(tasks));

	tasks->game_data = game_data;
}

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
TASK* TaskNew(
	TASKS* tasks,
	void (*execute)(TASK*),
	void (*delete_func)(TASK*),
	unsigned int priority,
	size_t data_size
)
{
	TASK *ret;		// �Ԃ�l
	TASK *previous;	// �^�X�N���X�g�̑O
	int id;			// for���p�̃J�E���^

	// �󂢂Ă���^�X�N��T��
	for(id=tasks->search_start; id<MAX_TASK; id++)
	{
		if(tasks->task[id].execute == NULL)
		{
			break;
		}
	}
	// �󂫂��������A���s�֐����Ȃ���ΏI��
	if(id == MAX_TASK || execute == NULL)
	{
		return NULL;
	}

	// ����̌��������������邽�߂Ɍ����J�n�ʒu���X�V
	tasks->search_start = id;

	// �Ԃ�l�ƂȂ�^�X�N�f�[�^�ɒl��ݒ�
	ret = &tasks->task[id];
	ret->execute = execute;
	ret->delete_func = delete_func;
	if(data_size > 0)
	{
		ret->data = MEM_ALLOC_FUNC(data_size);
	}
	else
	{
		ret->data = NULL;
	}
	ret->priority = priority;
	ret->tasks = tasks;
	
	// ���X�g�ɗD��x�ɍ��킹�ă^�X�N������
	if(tasks->top == NULL)
	{	// �ŏ��̃^�X�N�Ȃ�擪�ɐݒ�
		tasks->top = ret;
		ret->previous = ret->next = NULL;
	}
	else
	{
		previous = tasks->top;
		while(previous->next != NULL
			&& previous->next->priority < priority)
		{
			previous = previous->next;
		}

		// �}���ʒu���擪�Ȃ�擪�^�X�N���X�V
		if(previous == tasks->top
			&& previous->priority > priority)
		{
			tasks->top = ret;
			ret->previous = NULL;
			ret->next = previous;
			previous->previous = ret;
		}
		else
		{
			ret->previous = previous;
			ret->next = previous->next;
			previous->next = ret;
			if(ret->next != NULL)
			{
				ret->next->previous = ret;
			}
		}
	}

	return ret;
}

/*
 DeleteTask�֐�
 �^�X�N���폜����
 ����
 task	: �폜����^�X�N
*/
void DeleteTask(TASK* task)
{
	int id;

	// ��d�J���`�F�b�N
	if(task == NULL || task->execute == NULL)
	{
		return;
	}

	// �폜���̃R�[���o�b�N�֐�������Ύ��s
	if(task->delete_func != NULL)
	{
		task->delete_func(task);
	}
	MEM_FREE_FUNC(task->data);

	// ���s�֐������Z�b�g���邱�Ƃŋ󂫏�Ԃɂ���
	task->execute = NULL;

	// ���s���X�g���X�V
	if(task->previous != NULL)
	{
		task->previous->next = task->next;
	}
	if(task->next != NULL)
	{
		task->next->previous = task->previous;
	}

	// �擪�̃^�X�N���폜����ꍇ�͐擪�^�X�N���X�V
	if(task == task->tasks->top)
	{
		task->tasks->top = task->next;
	}

	// �^�X�N�쐬���̌����J�n�ʒu���X�V
	id = (int)(task - task->tasks->task);
	if(id < task->tasks->search_start)
	{
		task->tasks->search_start = id;
	}
}

/*
 TaskChangePriroity�֐�
 �^�X�N�̎��s�D��x��ύX����
 ����
 task			: 
 new_priority	: �V�����D��x
*/
void TaskChangePriority(TASK* task, unsigned int new_priority)
{
	TASK *previous;	// �^�X�N���X�g�̑O

	// ��x�^�X�N���X�g����Ώۂ̃��X�g���O��
	if(task->previous != NULL)
	{
		task->previous->next = task->next;
	}
	else
	{
		task->tasks->top = task->next;
	}
	if(task->next != NULL)
	{
		task->next->previous = task->previous;
	}

	if(task->tasks->top == NULL)
	{	// �ŏ��̃^�X�N�Ȃ�擪�ɐݒ�
		task->tasks->top = task;
		task->previous = task->next = NULL;
	}
	else
	{
		previous = task->tasks->top;
		while(previous->next != NULL
			&& previous->next->priority > new_priority)
		{
			previous = previous->next;
		}

		// �}���ʒu���擪�Ȃ�擪�^�X�N���X�V
		if(previous == task->tasks->top
			&& previous->priority > new_priority)
		{
			task->tasks->top = task;
			task->previous = NULL;
			task->next = previous;
			previous->previous = task;
		}
		else
		{
			task->previous = previous;
			task->next = previous->next;
			previous->next = task;
			if(task->next != NULL)
			{
				task->next->previous = task;
			}
		}
	}
}

/*
 TaskChangeExecution�֐�
 �^�X�N�̎��s������ύX����
 ����
 task			: ���s������ύX����^�X�N
 new_execution	: �V���Ȏ��s�����֐��̃|�C���^
*/
void TaskChangeExecution(TASK* task, void (*new_execution)(TASK*))
{
	task->execute = new_execution;
}

/*
 ExecuteTasks�֐�
 �^�X�N���X�g��1�������s����
 ����
 tasks	: �^�X�N�S�̂��Ǘ�����f�[�^
*/
void ExecuteTasks(TASKS* tasks)
{
	TASK *execute = tasks->top;

	while(execute != NULL)
	{
		execute->execute(execute);
		execute = execute->next;
	}
}

#ifdef __cplusplus
}
#endif
