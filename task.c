#include <string.h>
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeTasks関数
 タスク全体を管理するデータを初期化する
 引数
 tasks		: タスク全体を管理するデータ
 game_data	: ゲーム全体を管理するデータ
*/
void InitializeTasks(TASKS* tasks, GAME_DATA* game_data)
{
	(void)memset(tasks, 0, sizeof(tasks));

	tasks->game_data = game_data;
}

/*
 TaskNew関数
 新たにタスクを作成する
 引数
 tasks			: タスク全体を管理するデータ
 execute		: タスクの内容処理の関数ポインタ
 delete_func	: タスク削除時に実行するコールバック関数
 priority		: タスクの実行優先度
 data_size		: タスクが保有するデータのバイト数
 返り値
	作成したタスクデータ
*/
TASK* TaskNew(
	TASKS* tasks,
	void (*execute)(TASK*),
	void (*delete_func)(TASK*),
	unsigned int priority,
	size_t data_size
)
{
	TASK *ret;		// 返り値
	TASK *previous;	// タスクリストの前
	int id;			// for文用のカウンタ

	// 空いているタスクを探す
	for(id=tasks->search_start; id<MAX_TASK; id++)
	{
		if(tasks->task[id].execute == NULL)
		{
			break;
		}
	}
	// 空きが無いか、実行関数がなければ終了
	if(id == MAX_TASK || execute == NULL)
	{
		return NULL;
	}

	// 次回の検索を高速化するために検索開始位置を更新
	tasks->search_start = id;

	// 返り値となるタスクデータに値を設定
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
	
	// リストに優先度に合わせてタスクを入れる
	if(tasks->top == NULL)
	{	// 最初のタスクなら先頭に設定
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

		// 挿入位置が先頭なら先頭タスクを更新
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
 DeleteTask関数
 タスクを削除する
 引数
 task	: 削除するタスク
*/
void DeleteTask(TASK* task)
{
	int id;

	// 二重開放チェック
	if(task == NULL || task->execute == NULL)
	{
		return;
	}

	// 削除時のコールバック関数があれば実行
	if(task->delete_func != NULL)
	{
		task->delete_func(task);
	}
	MEM_FREE_FUNC(task->data);

	// 実行関数をリセットすることで空き状態にする
	task->execute = NULL;

	// 実行リストを更新
	if(task->previous != NULL)
	{
		task->previous->next = task->next;
	}
	if(task->next != NULL)
	{
		task->next->previous = task->previous;
	}

	// 先頭のタスクを削除する場合は先頭タスクを更新
	if(task == task->tasks->top)
	{
		task->tasks->top = task->next;
	}

	// タスク作成時の検索開始位置を更新
	id = (int)(task - task->tasks->task);
	if(id < task->tasks->search_start)
	{
		task->tasks->search_start = id;
	}
}

/*
 TaskChangePriroity関数
 タスクの実行優先度を変更する
 引数
 task			: 
 new_priority	: 新しい優先度
*/
void TaskChangePriority(TASK* task, unsigned int new_priority)
{
	TASK *previous;	// タスクリストの前

	// 一度タスクリストから対象のリストを外す
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
	{	// 最初のタスクなら先頭に設定
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

		// 挿入位置が先頭なら先頭タスクを更新
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
 TaskChangeExecution関数
 タスクの実行処理を変更する
 引数
 task			: 実行処理を変更するタスク
 new_execution	: 新たな実行処理関数のポインタ
*/
void TaskChangeExecution(TASK* task, void (*new_execution)(TASK*))
{
	task->execute = new_execution;
}

/*
 ExecuteTasks関数
 タスクリストを1周分実行する
 引数
 tasks	: タスク全体を管理するデータ
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
