#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

char *encrypt(char *string, int distance) {
	int length, i;
	char *encrypted_string, base;

	length = strlen(string);
	encrypted_string = (char *) malloc((length + 1) * sizeof(char));
	
	for (i = 0; i < length; i++) {
		if (isalpha(string[i])) {
			if (isupper(string[i])) {
				base = 'A';
			} else {
				base = 'a';
			}
			encrypted_string[i] = (string[i] + distance - base) % 26 + base;
		} else {
			encrypted_string[i] = string[i];
		}
	}
	encrypted_string[length] = '\0';
	
	return encrypted_string;
}

void decrypt(char *string, int distance) {
	int length, i;
	char base;

	length = strlen(string);

	for (i = 0; i < length; i++) {
		if (isalpha(string[i])) {
			if (isupper(string[i])) {
				base = 'A';
			} else {
				base = 'a';
			}
			string[i] = (string[i] - distance - base + 26) % 26 + base;
		}
	}
}

int main(int argc, char *argv[]) {

	int distance;
	char *string, *encrypted_string;
	
	
	if (argc != 4) {
		fprintf(stderr, "Usage: [encrypt/decrypt] [distance] [string]\n");
		exit(EXIT_FAILURE);
	}
	
	distance = atoi(argv[2]);
	
    if (distance < 1 || distance > 25) {
		fprintf(stderr, "Distance must be between 1 and 25\n");
		exit(EXIT_FAILURE);
	}

	string = argv[3];

	if (strcmp(argv[1], "encrypt") == 0) {
		encrypted_string = encrypt(string, distance);
		printf("%s\n", encrypted_string);
		free(encrypted_string);
	} else if (strcmp(argv[1], "decrypt") == 0) {
		decrypt(string, distance);
		printf("%s\n", string);
	} else {
		fprintf(stderr, "Use 'encrypt' or 'decrypt'\n");
		exit(EXIT_FAILURE);
	}

	return 0;
}
