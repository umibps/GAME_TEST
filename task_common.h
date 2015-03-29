#ifndef _INCLUDED_TASK_COMMON_H_
#define _INCLUDED_TASK_COMMON_H_

#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 TaskTop関数
 タスクリストの先頭で実行されるタスク
 画面のクリアと入力初期化を実行
*/
EXTERN void TaskTop(TASK* task);

/*
 TaskBottom関数
 タスクリストの最後で実行されるタスク
 画面の更新を行う
*/
EXTERN void TaskBottom(TASK* task);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_TASK_COMMON_H_
