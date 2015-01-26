/*
 * Copyright (C) 2012 Sistemas Operativos - UTN FRBA. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NODE_H_
#define NODE_H_


	struct link_element{
		void *data;
		struct link_element *next;
	};
	typedef struct link_element t_link_element;

	struct double_link_element{
		struct double_link_element *previous;
		void *data;
		struct double_link_element *next;
	};
	typedef struct double_link_element t_double_link_element;

	struct hash_element{
		char *key;
		unsigned int hashcode;
		void *data;
		struct hash_element *next;
	};
	typedef struct hash_element t_hash_element;

#endif /*NODE_H_*/

/*
 * Copyright (C) 2012 Sistemas Operativos - UTN FRBA. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

	#define DEFAULT_DICTIONARY_INITIAL_SIZE 20

	#include <stdbool.h>

	typedef struct {
		t_hash_element **elements;
		int table_max_size;
		int table_current_size;
		int elements_amount;
	} t_dictionary;

	t_dictionary *dictionary_create();
	void 		  dictionary_put(t_dictionary *, char *key, void *data);
	void 		 *dictionary_get(t_dictionary *, char *key);
	void 		 *dictionary_remove(t_dictionary *, char *key);
	void 		  dictionary_remove_and_destroy(t_dictionary *, char *, void(*data_destroyer)(void*));
	void 		  dictionary_iterator(t_dictionary *, void(*closure)(char*,void*));
	void 		  dictionary_clean(t_dictionary *);
	void 		  dictionary_clean_and_destroy_elements(t_dictionary *, void(*data_destroyer)(void*));
	bool 		  dictionary_has_key(t_dictionary *, char* key);
	bool 		  dictionary_is_empty(t_dictionary *);
	int 		  dictionary_size(t_dictionary *);
	void 		  dictionary_destroy(t_dictionary *);
	void 		  dictionary_destroy_and_destroy_elements(t_dictionary *, void(*data_destroyer)(void*));

#endif /* DICTIONARY_H_ */

/*
 * Copyright (C) 2012 Sistemas Operativos - UTN FRBA. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIST_H_
#define LIST_H_

	#include <stdbool.h>

	typedef struct {
		t_link_element *head;
		int elements_count;
	} t_list;

	t_list * list_create();

	void list_destroy(t_list *);
	void list_destroy_and_destroy_elements(t_list*, void(*element_destroyer)(void*));

	int list_add(t_list *, void *element);
	void list_add_in_index(t_list *, int index, void *element);
	void list_add_all(t_list*, t_list* other);

	void *list_get(t_list *, int index);

	t_list* list_take(t_list*, int count);
	t_list* list_take_and_remove(t_list*, int count);

	t_list* list_filter(t_list*, bool(*condition)(void*));
	t_list* list_map(t_list*, void*(*transformer)(void*));

	void *list_replace(t_list*, int index, void* element);
	void list_replace_and_destroy_element(t_list*, int index, void* element, void(*element_destroyer)(void*));

	void *list_remove(t_list *, int index);
	void list_remove_and_destroy_element(t_list *, int index, void(*element_destroyer)(void*));

	void *list_remove_by_condition(t_list *, bool(*condition)(void*));
	void list_remove_and_destroy_by_condition(t_list *, bool(*condition)(void*), void(*element_destroyer)(void*));

	void list_clean(t_list *);
	void list_clean_and_destroy_elements(t_list *self, void(*element_destroyer)(void*));

	void list_iterate(t_list *, void(*closure)(void*));
	void *list_find(t_list *, bool(*closure)(void*));

	int list_size(t_list *);
	int list_is_empty(t_list *);

	void list_sort(t_list *, bool (*comparator)(void *, void *));
	
	int list_count_satisfying(t_list* self, bool(*condition)(void*));
	bool list_any_satisfy(t_list* self, bool(*condition)(void*));
	bool list_all_satisfy(t_list* self, bool(*condition)(void*));

#endif /*LIST_H_*/

/*
 * Copyright (C) 2012 Sistemas Operativos - UTN FRBA. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QUEUE_H_
#define QUEUE_H_


	typedef struct {
		t_list* elements;
	} t_queue;

	t_queue *queue_create();
	void queue_destroy(t_queue *);
	void queue_destroy_and_destroy_elements(t_queue*, void(*element_destroyer)(void*));

	void queue_push(t_queue *, void *element);
	void *queue_pop(t_queue *);
	void *queue_peek(t_queue *);
	void queue_clean(t_queue *);
	void queue_clean_and_destroy_elements(t_queue *, void(*element_destroyer)(void*));

	int queue_size(t_queue *);
	int queue_is_empty(t_queue *);

#endif /*QUEUE_H_*/

/*
 * Copyright (C) 2012 Sistemas Operativos - UTN FRBA. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BITARRAY_H_
#define BITARRAY_H_

	#include <stdbool.h>
	#include <limits.h>
	#include <unistd.h>

	/* position of bit within character */
	#define BIT_CHAR(bit)         ((bit) / CHAR_BIT)

	/* array index for character containing bit */
	#define BIT_IN_CHAR(bit)      (0x80 >> (CHAR_BIT - 1 - ((bit)  % CHAR_BIT)))


	typedef struct {
		char *bitarray;
		size_t size;
	} t_bitarray;

	t_bitarray 	*bitarray_create(char *bitarray, size_t size);
	bool 		 bitarray_test_bit(t_bitarray*, off_t bit_index);
	void		 bitarray_set_bit(t_bitarray*, off_t bit_index);
	void		 bitarray_clean_bit(t_bitarray*, off_t bit_index);
	size_t		 bitarray_get_max_bit(t_bitarray*);
	void 		 bitarray_destroy(t_bitarray*);

#endif /* BITARRAY_H_ */
/*
 * Copyright (C) 2012 Sistemas Operativos - UTN FRBA. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

	typedef struct {
		char *path;
		t_dictionary *properties;
	} t_config;

	t_config *config_create(char *path);
	bool 	  config_has_property(t_config*, char* key);
	char 	 *config_get_string_value(t_config*, char *key);
	int 	  config_get_int_value(t_config*, char *key);
	long	  config_get_long_value(t_config*, char *key);
	double 	  config_get_double_value(t_config*, char *key);
	char**    config_get_array_value(t_config*, char* key);
	int 	  config_keys_amount(t_config*);
	void 	  config_destroy(t_config *config);

#endif /* CONFIG_H_ */
/*
 * Copyright (C) 2012 Sistemas Operativos - UTN FRBA. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef ERROR_H_
#define ERROR_H_

	/*
	 * @NAME: error_show
	 * @DESC: Muestra un mensaje de error por stdout
	 */
	void error_show(char *message, ...);

#endif /* ERROR_H_ */
/*
 * Copyright (C) 2012 Sistemas Operativos - UTN FRBA. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LOG_H_
#define LOG_H_

	#include <stdio.h>
	#include <stdbool.h>
	#include <sys/types.h>


	typedef enum {
		LOG_LEVEL_TRACE,
		LOG_LEVEL_DEBUG,
		LOG_LEVEL_INFO,
		LOG_LEVEL_WARNING,
		LOG_LEVEL_ERROR
	}t_log_level;

	typedef struct {
		FILE* file;
		bool is_active_console;
		t_log_level detail;
		char *program_name;
		pid_t pid;
	}t_log;


	t_log* 		log_create(char* file, char *program_name, bool is_active_console, t_log_level level);
	void 		log_destroy(t_log* logger);

	void 		log_trace(t_log* logger, const char* message, ...);
	void 		log_debug(t_log* logger, const char* message, ...);
	void 		log_info(t_log* logger, const char* message, ...);
	void 		log_warning(t_log* logger, const char* message, ...);
	void 		log_error(t_log* logger, const char* message, ...);

	char 		*log_level_as_string(t_log_level level);
	t_log_level log_level_from_string(char *level);

#endif /* LOG_H_ */
/*
 * Copyright (C) 2012 Sistemas Operativos - UTN FRBA. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PROCESS_H_
#define PROCESS_H_

unsigned int process_get_thread_id();
unsigned int process_getpid();


#endif /* PROCESS_H_ */
/*
 * Copyright (C) 2012 Sistemas Operativos - UTN FRBA. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef STRING_UTILS_H_
#define STRING_UTILS_H_

	#include <stdbool.h>
	#include <stdarg.h>

	char*   string_new();
        char*   string_itoa(int number);
	char*   string_from_format(const char* format, ...);
        char*   string_from_vformat(const char* format, va_list arguments);
	char*   string_repeat(char ch, int count);
	void 	string_append(char ** original, char * string_to_add);
	void    string_append_with_format(char **original, const char *format, ...);
	char*	string_duplicate(char* original);

	void 	string_to_upper(char * text);
	void 	string_to_lower(char * text);
	void 	string_capitalized(char * text);
	void 	string_trim(char ** text);
	void 	string_trim_left(char ** text);
	void 	string_trim_right(char ** text);

	int 	string_length(char * text);
	bool 	string_is_empty(char * text);
	bool 	string_starts_with(char * text, char * begin);
	bool	string_ends_with(char* text, char* end);
	bool 	string_equals_ignore_case(char * actual, char * expected);
	char**  string_split(char * text, char * separator);
	char*   string_substring(char* text, int start, int length);
	char*   string_substring_from(char *text, int start);
	char*   string_substring_until(char *text, int length);


	void 	string_iterate_lines(char ** strings, void (*closure)(char *));
	char**  string_get_string_as_array(char* text);

#endif /* STRING_UTILS_H_ */
/*
 * Copyright (C) 2012 Sistemas Operativos - UTN FRBA. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef TEMPORAL_H_
#define TEMPORAL_H_

	char *temporal_get_string_time();

#endif /* TEMPORAL_H_ */
/*
 * Copyright (C) 2012 Sistemas Operativos - UTN FRBA. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TXT_H_
#define TXT_H_

#include <stdio.h>

FILE* txt_open_for_append(char* path);
void txt_write_in_file(FILE* file, char* string);
void txt_write_in_stdout(char* string);
void txt_close_file(FILE* file);

#endif /* TXT_H_ */

