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

void event_update();

void call_mouse_motion(int mx, int my, int dx, int dy);
void call_mouse_press(int mx, int my, int button);
void call_mouse_release(int mx, int my, int button);
void call_key_press(const char* key_string);
void call_key_release(const char* key_string);
void call_key_text(const char* string);
void call_resize_event(int w, int h);
