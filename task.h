#ifndef _INCLUDED_TASK_H_
#define _INCLUDED_TASK_H_

#include <stdio.h>
#include "types.h"

#define MAX_TASK 128	// 作成できるタスク数の最大値

typedef struct _TASK
{
	void (*execute)(struct _TASK*);		// 実行関数
	void (*delete_func)(struct _TASK*);	// タスク削除時のコールバック関数
	void *data;							// タスクが保有するデータ
	unsigned int priority;				// タスクの優先度
	struct _TASK *previous;				// タスクリストの前
	struct _TASK *next;					// タスクリストの後
	struct _TASKS *tasks;				// タスク全体を管理するデータ
} TASK;

typedef struct _TASKS
{
	TASK task[MAX_TASK];	// タスクの実体
	TASK *top;				// 先頭に実行するタスク
	int search_start;		// 検索開始位置
	GAME_DATA *game_data;	// ゲーム全体を管理するデータ
} TASKS;

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
EXTERN void InitializeTasks(TASKS* tasks, GAME_DATA* game_data);

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
EXTERN TASK* TaskNew(
	TASKS* tasks,
	void (*execute)(TASK*),
	void (*delete_func)(TASK*),
	unsigned int priority,
	size_t data_size
);

/*
 DeleteTask関数
 タスクを削除する
 引数
 task	: 削除するタスク
*/
EXTERN void DeleteTask(TASK* task);

/*
 TaskChangePriroity関数
 タスクの実行優先度を変更する
 引数
 task			: 
 new_priority	: 新しい優先度
*/
EXTERN void TaskChangePriority(TASK* task, unsigned int new_priority);

/*
 TaskChangeExecution関数
 タスクの実行処理を変更する
 引数
 task			: 実行処理を変更するタスク
 new_execution	: 新たな実行処理関数のポインタ
*/
EXTERN void TaskChangeExecution(TASK* task, void (*new_execution)(TASK*));

/*
 ExecuteTasks関数
 タスクリストを1周分実行する
 引数
 tasks	: タスク全体を管理するデータ
*/
EXTERN void ExecuteTasks(TASKS* tasks);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_TASK_H_
