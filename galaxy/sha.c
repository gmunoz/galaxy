#include <stdlib.h>
#include <stdio.h>
#include <openssl/sha.h>

int main(int argc, char **argv)
{
	unsigned char md[SHA_DIGEST_LENGTH];

	printf("SHA_DIGEST_LENGTH: %d\n", SHA_DIGEST_LENGTH);

	SHA1("foobar", 6, md);
	int i;
	for (i = 0; i < SHA_DIGEST_LENGTH; i++)
		printf("%02x", md[i]);
	printf("\n");

	return 0;
}
