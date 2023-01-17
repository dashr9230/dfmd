
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

void check_bytes(uint8_t *buf, uint8_t count)
{
	puts("");

	for(uint8_t i = 0; i < count; i++)
	{
		printf("%02X ", *(uint8_t*)(buf + i));
	}
	
	puts("");
}

void getstring(uint8_t* &buf, uint8_t* out)
{
	uint8_t len;

	len = *(uint8_t*)buf++;
	memcpy(out, buf, len);
	out[len] = 0;
	
	buf += len;
}

void iprintf(uint32_t indentcount, char* fmt, ...)
{
	char buf[1024];
	va_list vl;

	indentcount *= 2;

	for(uint32_t i = 0; i < indentcount; i += 2)
	{
		buf[i+0] = ' ';
		buf[i+1] = ' ';
	}
	
	va_start(vl, fmt);
	vsnprintf(buf + indentcount, sizeof(buf) - indentcount, fmt, vl);
	va_end(vl);
	
	printf(buf);
}

void print_data(uint32_t indentcount, uint8_t* &buf)
{
	uint32_t i, size;
	char indent[256];

	indentcount++;
	indentcount *= 2;
	
	for(i = 0; i < indentcount; i += 2)
	{
		indent[i+0] = ' ';
		indent[i+1] = ' ';
	}
	indent[i] = 0;

	size = *(uint32_t*)buf;
	buf += 4;

	printf("{\n%s", indent);

	for(i = 0; i < size; i++)
	{
		if(((i+1) % 32) == 0)
		{
			printf("%02X", *(uint8_t*)buf++);
			printf("\n%s", indent);
		}
		else
		{
			printf("%02X", *(uint8_t*)buf++);
		}
	}
	
	puts("}");
}

void print_record(uint32_t indentcount, uint8_t* &buf)
{
	uint8_t i, size, value[256];
	char indent[256];

	indentcount++;
	indentcount *= 2;
	
	for(i = 0; i < indentcount; i += 2)
	{
		indent[i+0] = ' ';
		indent[i+1] = ' ';
	}
	indent[i] = 0;

	buf++; // skip 06 ??? what is that?
	
	puts("(");
	
	while(*(uint8_t*)buf != 0x00)
	{
		getstring(buf, value);
		printf("%s'%s'", indent, value);
		
		printf("%s", (*(uint8_t*)buf != 0x06) ? ")\n" : "\n");
		
		if(*(uint8_t*)buf == 0x06) buf++;
	}

	buf++;

	/*size = *(uint8_t*)buf++;
	size /= 2;

	puts("(");

	for(i = 0; i < size; i++)
	{
		getstring(buf, value);
		printf("%s'%s'", indent, value);
		buf++;
		
		printf("%s", i == (size-1) ? ")\n" : "\n");
	}*/
}

void print_float(uint8_t* &buf)
{
	printf("TODO: float ");
	check_bytes(buf, 10);
	
	buf += 10;
}

void print_collections(uint32_t &indentcount, uint8_t* &buf)
{
	uint8_t vartype, varname[256];

	printf("<\n");
	
	indentcount++;
	buf++; // skip 01
	
	while(*(uint8_t*)buf != 0x00)
	{
		iprintf(indentcount++, "item\n");

		while(*(uint8_t*)buf != 0x00)
		{
			getstring(buf, varname);
			iprintf(indentcount, "%s = ", varname);
				
			vartype = *(uint8_t*)buf++;

			if(vartype == 8 || vartype == 9)
			{		
				printf("%s\n", vartype == 8 ? "False" : "True");
			}
			else if(vartype == 6)
			{
				getstring(buf, varname);
				printf("'%s'\n", varname);
			}
			else if(vartype == 3)
			{
				printf("%d\n", *(uint16_t*)buf);
				buf += 2;
			}
			else if(vartype == 2)
			{
				printf("%d\n", *(int8_t*)buf);
				buf++;
			}
			else
			{
				printf("TODO: collections %d\n", vartype);
				check_bytes(buf, 5);
				exit(1);
			}
		}
		
		iprintf(--indentcount, "end");
		
		if(*(uint8_t*)(buf+1) == 0x01)
		{
			printf("\n");
			buf++;
		}

		buf++;
	}

	buf++;

	printf(">\n");
	
	indentcount--;
	
	/*uint8_t vartype, varname[256];
			
	indentcount++;
			
	vartype = *(uint8_t*)buf++;
	for(uint8_t i = 0; i < vartype; i++)
	{
		iprintf(indentcount++, "item\n");
		
		getstring(buf, varname);
		iprintf(indentcount, "%s = ", varname);
		
		buf++; // skip vartype
				
		printf("%d\n", *(uint16_t*)buf);
				
		buf+=2;
		
		iprintf(--indentcount, "end");
	}
		
	printf(">\n");
			
	indentcount--;
		
	buf += 2;*/
			
	//check_bytes(buf, 5);
}

int main(int argc, char* argv[])
{
	FILE *fp;
	uint32_t filesize, indentcount;
	uint8_t *buf, *origbuf;
	uint8_t vartype, isprop;
	uint8_t vartypename[256], varname[256];

	if(argc != 2) {
		puts("Usage: dfmd.exe <file name>");
		return 0;
	}

	fp = fopen(argv[1], "rb");
	if(fp == NULL)
	{
		printf("'%s' not found.\n", argv[1]);
		return 0;
	}
	
	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	if(filesize <= 4)
	{
		puts("Invalid or empty file.");
		fclose(fp);
		return 0;	
	}
	
	fseek(fp, 0, SEEK_SET);
	
	//printf("filesize %d\n", filesize);
	
	buf = (uint8_t*)malloc(filesize);
	if(buf == NULL)
	{
		puts("malloc failed.");
		fclose(fp);
		return 0;
	}
	origbuf = buf;
	
	fread(buf, filesize, 1, fp);	
	fclose(fp);
	
	if(memcmp(buf, "TPF0", 4) != 0)
	{
		puts("Invalid binary file.");
		free(origbuf);
		return 0;
	}
	
	buf += 4; // skip "TPF0"
	
	getstring(buf, vartypename);
	getstring(buf, varname);
		
	printf("object %s: %s\n", varname, vartypename);
	
	isprop = 1;
	indentcount = 1;
	
	while(buf < origbuf + filesize)
	{
		if(*(uint8_t*)buf == 0)
		{
			if(*(uint8_t*)(buf + 1) == 0)
			{
				indentcount--;
				
				iprintf(indentcount, "end\n");
				
				buf++;
				
				if(indentcount == 0)
					break;
				else
					continue;
			}
			else
			{
				buf++;

				getstring(buf, vartypename);
				getstring(buf, varname);
	
				iprintf(indentcount, "object %s: %s\n", varname, vartypename);

				indentcount++;
			
				continue;
			}
		}
		else
		{
			getstring(buf, varname);
			iprintf(indentcount, "%s = ", varname);
		}
		
		vartype = *(uint8_t*)buf++;
		
		if(vartype == 14) // collections
		{
			print_collections(indentcount, buf);
		}
		else if(vartype == 11) // [] set
		{
			if(*(uint8_t*)buf++ == 0)
				printf("[]\n");
			else
			{
				--buf;

				getstring(buf, varname);
				printf("[%s]\n", varname);
				
				buf++;
			}
		}
		else if(vartype == 10) // {} data
		{
			print_data(indentcount, buf);
		}
		else if(vartype == 8 || vartype == 9)
		{
			printf("%s\n", vartype == 8 ? "False" : "True");
		}
		else if(vartype == 7) // color
		{
			getstring(buf, varname);
			
			printf("%s\n", varname);
		}
		else if(vartype == 6) // string
		{
			getstring(buf, varname);
			
			printf("'%s'\n", varname);
		}
		else if(vartype == 5) // float
		{
			print_float(buf);
		}
		else if(vartype == 4) // 4 byte integer
		{
			printf("%d\n", *(int32_t*)buf);
			buf += 4;
		}
		else if(vartype == 3) // 2 byte integer
		{
			printf("%d\n", *(int16_t*)buf);
			buf += 2;
		}
		else if(vartype == 2) // 1 byte integer
		{
			printf("%d\n", *(int8_t*)buf);
			buf++;
		}
		else if(vartype == 1) // () records
		{
			print_record(indentcount, buf);
		}
		else
		{
			printf("TODO: %d (main)\n", vartype);
			break;	
		}
	}
	
	free(origbuf);

	return 0;
}
