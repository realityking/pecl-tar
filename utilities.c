/*
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,       |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt.                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Rouven Weßling <me@rouvenwessling.de>                      |
   +----------------------------------------------------------------------+
*/

#include "utilities.h"
#include "stdio.h"

#ifdef __cplusplus
extern "C"
{
#endif

int detect_tar_type(const char *pathname)
{
	FILE *fp;
	char buffer[2];
	size_t result;
	char hex[10];

	fp = fopen(pathname, "r");
	if (fp == NULL) {
		return -1;
	}

	result = fread(buffer, 1, 2, fp);
	if (result != 2) {
		return -1;
	}

	if ((buffer[0] == (char)0x1f) && (buffer[1] == (char)0x8b)) {
		return 1;
	}

	if ((buffer[0] == 'B') && (buffer[1] == 'Z')) {
		return 2;
	}

	return 0;
}

#ifdef __cplusplus
}
#endif
