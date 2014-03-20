#include <iostream.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define		FALSE		0
#define		TRUE		1

enum sections
{
	none,
	patchNames,
	noteNames,
	controllerNames,
	instrumentDefinitions
};

void trimend(char *t)
{
	char *x = t + strlen(t) - 1;

	while (!(isprint(x[0]) || (x == t)))
	{
		x[0] = 0;
		x--;
	}
}

int countPatches(char *fname)
{
	FILE *f = fopen(fname, "r");

	int patches = FALSE;
	int ret = 0;

	char t[80];
	bzero(t, 80);

	fgets(t, 79, f); trimend(t);
	while (!feof(f))
	{
		if (strncmp(t, ".Patch Names", 12) == 0)
			patches = TRUE;
		else if (t[0] == '.')
			patches = FALSE;
		else if (patches && isdigit(t[0]))
			ret++;

		fgets(t, 79, f); trimend(t);
	}

	fclose(f);
	return ret;
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cerr << "Usage: " << argv[0] << " cakewalkfile.ins\n";
		exit(-1);
	}
		
	FILE *inf = fopen(argv[1], "r");
	FILE *outf = fopen("other.jzi", "w");

	char t[256];
	bzero(t, 256);
	char bankname[80];

	int bank = -1;	// First bank will be -1+1 = 0
	int t_programs = 0;
	int b_programs = 0;
	enum sections currentSection = none;
	int numpatches = countPatches(argv[1]);

	fgets(t, 255, inf); trimend(t);
	while (!feof(inf))
	{
		if (t[0] == ';')
		{	// Comment line, change first character only
			t[0] = '#';
			fprintf(outf, "%s\n", t);
		}
		else if (strlen(t) == 1)
		{	// Blank line, don't change at all
			fprintf(outf, "\n");
		}
  		else if (t[0] == '[')
		{	
			switch (currentSection)
			{
				case (patchNames):
					// New bank
					if (b_programs != 0)
					{
						cout << "\no Processed " << b_programs
							<< " programs.\n\n";
					}
					cout << "o Reading bank " << t << " (#" << ++bank
						<< ") in.\n\n";
					b_programs = 0;
					bzero(bankname, 80);
					strncpy(bankname, t, 79);	
					break;
			}
		}
		else if (strncmp(t, ".Patch Names", 12) == 0)
		{	// Patch names directive
			fprintf(outf, ".max_voice_names %d\n",
				numpatches);
			fprintf(outf, ".voicenames\n");
			currentSection = patchNames;
		}
		else if (strncmp(t, ".Note Names", 11) == 0)
		{	// Note names section
			fprintf(outf, ".drumnames\n");

			currentSection = noteNames;
		}
		else if (strncmp(t, ".Controller Names", 17) == 0)
		{	// Controller names section
			fprintf(outf, ".ctrlnames\n");			

			currentSection = controllerNames;
		}
		else if (strncmp(t, ".Instrument Definitions", 23) == 0)
		{	// Instrument definitions -- NOT YET HANDLED
			currentSection = instrumentDefinitions;
		}
		else if ( isdigit(t[0]) && index(t, '=') )
		{	
			char *tok;
			int patchnum, controllernumber, notenum;

			switch (currentSection)
			{
				case (patchNames):	
					tok = strtok(t, "=\n");
					patchnum = atoi(tok);
					tok = strtok(NULL, "=\n");
					fprintf(outf, "%d %s %d %s\n",
						(bank * 256 + patchnum),
						bankname, patchnum, tok);
					printf("o \t%d %s\n", patchnum, tok);
					b_programs++; t_programs++;
					break;
				case (controllerNames):
					tok = strtok(t, "=\n");
					controllernumber = atoi(tok);
					tok = strtok(NULL, "=\n");
					fprintf(outf, "%d %s\n", controllernumber, tok);
					break;
				case (noteNames):
					tok = strtok(t, "=\n");
					int notenum = atoi(tok);
					tok = strtok(NULL, "=\n");
					fprintf(outf, "%d %s\n", notenum, tok);
					break;
			}
		}		
			
		fgets(t, 255, inf); trimend(t);
	}

	cout << "\no Processed " << t_programs << " programs total\n";

	fclose(inf); fclose(outf);
}
