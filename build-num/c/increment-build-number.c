/**
 * increment-build-number.c
 *
 *  Created on: Dec 22, 2021
 *      Author: bill_
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define LINE_SIZE 256

static const char *readLine(FILE *f, const char *name);
static char *chomp(char *s);
static size_t convertStringToCardinal(char *s);

int main(int argc, const char *argv[])
{
    char *lastSpace;
    char *buildNumberString;
    size_t buildNumber;
    size_t nextBuildNumber;
    const char *line;
    const char *buildNumberFileName;
    FILE *buildNumberFile;

    if (argc != 2) {
        fprintf(stderr, "%s: an argument is required\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    buildNumberFileName = argv[1];

    if ((buildNumberFile = fopen(buildNumberFileName, "r")) == NULL) {
        perror(buildNumberFileName);
        exit(EXIT_FAILURE);
    }

    line = readLine(buildNumberFile, buildNumberFileName);
    fclose(buildNumberFile);
    if ((lastSpace = strrchr(line, ' ')) == NULL) {
        fprintf(stderr, "Invalid syntax in build number file: %s\n", buildNumberFileName);
        exit(EXIT_FAILURE);
    }
    buildNumberString = lastSpace + 1;
    buildNumber = convertStringToCardinal(buildNumberString);

    nextBuildNumber = buildNumber + 1;

    if ((buildNumberFile = fopen(buildNumberFileName, "w")) == NULL) {
        perror(buildNumberFileName);
        exit(EXIT_FAILURE);
    }
    fprintf(buildNumberFile, "#define BUILD_NUMBER %zu\n", nextBuildNumber);
    fclose(buildNumberFile);
    printf("Incremented build number to %zu\n", nextBuildNumber);
    return EXIT_SUCCESS;
}

static const char *readLine(FILE *f, const char *name)
{
    static char line[LINE_SIZE];
    const char *FGETS_FAILURE = NULL;

    if (fgets(line, sizeof(line), f) == FGETS_FAILURE) {
        if (ferror(f)) {
            perror(name);
            exit(EXIT_FAILURE);
        } else if (feof(f)) {
            fprintf(stderr, "File %s is empty\n", name);
            exit(EXIT_FAILURE);
        }
    }
    return (const char *) chomp(line);
}

static char *chomp(char *s)
{
    char *ptr;

    /* Find the end of s */
    for (ptr = s; *ptr; ptr++);
    /* Back up, replacing line terminators with string terminator */
    for (ptr--; *ptr == '\r' || *ptr == '\n'; ptr--)
        *ptr = '\0';
    return s;
}

static size_t convertStringToCardinal(char *s)
{
    char *tail;
    long long result;

    result = strtoll(s, &tail, 10);
    if (result == LONG_LONG_MAX) {
        fprintf(stderr, "Number value of string is too large: %s\n", s);
        exit(EXIT_FAILURE);
    }
    if (result == LONG_LONG_MIN) {
        fprintf(stderr, "Number value of string is too small: %s\n", s);
        exit(EXIT_FAILURE);
    }
    return result;
}
