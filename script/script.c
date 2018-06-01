#include <stdarg.h>
#include "script.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 DefaultScriptErrorMessage�֐�
 �f�t�H���g�̃X�N���v�g�������̃G���[�\���֐�
 ����
 dummy
 file_name	: �G���[�̂������t�@�C����
 line		: �G���[�̔��������s�� (0�̏ꍇ�̓X�N���v�g�̓��e�ȊO�̃G���[)
 message	: �G���[���b�Z�[�W
 ...		: printf�Ɠ���
*/
void DefaultScriptErrorMessage(
	void* dummy,
	const char* file_name,
	int line,
	const char* message,
	...
)
{
	va_list list;

	if(line > 0)
	{
		(void)fprintf(stderr, "At line %d, in %s\n", line, file_name);
	}

	va_start(list, message);

	(void)vfprintf(stderr, message, list);

	va_end(list);
}

/*
 InitializreScriptBasicExecutor�֐�
 �f�t�H���g�̃X�N���v�g���s�p�f�[�^��������
 ����
 executor				: �f�t�H���g�̃X�N���v�g���s�p�f�[�^
 file_path				: ���s����X�N���v�g�̃t�@�C���p�X
 user_function_names	: ���[�U�[��`�֐��̖��O�z��
 num_user_functions		: ���[�U�[��`�֐��̐�
 user_data				: ���[�U�[��`�֐��Ŏg�����[�U�[�f�[�^
 error_message			: �G���[���b�Z�[�W�\���p�֐�
 error_message_data		: �G���[���b�Z�[�W�\���p�֐��Ŏg�����[�U�[�f�[�^
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
 ReleaseScriptBasicExecutor�֐�
 �f�t�H���g�̃X�N���v�g���s�p�f�[�^�̃������J��
 ����
 executor	: �f�t�H���g�̃X�N���v�g���s�p�f�[�^
*/
void ReleaseScriptBasicExecutor(SCRIPT_BASIC_EXECUTOR* executor)
{
	ReleaseLexicalAnalyser(&executor->analyser);
	executor->parser.element.release(&executor->parser.element);
	ReleaseMemoryPool(&executor->memory_pool);
}

/*
 ScriptBasicEcecutorExecute�֐�
 �f�t�H���g�̃X�N���v�g���s�p�f�[�^�ŃX�N���v�g���s
 ����
 executor	: �f�t�H���g�̃X�N���v�g���s�p�f�[�^
 open_func	: �\�[�X�R�[�h�f�[�^���J�����߂̊֐��|�C���^
 read_func	: �\�[�X�R�[�h�f�[�^��ǂݍ��ނ��߂̊֐��|�C���^
 seek_func	: �\�[�X�R�[�h�f�[�^���V�[�N���邽�߂̊֐��|�C���^
 tell_func	: �\�[�X�R�[�h�f�[�^�̃V�[�N�ʒu���擾���邽�߂̊֐��|�C���^
 close_func	: �\�[�X�R�[�h�f�[�^����邽�߂̊֐��|�C���^
 open_data	: �\�[�X�R�[�h���J���ۂɎg���O���f�[�^
 �Ԃ�l
	����I��:TRUE	�ُ�I��:FALSE
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
	const char **reserved_string;
	uint16 reserved_ids[] = {SCRIPT_BASIC_RESERVED_IF, SCRIPT_BASIC_RESERVED_ELSE};
	int (*rules[])(struct _SCRIPT_RULE_ELEMENT* rule, struct _LEXICAL_ANALYSER* analyser, void* function_data)
		= {ScriptBasicIfRule, ScriptBasicElseRule};
	int (*parse_functions[])(struct _SCRIPT_PARSER_ELEMENT* parser, int token_id, ABSTRACT_SYNTAX_TREE* parent)
		= {ScriptBasicParserParseIf, ScriptBasicParserParseElse};
	int num_strings;

	reserved_string = ScriptBasicGetDefaultReservedStrings(&num_strings);
	LexicalAnalyserSetReserved(&executor->analyser, reserved_string,
		reserved_ids, sizeof(rules) / sizeof(*rules));

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
	ScriptRuleElementSetReservedRule(&executor->rule.element,
		rules,  sizeof(rules) / sizeof(*rules), NULL);
	ScriptParserElementSetReservedParseRule(&executor->parser.element,
		parse_functions);

	if(executor->parser.element.parse(&executor->parser.element) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

/*
 ScriptBasicGetDefaultReservedStrings�֐�
 �f�t�H���g�̗\���̕�����z����擾����
 ����
 num_strings	: �\���̐�
 �Ԃ�l
	�f�t�H���g�̗\���̕�����z��
*/
const char** ScriptBasicGetDefaultReservedStrings(int* num_strings)
{
	static const char *reserved[] ={"if", "else", "while", "break"};
	if(num_strings != NULL)
	{
		*num_strings = sizeof(reserved) / sizeof(char*);
	}

	return reserved;
}

#ifdef __cplusplus
}
#endif
