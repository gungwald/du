#include <stdlib.h>
#include "du.h"
#include "string-utils.h"
#include "error-handling.h"

bool isGlob(const _TCHAR *argument)
{
	return _tcschr(argument, _TEXT('*')) != NULL || _tcschr(argument, _TEXT('?')) != NULL;
}

_TCHAR *concat(const _TCHAR *left, const _TCHAR *right)
{
	size_t capacity;
	_TCHAR *result;
	errno_t errorCode;

	capacity = _tcslen(left) + _tcslen(right) + 1;
	if ((result = (_TCHAR *) malloc(capacity * sizeof(_TCHAR))) != NULL) {
		if ((errorCode = _tcscpy_s(result, capacity, left)) == 0) {
			if ((errorCode = _tcscat_s(result, capacity, right)) != 0) {
				writeError2(errorCode, _TEXT("Count will be off because string concatenation failed"), result, right);
				free(result);
				result = NULL;
			}
		}
		else {
			writeError(errorCode, _TEXT("Count will be off because string copy failed"), left);
			free(result);
			result = NULL;
		}
	}
	else {
		writeError2(errno, _TEXT("Memory allocation failed for string concatenation"), left, right);
		exit(EXIT_FAILURE);
	}
	return result;
}

_TCHAR *concat3(const _TCHAR *first, const _TCHAR *second, const _TCHAR *third)
{
	size_t requiredCapacity;
	errno_t errorCode;
	_TCHAR *result;

	requiredCapacity = _tcslen(first) + _tcslen(second) + _tcslen(third) + 1;
	if ((result = (_TCHAR *) malloc(requiredCapacity * sizeof(_TCHAR))) != NULL) {
		if ((errorCode = _tcscpy_s(result, requiredCapacity, first)) == 0) {
			if ((errorCode = _tcscat_s(result, requiredCapacity, second)) == 0) {
				if ((errorCode = _tcscat_s(result, requiredCapacity, third)) != 0) {
					writeError2(errorCode, _TEXT("Count will be off because string concatenation failed"), result, third);
					free(result);
					result = NULL;
				}
			}
			else {
				writeError2(errorCode, _TEXT("Count will be off because string concatenation failed"), result, second);
				free(result);
				result = NULL;
			}
		}
		else {
			writeError(errorCode, _TEXT("Count will be off because string copy failed"), first);
			free(result);
			result = NULL;
		}
	}
	else {
		writeError3(errno, _TEXT("Memory allocation failed for string concatenation"), first, second, third);
		exit(EXIT_FAILURE);
	}
	return result;
}

/**
 * s is modified.
 */
_TCHAR *replaceAll(_TCHAR *s, _TCHAR searchFor, _TCHAR replaceWith)
{
	_TCHAR *p;

	for (p = s; *p != '\0'; p++) {
		if (*p == searchFor) {
			*p = replaceWith;
		}
	}
	return s;
}

/**
 * Returns a newly allocated string that must be free'd.
 */
_TCHAR *dirname(const _TCHAR *path)
{
	_TCHAR *duplicatePath;
	_TCHAR *lastBackslash;

	duplicatePath = _tcsdup(path);
	replaceAll(duplicatePath, _TEXT('/'), _TEXT('\\'));	/* Make sure all separators are backslashes. */
	lastBackslash = _tcsrchr(duplicatePath, '\\');		/* Find the last backslash. */
	if (lastBackslash != NULL) {
		*lastBackslash = '\0';							/* Terminate the string at the last backslash. */
	}
	else {
		*duplicatePath = '\0';							/* Make it an empty string. */
	}
	return duplicatePath;
}
