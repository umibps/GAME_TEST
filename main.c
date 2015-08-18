#include <time.h>
#include "game_data.h"
#include "initialize.h"
#include "random.h"
#include "script/lexical_analyzer.h"

int main(int argc, char** argv)
{
	FILE_ARCHIVE archive;
	FILE *fp;
	LEXICAL_ANALYZER analyzer;
	TOKEN **token;
	int i;

	if((fp = fopen("test.bin", "rb")) == NULL)
	{
		return 1;
	}

	InitializeFileArchive(&archive, "test.bin", fp, fread,
		fseek, ftell, fclose);
	
	InitializeLexicalAnalyzer(&analyzer, "scripts/test.txt",
		NULL, NULL);
	LexicalAnalyze(&analyzer, FileArchiveReadNew, FileArchiveRead, FileArchiveSeek,
		FileArchiveTell, DeleteFileArchiveRead, &archive);
	token = (TOKEN**)analyzer.tokens.buffer;
	for(i=0; i<(int)analyzer.tokens.num_data; i++)
	{
		if(token[i]->name != NULL)
		{
			printf("%s\n", token[i]->name);
		}
	}
	ReleaseLexicalAnalyzer(&analyzer);

	ReleaseFileArchive(&archive);

#ifdef _WIN32
	(void)system("pause");
#endif

	return 0;
}
