#ifndef _INCLUDED_MAIN_LOOP_H_
#define _INCLUDED_MAIN_LOOP_H_

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 TimerCallback�֐�
 ��莞�Ԍ�ɌĂяo�����֐�
 ����
 timer_id	: �^�C�}�[����ID (�g��Ȃ�)
*/
EXTERN void TimerCallback(int timer_id);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_MAIN_LOOP_H_
