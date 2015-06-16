#include <pebble.h>
#include "localization.h"

void initLocales(void)
{
	char *locale = setlocale(LC_ALL, "");

	ResHandle localeFile;
	if (strcmp("fr_FR", locale) == 0) {
		localeFile = resource_get_handle(RESOURCE_ID_FRENCH_LOCALE);
	} else if (strcmp("de_DE", locale) == 0) {
		localeFile = resource_get_handle(RESOURCE_ID_GERMAN_LOCALE);
	} else if (strcmp("es_ES", locale) == 0) {
		localeFile = resource_get_handle(RESOURCE_ID_SPANISH_LOCALE);
	} else {
		localeFile = resource_get_handle(RESOURCE_ID_ENGLISH_LOCALE);
	}

	size_t fileSize = resource_size(localeFile);

	uint8_t *strings = malloc(fileSize);
	resource_load(localeFile, strings, fileSize);

	for (uint16_t i = 0, strCurrentId = 0, strStartPosition = 0; i < fileSize; i++) {
		switch (strings[i]) {
			case '|':
				strings[i] = '\n';
				break;
			case '\n':
				locale = malloc(i - strStartPosition);
				memcpy(locale, &strings[strStartPosition], i - strStartPosition);
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
