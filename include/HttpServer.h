#pragma once

void http_register_handlers();
void http_start();
void http_ws_text_all(const char *msg);
void http_ws_printf(const char *format, ...) __attribute__((format(printf, 1, 2)));