/* BRAIN FUCK COMPILER FOR BARE BONES 16 BIT */
#include<stdio.h>
#include<stdlib.h>

#define MAX_STACK_SIZE 1000  // Set a reasonable stack size for loop nesting

int main(int c, char ** v)
{
	if (c != 3) return 1;
	FILE * fp = fopen(v[1], "r");
	FILE * ft = fopen("TEMP.ASM", "w");
	if (!fp) {
		printf("Error opening file: %s\n", v[1]);
		return 1;
	}

	char x;
	int loop_stack[MAX_STACK_SIZE];
	int stack_pointer = 0;

	/* Compile Bootloader */
	system("nasm src/bootloader.asm -o bin/bootloader.bin");
	fprintf(ft, "\tBITS 16\n_START:\n");

	while ((x = fgetc(fp)) != EOF)
	{
		switch (x)
		{
			case '+':
				fprintf(ft, "\tINC BYTE[ES:SI]\n");
				break;
			case '-':
				fprintf(ft, "\tDEC BYTE[ES:SI]\n");
				break;
			case '>':
				fprintf(ft, "\tINC SI\n");
				break;
			case '<':
				fprintf(ft, "\tDEC SI\n");
				break;
			case '.':
				fprintf(ft, "\tCALL PUT\n");
				break;
			case ',':
				fprintf(ft, "\tCALL GET\n");
				break;
			case '[':
				if (stack_pointer < MAX_STACK_SIZE)
				{
					loop_stack[stack_pointer++] = ftell(fp);
				}
				fprintf(ft, "\tCMP BYTE[ES:SI], 0\n\tJE SKIP_LOOP_%ld\n", ftell(fp));
				fprintf(ft, "LOOP_%ld:\n", ftell(fp));
				break;
			case ']':
				if (stack_pointer > 0)
				{
					int loop_start_pos = loop_stack[--stack_pointer];
					fprintf(ft, "\tCMP BYTE[ES:SI], 0\n\tJNZ LOOP_%d\n", loop_start_pos);
					fprintf(ft, "SKIP_LOOP_%d:\n", loop_start_pos); // Label for skipping the loop
				}
				break;
			/* Extensions */
			case 'j':
				fprintf(ft, "\tMOV BX,WORD[ES:SI]\n");
				fprintf(ft, "\tJMP BX\n");
				break;
			case 'P':
				fprintf(ft, "\tMOV WORD[ES:SI],SI\n");
				break;
			case 'p':
				fprintf(ft, "\tMOV SI,WORD[ES:SI]\n");
				break;
			default:
				if (x > 32)
					printf("? : %c\n",x);
				break;
		}
	}

	fprintf(ft, "INF:\n\tJMP INF\n");
	fprintf(ft, "PUT:\n\tMOV AL,BYTE[ES:SI]\n\tMOV AH,0xE\n\tINT 0x10\n\tRET\n");
	fprintf(ft, "GET:\n\tXOR AX,AX\n\tINT 0x16\n\tMOV BYTE[ES:SI],AL\n\tRET\n");

	fclose(fp);
	fclose(ft);

	/* Compile Temp */
	system("nasm TEMP.ASM -o TEMP.BIN");
	char command[256];
	snprintf(command, sizeof(command), "cat bin/bootloader.bin TEMP.BIN > %s", v[2]);
	system(command);
}
