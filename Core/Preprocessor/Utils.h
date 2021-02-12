#pragma once

/*
* @brief Conctenates both arguments
*/
#define PP_CAT(arg1, arg2) arg1 ## arg2

/*
* @brief Convert teh argument to a string literal
* @param arg
*/
#define PP_STR(arg) _PP_STR(arg)
#define _PP_STR(arg) #arg

