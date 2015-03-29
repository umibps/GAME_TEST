#ifndef _INCLUDED_MAIN_LOOP_H_
#define _INCLUDED_MAIN_LOOP_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 TimerCallback関数
 一定時間後に呼び出される関数
 引数
 timer_id	: タイマー識別ID (使わない)
*/
EXTERN void TimerCallback(int timer_id);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_MAIN_LOOP_H_
