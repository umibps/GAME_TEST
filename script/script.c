#include "script.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializreScriptBasicExecutor関数
 デフォルトのスクリプト実行用データを初期化
 引数
 executor				: デフォルトのスクリプト実行用データ
 file_path				: 実行するスクリプトのファイルパス
 user_function_names	: ユーザー定義関数の名前配列
 num_user_functions		: ユーザー定義関数の数
 user_data				: ユーザー定義関数で使うユーザーデータ
 error_message			: エラーメッセージ表示用関数
 error_message_data		: エラーメッセージ表示用関数で使うユーザーデータ
*/
void InitializeScriptBasicExecutor(
	SCRIPT_BASIC_EXECUTOR* executor,
	const char* file_path,
	const char** user_function_names,
	int num_user_functions,
	void* user_data,
	void (*error_message)(void*, const char*, ...),
	void* error_message_data
)
{
#define MEMORY_BLOCK_SIZE 4096
	(void)memset(executor, 0, sizeof(*executor));
	executor->user_function_names = user_function_names;
	executor->num_user_functions = num_user_functions;
	executor->user_data = user_data;

	InitializeLexicalAnalyser(&executor->analyser, file_path, error_message, error_message_data);
	InitializeMemoryPool(&executor->memory_pool, MEMORY_BLOCK_SIZE);
#undef MEMORY_BLOCK_SIZE
}

/*
 ReleaseScriptBasicExecutor関数
 デフォルトのスクリプト実行用データのメモリ開放
 引数
 executor	: デフォルトのスクリプト実行用データ
*/
void ReleaseScriptBasicExecutor(SCRIPT_BASIC_EXECUTOR* executor)
{
	ReleaseLexicalAnalyser(&executor->analyser);
	executor->parser.element.release(&executor->parser.element);
	ReleaseMemoryPool(&executor->memory_pool);
}

/*
 ScriptBasicEcecutorExecute関数
 デフォルトのスクリプト実行用データでスクリプト実行
 引数
 executor	: デフォルトのスクリプト実行用データ
 open_func	: ソースコードデータを開くための関数ポインタ
 read_func	: ソースコードデータを読み込むための関数ポインタ
 seek_func	: ソースコードデータをシークするための関数ポインタ
 tell_func	: ソースコードデータのシーク位置を取得するための関数ポインタ
 close_func	: ソースコードデータを閉じるための関数ポインタ
 open_data	: ソースコードを開く際に使う外部データ
 返り値
	正常終了:TRUE	異常終了:FALSE
*/
int ScriptBasicExecutorExecute(
	SCRIPT_BASIC_EXECUTOR* executor,
	void* (*open_func)(const char*, const char*, void*),
	size_t (*read_func)(void*, size_t, size_t, void*),
	void* (*seek_func)(void*, long, int),
	long (*tell_func)(void*),
	int (*close_func)(void*),
	void* open_data
)
{
	if(LexicalAnalyse(&executor->analyser, open_func, read_func,
		seek_func, tell_func, close_func, open_data) == FALSE)
	{
		return FALSE;
	}

	InitializeScriptBasicRule(&executor->rule, executor->analyser.error_message_func,
		executor->analyser.error_message_func_data, (const char**)executor->analyser.file_names.buffer, &executor->memory_pool);
	InitializeScriptBasicParser(&executor->parser, &executor->rule.element,
		(TOKEN**)executor->analyser.tokens.buffer, (int)executor->analyser.tokens.num_data,
		executor->user_function_names, executor->num_user_functions, executor->user_data
	);

	if(executor->parser.element.parse(&executor->parser.element) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

#ifdef __cplusplus
}
#endif
