/**
 * This file is part of Drystal.
 *
 * Drystal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Drystal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Drystal.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "log.h"

_printf_(3, 0) static void log_message(const char *level, const char *category, const char *format, va_list ap)
{
	fprintf(stderr, "[%s|%10s] ", level, category);
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
}

_printf_(3, 0) static void log_internalv(LogLevel level, const char *category, const char *format, va_list ap)
{
	switch (level) {
		case LOG_ERROR:
			log_message(stderr_use_colors() ? ANSI_HIGHLIGHT_RED_ON    "ERR " ANSI_RESET : "ERR ", category, format, ap);
			break;
		case LOG_WARNING:
			log_message(stderr_use_colors() ? ANSI_HIGHLIGHT_YELLOW_ON "WARN" ANSI_RESET : "WARN", category, format, ap);
			break;
		case LOG_INFO:
			log_message(stderr_use_colors() ? ANSI_HIGHLIGHT_ON        "INFO" ANSI_RESET : "INFO", category, format, ap);
			break;
		case LOG_DEBUG:
			log_message(stderr_use_colors() ? ANSI_HIGHLIGHT_GRAY_ON   "DBG " ANSI_RESET : "DBG ", category, format, ap);
			break;
	}
}

void log_oom_and_exit(void)
{
	fprintf(stderr, "Out of memory. Exiting.");
	exit(EXIT_FAILURE);
}

void log_internal(LogLevel level, const char *category, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	log_internalv(level, category, format, ap);
	va_end(ap);
}
