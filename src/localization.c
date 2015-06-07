#include <pebble.h>
#include "localization.h"

void initLocales(void)
{
	char *locale = setlocale(LC_ALL, "");
	ResHandle localeFile;

	if (strcmp("fr_FR", locale) == 0) {
		localeFile = resource_get_handle(RESOURCE_ID_FRENCH_LOCALE);
	} else {
		localeFile = resource_get_handle(RESOURCE_ID_ENGLISH_LOCALE);
	}

	int fileSize = resource_size(localeFile);

	uint8_t *strings = malloc(fileSize);
	resource_load(localeFile, strings, fileSize);

	for (int i = 0, strCurrentId = 0, strStartPosition = 0; i < fileSize; i++) {
		switch (strings[i]) {
			case '|':
				strings[i] = '\n';
				break;
			case '\n':
				locale = malloc(i - strStartPosition);
				strncpy(locale, (char *) &strings[strStartPosition], i - strStartPosition);
				locale[i - strStartPosition] = '\0';
				str[strCurrentId++] = locale;
				strStartPosition = i + 1;
		}
	}

	free(strings);
}

void deinitLocales(void)
{
	for (int i = 0; i < STRING_NUMBER; i++)
		if (str[i])
			free(str[i]);
}
