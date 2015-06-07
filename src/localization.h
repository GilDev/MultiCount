#ifndef LOCALIZATION
#define LOCALIZATION

#define STRING_NUMBER 9
enum localizations {LC_ADD, LC_DELETE, LC_ABOUT, LC_ADD_NAME, LC_ADD_INITIAL_VALUE, LC_ADD_ERROR_NO_NAME, LC_ADD_ERROR_NO_PLACE_LEFT, LC_DELETE_ERROR_NO_COUNTER, LC_ABOUT_TEXT};

char *str[STRING_NUMBER];

#define lc(x) (str[x])
void initLocales(void);
void deinitLocales(void);

#endif