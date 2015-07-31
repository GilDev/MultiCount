#ifndef LOCALIZATION
#define LOCALIZATION

enum localizations {LC_ADD = 0, LC_ABOUT, LC_RESET_ALL, LC_ADD_NAME, LC_ADD_INITIAL_VALUE, LC_ADD_ERROR_NO_NAME, LC_ADD_ERROR_NO_PLACE_LEFT, LC_ABOUT_TEXT, STRING_NUMBER};

extern char *str[STRING_NUMBER];

#define lc(x) (str[x])
void initLocales(void);
void deinitLocales(void);

#endif