/* shar */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uthash.h"


#define KEY_ALLOC	"ALLOC"
#define KEY_FREE	"FREE"


struct alloc_block_t {
	void *block;
	int siz;
	UT_hash_handle hh;
};


struct alloc_block_t *table=NULL;
int numalloc=0, numfree=0;


int	allocare(void *block, const int siz);
int	checkare(FILE *fp);
int	deallocare(const void *block);
int	empaine(void);
int	tameio(void);


int
allocare(void *block, const int siz)
{
	struct alloc_block_t *p;

	p=(struct alloc_block_t *) malloc(sizeof(struct alloc_block_t));
	if(!p) {
		fprintf(stderr, "PANIC!\n");
		exit(1);
	}
	p->block=block;
	p->siz=siz;

	HASH_ADD_PTR(table, block, p);

	numalloc++;

	return 1;
}

int
deallocare(const void *block)
{
	struct alloc_block_t *p=NULL;

	numfree++;

	HASH_FIND_PTR(table, &block, p);
	if(p) {
		HASH_DEL(table, p);

		free(p);
		p=NULL;

		return 1;
	}

	printf("# trying to free invalid block %p\n", block);

	return 0;
}

int
empaine(void)
{
	return 1;
}

int
tameio(void)
{
	struct alloc_block_t *p;
	int j;

	if(!numalloc&&!numfree) {
		printf("# no allocations/deallocations - "
			"no leaker debug info???\n"
		);
		printf("Ok.\n");

		return 0;
	}

	j=HASH_COUNT(table);
	if(j)
		for(p=table; p; p=p->hh.next)
			printf("# didn't free block %p of size %d\n",
				p->block, p->siz
			);

	if(numalloc!=numfree)
		printf("# warning: "
			"number of allocations/deallocations don't match\n"
		);

	printf("# allocations: %d\t\tdeallocations: %d\n", numalloc, numfree);

	if(!j)
		printf("Ok.\n");
	else
		printf("Nonsense.\n");

	HASH_CLEAR(hh, table);

	return j;
}

int
checkare(FILE *fp)
{
	char *endptr, *s3, *s4;
	char sx[256], s1[256], s2[256], s5[256];
	void *block;
	int siz;

	empaine();

	while(!feof(fp))
		if(fgets(sx, 256, fp)) {
			s1[0]=s2[0]=0;
			s3=strstr(sx, KEY_ALLOC);
			s4=strstr(sx, KEY_FREE);
			if(s3) {
				if(3==sscanf(s3, "%s %s %s", s1, s2, s5)) {
					block=(void *)
						strtol(s2, &endptr, 16);
					siz=atoi(s5);
					allocare(block, siz);
				} else if(2==sscanf(s3, "%s %s", s1, s2)) {
					block=(void *)
						strtol(s2, &endptr, 16);
					siz=-42;
					allocare(block, siz);
				}
			}
			if(s4)
				if(sscanf(s4, "%s %s", s1, s2)) {
					block=(void *)
						strtol(s2, &endptr, 16);
					deallocare(block);
				}
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
