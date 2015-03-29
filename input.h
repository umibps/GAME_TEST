#ifndef _INCLUDED_INPUT_H_
#define _INCLUDED_INPUT_H_

#include "types.h"

#define MAX_BUTTONS 29	// 使用するボタンの最大数

/*
 eBUTTON_FLAGS列挙体
 キーボードとジョイスティックのボタンのフラグ
*/
typedef enum _eBUTTON_FLAGS
{
	BUTTON_FLAG_UP = (1 << 0),
	BUTTON_FLAG_DOWN = (1 << 1),
	BUTTON_FLAG_LEFT = (1 << 2),
	BUTTON_FLAG_RIGHT = (1 << 3),
	BUTTON_FLAG_MOUSE_LEFT = (1 << 4),
	BUTTON_FLAG_MOUSE_RIGHT = (1 << 5),
	BUTTON_FLAG_0 = (1 << 6),
	BUTTON_FLAG_1 = (1 << 7),
	BUTTON_FLAG_2 = (1 << 8),
	BUTTON_FLAG_3 = (1 << 9),
	BUTTON_FLAG_4 = (1 << 10),
	BUTTON_FLAG_5 = (1 << 11),
	BUTTON_FLAG_6 = (1 << 12),
	BUTTON_FLAG_7 = (1 << 13),
	BUTTON_FLAG_8 = (1 << 14),
	BUTTON_FLAG_9 = (1 << 15),
	BUTTON_FLAG_10 = (1 << 16),
	BUTTON_FLAG_11 = (1 << 17),
	BUTTON_FLAG_12 = (1 << 18),
	BUTTON_FLAG_13 = (1 << 19),
	BUTTON_FLAG_14 = (1 << 20),
	BUTTON_FLAG_15 = (1 << 21),
	BUTTON_FLAG_16 = (1 << 22),
	BUTTON_FLAG_17 = (1 << 23),
	BUTTON_FLAG_18 = (1 << 24),
	BUTTON_FLAG_19 = (1 << 25),
	BUTTON_FLAG_20 = (1 << 26),
	BUTTON_FLAG_21 = (1 << 27),
	BUTTON_FLAG_22 = (1 << 28),
	BUTTON_FLAG_23 = (1 << 29),
	BUTTON_FLAG_24 = (1 << 30)
} eBUTTON_FLAGS;

/*
 eBUTTON_ID列挙体
 ボタンの識別ID
*/
typedef enum _eBUTTON_ID
{
	BUTTON_ID_UP,
	BUTTON_ID_DOWN,
	BUTTON_ID_LEFT,
	BUTTON_ID_RIGHT,
	BUTTON_ID_LEFT_BUTTON,
	BUTTON_ID_RIGHT_BUTTON,
	BUTTON_ID_0,
	BUTTON_ID_1,
	BUTTON_ID_2,
	BUTTON_ID_3,
	BUTTON_ID_4,
	BUTTON_ID_5,
	BUTTON_ID_6,
	BUTTON_ID_7,
	BUTTON_ID_8,
	BUTTON_ID_9,
	BUTTON_ID_10,
	BUTTON_ID_11,
	BUTTON_ID_12,
	BUTTON_ID_13,
	BUTTON_ID_14,
	BUTTON_ID_15,
	BUTTON_ID_16,
	BUTTON_ID_17,
	BUTTON_ID_18,
	BUTTON_ID_19,
	BUTTON_ID_20,
	BUTTON_ID_21,
	BUTTON_ID_22,
	BUTTON_ID_23,
	BUTTON_ID_24,
	BUTTON_ID_25,
	NUM_BUTTON
} eBUTTON_ID;

/*
 KEYBOARD_INPUT構造体
 キーボードからの入力を処理
*/
typedef struct _KEYBOARD_INPUT
{
	unsigned int keyboard[256/(sizeof(unsigned int)*8)];
	unsigned int special[256/(sizeof(unsigned int)*8)];
	unsigned int keyboard_map[32];
	unsigned int special_map[32];
} KEYBOARD_INPUT;

/*
 JOYSTICK_INPUT構造体
 ジョイスティックからの入力を処理
*/
typedef struct _JOYSTICK_INPUT
{
	unsigned int joystick[64/(sizeof(unsigned int)*8)];
	unsigned int joystick_map[32];
} JOYSTICK_INPUT;

/*
 MOUSE_INPUT構造体
 マウスからの入力を処理
*/
typedef struct _MOUSE_INPUT
{
	unsigned int mouse[128/(sizeof(unsigned int)*8)];
	unsigned int mouse_map[32];
	int x;
	int y;
} MOUSE_INPUT;

/*
 GAME_INPUT構造体
 入力全体のデータを管理
*/
typedef struct _GAME_INPUT
{
	KEYBOARD_INPUT keyboard;	// キーボードの状態
	JOYSTICK_INPUT joystick;	// ジョイスティックの状態
	MOUSE_INPUT mouse;			// マウスの状態
	uint32 current_input;		// 現在の入力状態
	uint32 input_down;			// ボタンが押されたフレームのみON
	uint32 intpu_up;			// ボタンが離されたフレームのみON
} GAME_INPUT;

#ifdef __cplusplus
extern "C" {
#endif

/*
 KeyboardDownCallback関数
 キーボードのキーが押された時のコールバック関数
 引数
 key	: 押されたキー
 x		: キーが押された時のマウスのX座標
 y		: キーが押された時のマウスのY座標
*/
EXTERN void KeyboardDownCallback(unsigned char key, int x, int y);

/*
 KeyboardUpCallback関数
 キーボードのキーが離された時のコールバック関数
 引数
 key	: 離されたキー
 x		: キーが離された時のマウスのX座標
 y		: キーが離された時のマウスのY座標
*/
EXTERN void KeyboardUpCallback(unsigned char key, int x, int y);

/*
 KeyboardSpecialDownCallback関数
 キーボードの特殊キーが押された時のコールバック関数
 引数
 key	: 押された特殊キー
 x		: 特殊キーが押された時のマウスのX座標
 y		: 特殊キーが押された時のマウスのY座標
*/
EXTERN void KeyboardSpecialDownCallback(unsigned int key, int x, int y);

/*
 KeyboardSpecialUpCallback関数
 キーボードの特殊キーが離された時のコールバック関数
 引数
 key	: 離された特殊キー
 x		: 特殊キーが離された時のマウスのX座標
 y		: 特殊キーが離された時のマウスのY座標
*/
EXTERN void KeyboardSpecialUpCallback(unsigned int key, int x, int y);

/*
 JoystickCallback関数
 ジョイスティックの状態が更新された時のコールバック関数
 引数
 button	: 状態が更新されたボタン
 x		: スティックのX座標
 y		: スティックのY座標
 z		: スティックのZ座標
*/
EXTERN void JoystickCallback(unsigned int button, int x, int y, int z);

/*
 MouseCallback関数
 マウスの状態が更新された時のコールバック関数
 引数
 button	: 状態の変更があったボタン
 state	: ボタンの状態
 x		: マウスのX座標
 y		: マウスのY座標	
*/
EXTERN void MouseCallback(int button, int state, int x, int y);

/*
 SetInputCallbacks関数
 入力に対するコールバック関数を設定する
*/
EXTERN void SetInputCallbacks(void);

/*
 LoadDefaultInputSettings関数
 入力設定のデフォルトデータを設定する
 引数
 input	: 入力処理全体を管理するデータ
*/
EXTERN void LoadDefaultInputSettings(GAME_INPUT* input);

/*
 UpdateInput関数
 入力状態を更新する
 引数
 input	: 入力処理全体を管理するデータ
*/
EXTERN void UpdateInput(GAME_INPUT* input);

#ifdef __cplusplus
}
#endif

#endif	// #ifndef _INCLUDED_INPUT_H_
