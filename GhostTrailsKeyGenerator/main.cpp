// GhostTrailsKeyGenerator.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "RegCode.h"
#include <conio.h>

// Encode and decode lots of randomly generated strings as a test
//
int v30_UnitTest(int runs)
{
	srand(time(0));

#ifdef	GT_V30
	DWORD lictype = RegCode::GHOSTTRAILSREG_30_PAID;

	for(int i = 0; i < runs; i++)
	{
		RegCode generator;
		int codeLength = generator.RegCodeLength(RegCode::REGVERSION_V2);

		char	pchBuff[1024];
		char	companyname[RegCode::NUM_COMPANYNAME_CHARS_V30 + 1];
		memset(companyname, 0, RegCode::NUM_COMPANYNAME_CHARS_V30 + 1);
		memset(pchBuff, 0, 1024);

		// generate a random company name
		int nameLength;
		if(i == 0)
			nameLength = 0;
		else if(i == 1)
			nameLength = RegCode::NUM_COMPANYNAME_CHARS_V30;
		else
		{
			nameLength = rand() % RegCode::NUM_COMPANYNAME_CHARS_V30;
		}

		for(int j = 0; j < nameLength; j++)
			companyname[j] = 32 + (rand() % (126 - 32));

		printf("[%s] ...", companyname);

		BOOL bRes = generator.GenerateRegCode(RegCode::REGVERSION_V2, lictype, companyname, pchBuff);
			
		if(bRes)
		{
			printf("OK\n");

			// test validation fails if we change the code
			int idx = rand() % RegCode::NUM_COMPANYNAME_CHARS_V30;
			pchBuff[idx]++;
			if(generator.ValidateRegCode(pchBuff) == 1)
			{
				printf("ERROR - [%s] should not validate\n", pchBuff);
				return 1;
			}
		}
		else
		{
			printf("ERROR\n");
			return 0;
		}
	}

	return 1;
#else
	return 0;
#endif
}

// --------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	if((strcmp(argv[1], "V30_TEST") == 0) && (argc >= 3))
	{
		return v30_UnitTest(atoi(argv[2]));
	}
	else if(argc!=2)
	{
		fprintf(stderr, "Usage: keygen \"Bill Smith\"\n");
		return 0;
	}

	
	RegCode generator;

#ifdef	GT_V30
	// regversion is the version number of the encryption algorithm - make this 1.
	// lictype determines the 'type' of license (trial, full, etc.)
	BYTE regversion = 2;
	int codeLength = generator.RegCodeLength(RegCode::REGVERSION_V2);
	DWORD lictype = RegCode::GHOSTTRAILSREG_30_PAID;
#else
	BYTE regversion = 1;
	int codeLength = generator.RegCodeLength(RegCode::REGVERSION_V1);
	DWORD lictype = RegCode::GHOSTTRAILSREG_FULL;
#endif

	char* pchBuff = new char[codeLength];
		
	BOOL bRes = generator.GenerateRegCode(regversion, lictype, argv[1], pchBuff);
		
	if(bRes){
		printf(pchBuff);
		//printf("The Bytegeist GhostTrails code for %s :\n%s\n", argv[1], pchBuff);
	}else{
		printf("Error calling GenerateRegCode\n");
	}
		
	delete [] pchBuff;

#ifdef _DEBUG
	char ch = getchar();
#endif
	return 0;
}

