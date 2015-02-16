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
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "macro.h"
#include "util.h"

typedef enum LogLevel
{
	LOG_ERROR,
	LOG_WARNING,
	LOG_INFO,
	LOG_DEBUG,
} LogLevel;

/*
 * Before using the logging API you must call log_category() at the beginning
 * of your file, like this:
 * log_category("display");
 * [...]
 * if (r < 0) {
 *     log_error("an error happenned"");
 * }
 */
#define log_category(cat) \
	static _unused_ const char *_log_category = cat

void log_internal(LogLevel level, const char *category, const char *format, ...) _printf_(3,4);

#ifndef NDEBUG
#define log_debug(msg, ...) \
	log_internal(LOG_DEBUG, _log_category, "[%s:%d %s()] " msg, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define log_info(msg, ...) \
	log_internal(LOG_INFO, _log_category, msg, ##__VA_ARGS__)
#else
#define log_debug(msg, ...)
#define log_info(msg, ...)
#endif

#define log_warning(msg, ...) \
	log_internal(LOG_WARNING, _log_category, msg, ##__VA_ARGS__)

#define log_error(msg, ...) \
	log_internal(LOG_ERROR, _log_category, msg, ##__VA_ARGS__)

__attribute__((noreturn)) void log_oom_and_exit(void);

#ifdef __cplusplus
}
#endif

