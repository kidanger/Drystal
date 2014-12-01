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

#include <cstdio>

#include "macro.h"

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

#define log_message(level, msg, ...) \
	fprintf(stderr, "[%4s|%10s] " msg "\n", level, _log_category, ##__VA_ARGS__)

#ifdef DODEBUG
#define log_debug(msg, ...) \
	log_message("DBG", "[%s:%d %s()] " msg, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define log_info(msg, ...) \
	log_message("INFO", msg, ##__VA_ARGS__)
#else
#define log_debug(msg, ...)
#define log_info(msg, ...)
#endif

#define log_warning(msg, ...) \
	log_message("WARN", msg, ##__VA_ARGS__)

#define log_error(msg, ...) \
	log_message("ERR", msg, ##__VA_ARGS__)

