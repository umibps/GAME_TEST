#include "random.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 SeedXorShift32�֐�
 32bitXorShift�[������������̏�����
 ����
 table	: �����e�[�u��
 seed	: �����̎�
*/
void SeedXorShift32(XOR_SHIFT32* table, random_uint32 seed)
{
	table->buffer = (seed == 0) ? 2463534242UL : seed;
}

/*
 XorShift32GetNext�֐�
 32bitXorShift�̎��̒l���擾����
 ����
 table	: �����e�[�u��
 �Ԃ�l
	0�`0xFFFFFFFF�̕�������32bit�����̒l
*/
random_uint32 XorShift32GetNext(XOR_SHIFT32* table)
{
	random_uint32 x = table->buffer;
	x ^= (x << 13);
	x ^= (x >> 17);
	table->buffer = (x ^= (x << 5));
	return x;
}

/*
 SeedXorShift64�֐�
 64bitXorShift�[������������̏�����
 ����
 table	: �����e�[�u��
 seed	: �����̎�
*/
void SeedXorShift64(XOR_SHIFT64* table, random_uint32 seed)
{
	table->buffer[0] = seed = 1812433253UL * (seed ^ (seed >> 30)) + 1;
	table->buffer[1] = 1812433253UL * (seed ^ (seed >> 30)) + 2;
}

/*
 XorShift64GetNext�֐�
 64bitXorShift�̎��̒l���擾����
 ����
 table	: �����e�[�u��
 �Ԃ�l
	0�`0xFFFFFFFF�̕�������32bit�����̒l
*/
random_uint32 XorShift64GetNext(XOR_SHIFT64* table)
{
#define A 10
#define B 13
#define C 10
	random_uint32 t, y;
	random_uint32 *a = table->buffer;
	t = a[0];
	t = (t ^ (t << A));
	a[0] = y = a[1];
	t = (y ^ (y >> C)) ^ (t ^ (t >> B));
	a[1] = t;

	return t;
#undef A
#undef B
#undef C
}

/*
 SeedXorShift96�֐�
 96bitXorShift�[������������̏�����
 ����
 table	: �����e�[�u��
 seed	: �����̎�
*/
void SeedXorShift96(XOR_SHIFT96* table, random_uint32 seed)
{
	int i;
	for(i=0; i<3; i++)
	{
		table->buffer[i] = seed = 1812433253UL * (seed ^ (seed >> 30)) + i+1;
	}
}

/*
 XorShift96GetNext�֐�
 96bitXorShift�̎��̒l���擾����
 ����
 table	: �����e�[�u��
 �Ԃ�l
	0�`0xFFFFFFFF�̕�������32bit�����̒l
*/
random_uint32 XorShift96GetNext(XOR_SHIFT96* table)
{
#define A 10
#define B 1
#define C 26
	random_uint32 t, z;
	random_uint32 *a = table->buffer;
	t = a[0];
	t = (t ^ (t << A));
	a[0] = a[1];
	a[1] = z = a[2];
	t = (z ^ (z >> C)) ^ (t ^ (t >> B));
	a[2] = t;

	return t;
#undef A
#undef B
#undef C
}

/*
 SeedXorShift128�֐�
 128bitXorShift�[������������̏�����
 ����
 table	: �����e�[�u��
 seed	: �����̎�
*/
void SeedXorShift128(XOR_SHIFT128* table, random_uint32 seed)
{
	int i;
	for(i=0; i<4; i++)
	{
		table->buffer[i] = seed = 1812433253UL * (seed ^ (seed >> 30)) + i+1;
	}
}

/*
 XorShift128GetNext�֐�
 128bitXorShift�̎��̒l���擾����
 ����
 table	: �����e�[�u��
 �Ԃ�l
	0�`0xFFFFFFFF�̕�������32bit�����̒l
*/
random_uint32 XorShift128GetNext(XOR_SHIFT128* table)
{
#define A 11
#define B 8
#define C 19
	random_uint32 t, w;
	random_uint32 *a = table->buffer;
	t = a[0];
	a[0] = a[1];
	a[1] = a[2];
	a[2] = w = a[3];
	t = t ^ (t << A);
	w = (w ^ (w >> C)) ^ (t ^ (t >> B));
	a[3] = w;

	return w;
#undef A
#undef B
#undef C
}

#define ROTATE(X, R) (((X) << (R) | ((X) >> (sizeof((X))*8-(R)))))

/*
 RanrotGetNext�֐�
 RantotB�[�������̎��̒l���擾����
 ����
 table	: �����e�[�u��
 �Ԃ�l
	0�`0xFFFFFFFF�̕�������32bit����
*/
random_int32 RanrotGetNext(RANROT* table)
{
	random_uint32 result;

	// ��������
	result = table->buffer[table->point1] = ROTATE(table->buffer[table->point2], RANROT_R1)
		+ ROTATE(table->buffer[table->point1], RANROT_R2);

	// �Q�ƈʒu���X�V
	table->point1--;
	if(table->point1 < 0)
	{
		table->point1 = 0;
	}
	table->point2--;
	if(table->point2 < 0)
	{
		table->point2 = 0;
	}

	return result;
}

/*
 RanrotSetRange�֐�
 RantotB���������闐���͈̔͂�ݒ肷��
 ����
 table		: �����e�[�u��
 minimum	: �������闐���̍ŏ��l
 maximum	: �������闐���̍ő�l
*/
void RanrotSetRange(RANROT* table, random_int32 minimum, random_int32 maximum)
{
	if(minimum > maximum)
	{
		table->maximum = minimum;
		table->minimum = maximum;
	}
	else
	{
		table->minimum = minimum;
		table->maximum = maximum;
	}
	table->range = (random_uint32)(table->maximum - table->minimum);
}

/*
 SeedRanrot�֐�
 RanrotB�[�����������������������
 ����
 table	: �����e�[�u��
 seed	: �����̎�
*/
void SeedRanrot(RANROT* table, random_uint32 seed)
{
	int i;

	RanrotSetRange(table, 0, INTEGER_RANROT_MAX);

	if(seed == 0)
	{
		seed--;
	}

	for(i=0; i<RANROT_KK; i++)
	{
		seed ^= seed << 13;
		seed ^= seed >> 17;
		seed ^= seed << 5;
		table->buffer[i] = seed;
	}

	table->point1 = 0;
	table->point2 = RANROT_JJ;

	// ������Ԃ̕΂�����
	for(i=0; i<10; i++)
	{
		(void)RanrotGetNext(table);
	}
}

/*
 RanrotGetNextInteger�֐�
 RanrotB�̎��̐����l���擾����
 ����
 talbe	: �����e�[�u��
 �Ԃ�l
	�w�肳�ꂽ�͈�(����0�`0x7FFFFFFF)�̕����t��32bit�����l
*/
random_int32 RanrotGetNextInteger(RANROT* table)
{
	if(table->range <= 0)
	{
		return 0;
	}

	return (RanrotGetNext(table) % table->range) + table->minimum;
}

/*
 RanrotGetNextFloat�֐�
 RanrotB�̎���0�`1�̔{���x���������̒l���擾����
 ����
 table	: �����e�[�u��
 �Ԃ�l
	0�`1�̔{���x���������̒l
*/
random_float RanrotGetNextFloat(RANROT* table)
{
	return RanrotGetNext(table) * (1.0/4294967295.0);
}

/*
 ShuffleArray�֐�
 int�^�̔z����V���b�t������
 ����
 target			: �V���b�t������z��(NULL�w��\)
 size			: �z��̃T�C�Y
 start			: �J�n�ʒu(�ŏ��������_���ɂ������ꍇ��-1)
 random_func	: ���������Ɏg���֐��|�C���^
 random_table	: ���������p�̃f�[�^
 �Ԃ�l
	�V���b�t�����ꂽ�z��
	(target��NULL���w�肵���ꍇ��MEM_ALLOC_FUNC�����z��)
*/
int* ShuffleArray(
	int* target,
	int size,
	int start,
	random_int32 (*random_func)(void*),
	void* random_table
)
{
	int *rest;				// �܂����o���Ă��Ȃ��v�f
	int remain = size - 1;	// �܂����o���Ă��Ȃ��v�f�̐�
	int next;				// ���Ɏ��o���v�f�̃C���f�b�N�X
	int i, j;

	// �z��̃T�C�Y�`�F�b�N
	if(size <= 0)
	{
		return target;
	}

	if(target == NULL)
	{
		target = (int*)MEM_ALLOC_FUNC(sizeof(*target)*size);
	}

	// �V���b�t���p�̈ꎞ�ۑ��̈���m��
	rest = (int*)MEM_ALLOC_FUNC(sizeof(*rest)*size);
	for(i=0; i<size; i++)
	{
		rest[i] = i;
	}

	if(start < 0 || start >= size)
	{	// �J�n�ʒu���w�肳��Ă��Ȃ��̂Ń����_���ɑI��
		target[0] = (int)((unsigned int)random_func(random_table) % (unsigned int)size);
	}
	else
	{	// �J�n�ʒu��ݒ�
		target[0] = start;
	}
	// �ŏ��̗v�f���܂����o���Ă��Ȃ��v�f�z�񂩂�폜
	for(i=target[0]; i<remain-1; i++)
	{
		rest[i] = rest[i+1];
	}

	// �c��̗v�f��I��
	for(i=1; remain > 0; i++, remain--)
	{	// ���̒l������
		next = (int)((unsigned int)random_func(random_table) % (unsigned int)remain);
		target[i] = rest[next];
		// ���o�����v�f���܂����o���Ă��Ȃ��v�f�z�񂩂�폜
		for(j=next; j<remain-1; j++)
		{
			rest[j] = rest[j+1];
		}
	}

	MEM_FREE_FUNC(rest);

	return target;
}

#ifdef __cplusplus
}
#endif
