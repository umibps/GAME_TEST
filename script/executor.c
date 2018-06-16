// Visual Studio 2005以降では古いとされる関数を使用するので
// 警告が出ないようにする
#if defined _MSC_VER && _MSC_VER >= 1400
# define _CRT_SECURE_NO_DEPRECATE
# define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include <string.h>
#include <math.h>
#include "executor.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 InitializeScriptBasicExecutor関数
 デフォルトのスクリプト実行用データを初期化する
 引数
 executor			: デフォルトのスクリプト実行用データ
 code				: バイトコード
 code_size			: バイトコードのサイズ
 user_function		: 組み込み関数の関数ポインタ配列
 num_user_function	: 組み込み関数の数
 user_data			: 組み込み関数で使用するデータ
*/
void InitializeScriptBasicExecutor(
	SCRIPT_BASIC_EXECUTOR* executor,
	uint8* code,
	size_t code_size,
	script_user_function* user_function,
	int num_user_function,
	void* user_data
)
{
#define REGISTER_BLOCK_SIZE 128
#define MEMORY_POOL_BLOCK_SIZE 4096
	(void)memset(executor, 0, sizeof(*executor));

	executor->code = code;
	executor->code_size = code_size;
	executor->user_function = user_function;
	executor->num_user_function = num_user_function;
	executor->user_data = user_data;

	InitializeStructArray(&executor->register_buffer, sizeof(SCRIPT_BASIC_ARGUMENT),
		REGISTER_BLOCK_SIZE, NULL);
	InitializeStructArray(&executor->heap_buffer, sizeof(SCRIPT_BASIC_ARGUMENT),
		REGISTER_BLOCK_SIZE, NULL);
	InitializeStructArray(&executor->local_buffer, sizeof(SCRIPT_BASIC_ARGUMENT),
		REGISTER_BLOCK_SIZE, NULL);
	InitializeMemoryPool(&executor->memory_pool, MEMORY_POOL_BLOCK_SIZE);
}

/*
 ReleaseScriptBasicExecutor関数
 デフォルトのスクリプト実行用データを開放する
 引数
 executor	: デフォルトのスクリプト実行用データ
*/
void ReleaseScriptBasicExecutor(SCRIPT_BASIC_EXECUTOR* executor)
{
	ReleaseStructArray(&executor->register_buffer);
	ReleaseStructArray(&executor->heap_buffer);
	ReleaseStructArray(&executor->local_buffer);
	ReleaseMemoryPool(&executor->memory_pool);
}

/*
 IsRegisterZero関数
 レジスターのデータが0か調べる
 引数
 register_data	: 0か調べるレジスターのデータ
 返り値
	0:TRUE	0以外:FALSE
*/
static int IsRegisterZero(SCRIPT_BASIC_ARGUMENT* register_data)
{
	if(register_data->type == SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER && register_data->value.integer_value == 0
		|| register_data->type == SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT && register_data->value.float_value == 0.0
		|| register_data->type == SCRIPT_BASIC_ARGUMENT_TYPE_STRING && register_data->value.string_value[0] == '0' && atoi(register_data->value.string_value) == 0
		|| register_data->type == SCRIPT_BASIC_ARGUMENT_TYPE_STRING && StringCompareIgnoreCase(register_data->value.string_value, "FALSE") == 0)
	{
		return TRUE;
	}
	return FALSE;
}

/*
 ScriptBasicExecutorExecute関数
 スクリプトのバイトコードを実行する
 引数
 executor	: デフォルトのスクリプト実行用データ
 返り値
	スクリプトの返り値(デフォルトは0)
*/
int ScriptBasicExecutorExecute(SCRIPT_BASIC_EXECUTOR* executor)
{
	script_operand *operand;
	script_index *index;
	SCRIPT_BASIC_ARGUMENT *argument;
	const int end = (int)executor->code_size;
	int local_start = 0;
	int result = 0;

	while(executor->current_position < end)
	{
		operand = (script_operand*)&executor->code[executor->current_position];
		executor->current_position += sizeof(*operand);
		switch(*operand)
		{
		case SCRIPT_OPERAND_STORE:
			{
				SCRIPT_BASIC_ARGUMENT *store_target;
				SCRIPT_BASIC_ARGUMENT *registers = (SCRIPT_BASIC_ARGUMENT*)executor->register_buffer.buffer;
				SCRIPT_BASIC_ARGUMENT *local = (SCRIPT_BASIC_ARGUMENT*)executor->local_buffer.buffer;
				script_index *local_index, *register_index;
				register_index = (script_index*)&executor->code[executor->current_position];
				executor->current_position += sizeof(*register_index);
				local_index = (script_index*)&executor->code[executor->current_position];
				executor->current_position += sizeof(*local_index);
				*local_index += local_start;

				if(*local_index < executor->local_buffer.num_data)
				{
					store_target = &local[*local_index];
				}
				else
				{
					int i = 0;
					do
					{
						store_target = (SCRIPT_BASIC_ARGUMENT*)StructArrayAllocateNewItem(&executor->local_buffer);
						i++;
					} while(i < *local_index);
				}

				*store_target = registers[*register_index];
				executor->register_buffer.num_data--;
			}
			break;
		case SCRIPT_OPERAND_RESTORE:
			{
				SCRIPT_BASIC_ARGUMENT *registers = (SCRIPT_BASIC_ARGUMENT*)executor->register_buffer.buffer;
				SCRIPT_BASIC_ARGUMENT *local = (SCRIPT_BASIC_ARGUMENT*)executor->local_buffer.buffer;
				SCRIPT_BASIC_ARGUMENT *target = (SCRIPT_BASIC_ARGUMENT*)StructArrayAllocateNewItem(&executor->register_buffer);
				script_index *restore_index, *register_index;
				register_index = (script_index*)&executor->code[executor->current_position];
				executor->current_position += sizeof(*register_index);
				restore_index = (script_index*)&executor->code[executor->current_position];
				executor->current_position += sizeof(*restore_index);
				*restore_index += local_start;

				*target = local[*restore_index];
				executor->register_buffer.num_data++;
			}
			break;
		case SCRIPT_OPERAND_GLOBAL_STORE:
			{
				SCRIPT_BASIC_ARGUMENT *store_target;
				SCRIPT_BASIC_ARGUMENT *registers = (SCRIPT_BASIC_ARGUMENT*)executor->register_buffer.buffer;
				SCRIPT_BASIC_ARGUMENT *global = (SCRIPT_BASIC_ARGUMENT*)executor->heap_buffer.buffer;
				script_index *global_index, *register_index;
				register_index = (script_index*)&executor->code[executor->current_position];
				executor->current_position += sizeof(*register_index);
				global_index = (script_index*)&executor->code[executor->current_position];
				executor->current_position += sizeof(*global_index);

				if(*global_index < executor->heap_buffer.num_data)
				{
					store_target = &global[*global_index];
				}
				else
				{
					int i = 0;
					do
					{
						store_target = (SCRIPT_BASIC_ARGUMENT*)StructArrayAllocateNewItem(&executor->heap_buffer);
						i++;
					} while(i < *global_index);
				}

				*store_target = registers[*register_index];
				executor->register_buffer.num_data--;
			}
			break;
		case SCRIPT_OPERAND_GLOBAL_RESTORE:
			{
				SCRIPT_BASIC_ARGUMENT *registers = (SCRIPT_BASIC_ARGUMENT*)executor->register_buffer.buffer;
				SCRIPT_BASIC_ARGUMENT *global = (SCRIPT_BASIC_ARGUMENT*)executor->heap_buffer.buffer;
				SCRIPT_BASIC_ARGUMENT *target = (SCRIPT_BASIC_ARGUMENT*)StructArrayAllocateNewItem(&executor->register_buffer);
				script_index *restore_index, *register_index;
				register_index = (script_index*)&executor->code[executor->current_position];
				executor->current_position += sizeof(*register_index);
				restore_index = (script_index*)&executor->code[executor->current_position];
				executor->current_position += sizeof(*restore_index);

				*target = global[*restore_index];
			}
			break;
		case SCRIPT_OPERAND_PLUS:
			{
				SCRIPT_BASIC_ARGUMENT *registers =
					(SCRIPT_BASIC_ARGUMENT*)&executor->register_buffer.buffer[sizeof(SCRIPT_BASIC_ARGUMENT)*(executor->register_buffer.num_data-2)];
				switch(registers[0].type)
				{
				case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
					switch(registers[1].type)
					{
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.integer_value += registers[1].value.integer_value;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].type = SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT;
						registers[0].value.float_value = registers[0].value.integer_value + registers[1].value.float_value;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
						(void)sprintf(executor->string_buffer, "%d%s", registers[0].value.integer_value, registers[1].value.string_value);
						break;
					}
					break;
				case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
					switch(registers[1].type)
					{
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.float_value += registers[1].value.integer_value;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].value.float_value += registers[1].value.float_value;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
						(void)sprintf(executor->string_buffer, SCRIPT_FLOAT_PRINT_FORMAT "%s", registers[0].value.float_value, registers[1].value.string_value);
						break;
					}
					break;
				case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
					switch(registers[1].type)
					{
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						(void)sprintf(executor->string_buffer, "%s%d", registers[0].value.string_value, registers[1].value.integer_value);
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						(void)sprintf(executor->string_buffer, "%s"SCRIPT_FLOAT_PRINT_FORMAT, registers[0].value.string_value, registers[1].value.float_value);
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
						(void)sprintf(executor->string_buffer, "%s%s", registers[0].value.string_value, registers[1].value.float_value);
						break;
					}
					break;
				}
			}
			executor->current_position += sizeof(script_index) * 2;
			executor->register_buffer.num_data--;
			break;
		case SCRIPT_OPERAND_MINUS:
			{
				SCRIPT_BASIC_ARGUMENT *registers =
					(SCRIPT_BASIC_ARGUMENT*)&executor->register_buffer.buffer[sizeof(SCRIPT_BASIC_ARGUMENT)*(executor->register_buffer.num_data-2)];
				switch(registers[0].type)
				{
				case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
					switch(registers[1].type)
					{
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.integer_value -= registers[1].value.integer_value;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].type = SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT;
						registers[0].value.float_value = (script_float)registers[0].value.integer_value - registers[1].value.float_value;
						break;
					}
					break;
				case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
					switch(registers[1].type)
					{
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.float_value -= registers[1].value.integer_value;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].value.float_value -= registers[1].value.float_value;
						break;
					}
					break;
				}
			}
			executor->current_position += sizeof(script_index) * 2;
			executor->register_buffer.num_data--;
			break;
		case SCRIPT_OPERAND_MULTI:
			{
				SCRIPT_BASIC_ARGUMENT *registers =
					(SCRIPT_BASIC_ARGUMENT*)&executor->register_buffer.buffer[sizeof(SCRIPT_BASIC_ARGUMENT)*(executor->register_buffer.num_data-2)];
				switch(registers[0].type)
				{
				case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
					switch(registers[1].type)
					{
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.integer_value *= registers[1].value.integer_value;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].type = SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT;
						registers[0].value.float_value = (script_float)registers[0].value.integer_value * registers[1].value.float_value;
						break;
					}
					break;
				case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
					switch(registers[1].type)
					{
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.float_value *= registers[1].value.integer_value;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].value.float_value *= registers[1].value.float_value;
						break;
					}
					break;
				}
			}
			executor->current_position += sizeof(script_index) * 2;
			executor->register_buffer.num_data--;
			break;
		case SCRIPT_OPERAND_DIVIDE:
			{
				SCRIPT_BASIC_ARGUMENT *registers =
					(SCRIPT_BASIC_ARGUMENT*)&executor->register_buffer.buffer[sizeof(SCRIPT_BASIC_ARGUMENT)*(executor->register_buffer.num_data-2)];
				switch(registers[0].type)
				{
				case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
					switch(registers[1].type)
					{
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						if(registers[1].value.integer_value != 0)
						{
							registers[0].value.integer_value /= registers[1].value.integer_value;
						}
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						if(registers[1].value.float_value != 0.0)
						{
							registers[0].type = SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT;
							registers[0].value.float_value = (script_float)registers[0].value.integer_value / registers[1].value.float_value;
						}
						break;
					}
					break;
				case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
					switch(registers[1].type)
					{
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						if(registers[1].value.integer_value != 0)
						{
							registers[0].value.float_value /= registers[1].value.integer_value;
						}
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						if(registers[1].value.float_value != 0.0)
						{
							registers[0].value.float_value /= registers[1].value.float_value;
						}
						break;
					}
					break;
				}
			}
			executor->current_position += sizeof(script_index) * 2;
			executor->register_buffer.num_data--;
			break;
		case SCRIPT_OPERAND_MOD:
			{
				SCRIPT_BASIC_ARGUMENT *registers =
					(SCRIPT_BASIC_ARGUMENT*)&executor->register_buffer.buffer[sizeof(SCRIPT_BASIC_ARGUMENT)*(executor->register_buffer.num_data-2)];
				switch(registers[0].type)
				{
				case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
					switch(registers[1].type)
					{
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						if(registers[1].value.integer_value != 0)
						{
							registers[0].value.integer_value %= registers[1].value.integer_value;
						}
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						if(fabs(registers[1].value.float_value) >= 1.0)
						{
							registers[0].value.integer_value = registers[0].value.integer_value % (int)registers[1].value.float_value;
						}
						break;
					}
					break;
				case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
					switch(registers[1].type)
					{
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						if(registers[1].value.integer_value != 0)
						{
							registers[0].type = SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER;
							registers[0].value.integer_value = (int)registers[0].value.float_value % registers[1].value.integer_value;
						}
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						if(fabs(registers[1].value.float_value) >= 1.0)
						{
							registers[0].type = SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER;
							registers[0].value.integer_value = (int)registers[0].value.float_value % (int)registers[1].value.float_value;
						}
						break;
					}
					break;
				}
			}
			executor->current_position += sizeof(script_index) * 2;
			executor->register_buffer.num_data--;
			break;
		case SCRIPT_OPERAND_EQUAL:
		case SCRIPT_OPERAND_NOT_EQUAL:
			{
				SCRIPT_BASIC_ARGUMENT *registers =
					(SCRIPT_BASIC_ARGUMENT*)&executor->register_buffer.buffer[sizeof(SCRIPT_BASIC_ARGUMENT)*(executor->register_buffer.num_data-2)];
				switch(registers[0].type)
				{
				case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
					switch(registers[1].type)
					{
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.integer_value = (registers[0].value.integer_value ==  registers[1].value.integer_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].value.integer_value = (registers[0].value.integer_value == (int)registers[1].value.float_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
						registers[0].value.integer_value = (registers[0].value.integer_value == atoi(registers[1].value.string_value)) ? TRUE : FALSE;
						break;
					}
					break;
				case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
					switch(registers[1].type)
					{
						registers[0].type = SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER;
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.integer_value = ((int)registers[0].value.float_value == registers[1].value.integer_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].value.integer_value = (registers[0].value.float_value == registers[1].value.float_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
						registers[0].value.integer_value = (registers[0].value.float_value == atof(registers[1].value.string_value)) ? TRUE : FALSE;
						break;
					}
					break;
				case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
					switch(registers[1].type)
					{
						registers[0].type = SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER;
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.integer_value = (atoi(registers[0].value.string_value) == registers[1].value.integer_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].value.integer_value = (atof(registers[0].value.string_value) == registers[1].value.float_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
						registers[0].value.integer_value = (strcmp(registers[0].value.string_value, registers[1].value.string_value) == 0) ? TRUE : FALSE;
						break;
					}
					break;
				}
				if(*operand == SCRIPT_OPERAND_NOT_EQUAL)
				{
					registers[0].value.integer_value = !registers[0].value.integer_value;
				}
			}
			executor->current_position += sizeof(script_index) * 2;
			executor->register_buffer.num_data--;
			break;
		case SCRIPT_OPERAND_LESS:
			{
				SCRIPT_BASIC_ARGUMENT *registers =
					(SCRIPT_BASIC_ARGUMENT*)&executor->register_buffer.buffer[sizeof(SCRIPT_BASIC_ARGUMENT)*(executor->register_buffer.num_data-2)];
				switch(registers[0].type)
				{
				case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
					switch(registers[1].type)
					{
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.integer_value = (registers[0].value.integer_value <  registers[1].value.integer_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].value.integer_value = (registers[0].value.integer_value < (int)registers[1].value.float_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
						registers[0].value.integer_value = (registers[0].value.integer_value < atoi(registers[1].value.string_value)) ? TRUE : FALSE;
						break;
					}
					break;
				case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
					switch(registers[1].type)
					{
						registers[0].type = SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER;
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.integer_value = ((int)registers[0].value.float_value < registers[1].value.integer_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].value.integer_value = (registers[0].value.float_value < registers[1].value.float_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
						registers[0].value.integer_value = (registers[0].value.float_value < atof(registers[1].value.string_value)) ? TRUE : FALSE;
						break;
					}
					break;
				case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
					switch(registers[1].type)
					{
						registers[0].type = SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER;
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.integer_value = (atoi(registers[0].value.string_value) < registers[1].value.integer_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].value.integer_value = (atof(registers[0].value.string_value) < registers[1].value.float_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
						registers[0].value.integer_value = (strcmp(registers[0].value.string_value, registers[1].value.string_value) < 0) ? TRUE : FALSE;
						break;
					}
					break;
				}
			}
			executor->current_position += sizeof(script_index) * 2;
			executor->register_buffer.num_data--;
			break;
		case SCRIPT_OPERAND_LESS_EQUAL:
			{
				SCRIPT_BASIC_ARGUMENT *registers =
					(SCRIPT_BASIC_ARGUMENT*)&executor->register_buffer.buffer[sizeof(SCRIPT_BASIC_ARGUMENT)*(executor->register_buffer.num_data-2)];
				switch(registers[0].type)
				{
				case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
					switch(registers[1].type)
					{
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.integer_value = (registers[0].value.integer_value <=  registers[1].value.integer_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].value.integer_value = (registers[0].value.integer_value <= (int)registers[1].value.float_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
						registers[0].value.integer_value = (registers[0].value.integer_value <= atoi(registers[1].value.string_value)) ? TRUE : FALSE;
						break;
					}
					break;
				case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
					switch(registers[1].type)
					{
						registers[0].type = SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER;
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.integer_value = ((int)registers[0].value.float_value <= registers[1].value.integer_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].value.integer_value = (registers[0].value.float_value <= registers[1].value.float_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
						registers[0].value.integer_value = (registers[0].value.float_value <= atof(registers[1].value.string_value)) ? TRUE : FALSE;
						break;
					}
					break;
				case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
					switch(registers[1].type)
					{
						registers[0].type = SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER;
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.integer_value = (atoi(registers[0].value.string_value) <= registers[1].value.integer_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].value.integer_value = (atof(registers[0].value.string_value) <= registers[1].value.float_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
						registers[0].value.integer_value = (strcmp(registers[0].value.string_value, registers[1].value.string_value) <= 0) ? TRUE : FALSE;
						break;
					}
					break;
				}
			}
			executor->current_position += sizeof(script_index) * 2;
			executor->register_buffer.num_data--;
			break;
		case SCRIPT_OPERAND_GREATER:
			{
				SCRIPT_BASIC_ARGUMENT *registers =
					(SCRIPT_BASIC_ARGUMENT*)&executor->register_buffer.buffer[sizeof(SCRIPT_BASIC_ARGUMENT)*(executor->register_buffer.num_data-2)];
				switch(registers[0].type)
				{
				case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
					switch(registers[1].type)
					{
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.integer_value = (registers[0].value.integer_value >  registers[1].value.integer_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].value.integer_value = (registers[0].value.integer_value > (int)registers[1].value.float_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
						registers[0].value.integer_value = (registers[0].value.integer_value > atoi(registers[1].value.string_value)) ? TRUE : FALSE;
						break;
					}
					break;
				case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
					switch(registers[1].type)
					{
						registers[0].type = SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER;
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.integer_value = ((int)registers[0].value.float_value > registers[1].value.integer_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].value.integer_value = (registers[0].value.float_value > registers[1].value.float_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
						registers[0].value.integer_value = (registers[0].value.float_value > atof(registers[1].value.string_value)) ? TRUE : FALSE;
						break;
					}
					break;
				case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
					switch(registers[1].type)
					{
						registers[0].type = SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER;
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.integer_value = (atoi(registers[0].value.string_value) > registers[1].value.integer_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].value.integer_value = (atof(registers[0].value.string_value) > registers[1].value.float_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
						registers[0].value.integer_value = (strcmp(registers[0].value.string_value, registers[1].value.string_value) > 0) ? TRUE : FALSE;
						break;
					}
					break;
				}
			}
			executor->current_position += sizeof(script_index) * 2;
			executor->register_buffer.num_data--;
			break;
		case SCRIPT_OPERAND_GREATER_EQUAL:
			{
				SCRIPT_BASIC_ARGUMENT *registers =
					(SCRIPT_BASIC_ARGUMENT*)&executor->register_buffer.buffer[sizeof(SCRIPT_BASIC_ARGUMENT)*(executor->register_buffer.num_data-2)];
				switch(registers[0].type)
				{
				case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
					switch(registers[1].type)
					{
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.integer_value = (registers[0].value.integer_value >=  registers[1].value.integer_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].value.integer_value = (registers[0].value.integer_value >= (int)registers[1].value.float_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
						registers[0].value.integer_value = (registers[0].value.integer_value >= atoi(registers[1].value.string_value)) ? TRUE : FALSE;
						break;
					}
					break;
				case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
					switch(registers[1].type)
					{
						registers[0].type = SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER;
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.integer_value = ((int)registers[0].value.float_value >= registers[1].value.integer_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].value.integer_value = (registers[0].value.float_value >= registers[1].value.float_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
						registers[0].value.integer_value = (registers[0].value.float_value >= atof(registers[1].value.string_value)) ? TRUE : FALSE;
						break;
					}
					break;
				case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
					switch(registers[1].type)
					{
						registers[0].type = SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER;
					case SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER:
						registers[0].value.integer_value = (atoi(registers[0].value.string_value) >= registers[1].value.integer_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_FLOAT:
						registers[0].value.integer_value = (atof(registers[0].value.string_value) >= registers[1].value.float_value) ? TRUE : FALSE;
						break;
					case SCRIPT_BASIC_ARGUMENT_TYPE_STRING:
						registers[0].value.integer_value = (strcmp(registers[0].value.string_value, registers[1].value.string_value) >= 0) ? TRUE : FALSE;
						break;
					}
					break;
				}
			}
			executor->current_position += sizeof(script_index) * 2;
			executor->register_buffer.num_data--;
			break;
		case SCRIPT_OPERAND_LOAD_CONST_INTEGER:
			{
				script_int *data;
				argument = StructArrayAllocateNewItem(&executor->register_buffer);
				argument->type = SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER;

				index = (script_index*)&executor->code[executor->current_position];
				executor->current_position += sizeof(*index);
				data = (script_int*)&executor->code[executor->current_position];
				argument->value.integer_value = *data;
				executor->current_position += sizeof(*data);
			}
			break;
		case SCRIPT_OPERAND_LOAD_CONST_FLOAT:
			{
				script_float *data;
				argument = StructArrayAllocateNewItem(&executor->register_buffer);
				argument->type = SCRIPT_BASIC_ARGUMENT_TYPE_INTEGER;

				index = (script_index*)&executor->code[executor->current_position];
				executor->current_position += sizeof(*index);
				data = (script_float*)&executor->code[executor->current_position];
				argument->value.float_value = *data;
				executor->current_position += sizeof(*data);
			}
			break;
		case SCRIPT_OPERAND_LOAD_CONST_STRING:
			{
				size_t length;
				argument = StructArrayAllocateNewItem(&executor->register_buffer);
				argument->type = SCRIPT_BASIC_ARGUMENT_TYPE_STRING;

				index = (script_index*)&executor->code[executor->current_position];
				executor->current_position += sizeof(*index);
				length = strlen((const char*)&executor->code[executor->current_position]) + 1;
				argument->value.string_value = (char*)MemoryPoolAllocate(&executor->memory_pool, length);
				(void)memcpy(argument->value.string_value, &executor->code[executor->current_position], length + 1);
				executor->current_position += length;
			}
			break;
		case SCRIPT_OPERAND_JUMP_IF_ZERO:
			{
				SCRIPT_BASIC_ARGUMENT *registers = (SCRIPT_BASIC_ARGUMENT*)executor->register_buffer.buffer;
				SCRIPT_BASIC_ARGUMENT *register_data;
				index = (script_index*)&executor->code[executor->current_position];
				register_data = &registers[*index];
				if(IsRegisterZero(register_data) == FALSE)
				{
					executor->current_position += sizeof(*index) * 2;
				}
				else
				{
					executor->current_position += sizeof(*index);
					index = (script_index*)&executor->code[executor->current_position];
					executor->current_position = *index;
				}
				executor->register_buffer.num_data--;
			}
			break;
		case SCRIPT_OPERAND_JUMP:
			index = (script_index*)&executor->code[executor->current_position];
			executor->current_position = *index;
			break;
		case SCRIPT_OPERAND_CALL_USER_FUNCTION:
			{
				SCRIPT_BASIC_ARGUMENT function_result;
				script_index function_index, num_argument;
				index = (script_index*)&executor->code[executor->current_position];
				executor->current_position += sizeof(*index);
				function_index = *index;
				index = (script_index*)&executor->code[executor->current_position];
				executor->current_position += sizeof(*index);
				num_argument = *index;
				function_result = executor->user_function[function_index](executor, executor->user_data,
					(SCRIPT_BASIC_ARGUMENT*)&executor->register_buffer.buffer[sizeof(SCRIPT_BASIC_ARGUMENT) * (executor->register_buffer.num_data - num_argument)], num_argument);
				index = (script_index*)&executor->code[executor->current_position];
				executor->current_position += sizeof(*index);
				executor->register_buffer.num_data -= num_argument;
				if(*index == FALSE)
				{
					StructArrayAppend(&executor->register_buffer, (void*)&function_result);
				}
			}
			break;
		}
	}

	return result;
}

#ifdef __cplusplus
}
#endif
