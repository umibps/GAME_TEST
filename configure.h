#ifndef _INCLUDED_CONFIGURE_H_
#define _INCLUDED_CONFIGURE_H_

#define OPEN_FUNC FileOpen
#define CLOSE_FUNC FileClose
#define READ_FUNC fread
#define WRITE_FUNC fwrite
#define SEEK_FUNC fseek
#define TELL_FUNC ftell

#define DISPLAY_WIDTH 1280		// 描画領域の幅
#define DISPLAY_HEIGHT 720		// 描画領域の高さ

#define FRAMES_PER_SECOND 60	// フレームレート

#endif	// #ifndef _INCLUDED_CONFIGURE_H_
