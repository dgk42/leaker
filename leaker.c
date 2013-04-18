/* shar */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_BLOCKS	(65535*32)
#define BLOCK_POISON	(-1)
#define BLOCK_UNINIT	(-10)
#define KEY_ALLOC	"ALLOC"
#define KEY_FREE	"FREE"


long blocks[MAX_BLOCKS];
int currblock=0;
int numalloc=0, numfree=0;


int	allocare(const long x);
int	checkare(FILE *fp);
int	deallocare(const long x);
int	empaine(void);
int	tameio(void);


int
allocare(const long x)
{
	if(currblock>=MAX_BLOCKS) {
		fprintf(stderr, "PANIC!\n");
		exit(1);
	}
	blocks[currblock++]=x;
	numalloc++;

	return 1;
}

int
deallocare(const long x)
{
	int i;

	numfree++;
	for(i=0; i<currblock; i++) {
		if(x==blocks[i]&&BLOCK_POISON==blocks[i]) {
			printf("# trying to free previously freed block %p\n",
				(void *) x
			);

			return 0;
		}
		if(x==blocks[i]) {
			blocks[i]=BLOCK_POISON;

			return 1;
		}
	}

	printf("# trying to free unallocated block %p\n", (void *) x);

	return 0;
}

int
empaine(void)
{
	int i;

	for(i=0; i<MAX_BLOCKS; i++)
		blocks[i]=BLOCK_UNINIT;

	return 1;
}

int
tameio(void)
{
	int i, j;

	if(!numalloc&&!numfree) {
		printf("# no allocations/deallocations - "
			"no leaker debug info???\n"
		);
		printf("Ok.\n");

		return 0;
	}

	j=0;
	for(i=0; i<MAX_BLOCKS; i++) {
		if(BLOCK_POISON!=blocks[i]&&BLOCK_UNINIT!=blocks[i]) {
			printf("# didn't free block %p\n",
				(void *) blocks[i]
			);
			j++;
		}
	}

	if(numalloc!=numfree)
		printf("# warning: "
			"number of allocations/deallocations don't match\n"
		);

	printf("# allocations: %d\t\tdeallocations: %d\n", numalloc, numfree);

	if(!j)
		printf("Ok.\n");
	else
		printf("Nonsense.\n");

	return j;
}

int
checkare(FILE *fp)
{
	char *endptr, *s3, *s4;
	char sx[256], s1[256], s2[256];

	empaine();

	while(!feof(fp))
		if(fgets(sx, 256, fp)) {
			s1[0]=s2[0]=0;
			s3=strstr(sx, KEY_ALLOC);
			s4=strstr(sx, KEY_FREE);
			if(s3)
				if(sscanf(s3, "%s %s", s1, s2))
					allocare(strtol(s2, &endptr, 16));
			if(s4)
				if(sscanf(s4, "%s %s", s1, s2))
					deallocare(strtol(s2, &endptr, 16));
		}

	return tameio();
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	int i;

	if(1==argc) {
		printf("# checking: stdin\n");
		checkare(stdin);
	} else {
		for(i=1; i<argc; i++) {
			printf("\n# checking: \"%s\"\n", argv[i]);
			fp=fopen(argv[i], "r");
			if(!fp) {
				fprintf(stderr, "#file not found\n");
				continue;
			}

			checkare(fp);

			fclose(fp);
		}
	}

	return 0;
}
