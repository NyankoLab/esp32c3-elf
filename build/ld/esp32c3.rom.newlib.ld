/* ROM function interface esp32c3.rom.newlib.ld for esp32c3
 *
 *
 * Generated from ./interface-esp32c3.yml md5sum 93b28a9e1fe42d212018eb4336849208
 *
 * Compatible with ROM where ECO version equal or greater to 0.
 *
 * THIS FILE WAS AUTOMATICALLY GENERATED. DO NOT EDIT.
 */

/***************************************
 Group newlib
 ***************************************/

/* Functions */
PROVIDE ( esp_rom_newlib_init_common_mutexes = 0x40000350 );
PROVIDE ( memset = 0x40000354 );
PROVIDE ( memcpy = 0x40000358 );
PROVIDE ( memmove = 0x4000035c );
PROVIDE ( memcmp = 0x40000360 );
PROVIDE ( strcpy = 0x40000364 );
PROVIDE ( strncpy = 0x40000368 );
PROVIDE ( strcmp = 0x4000036c );
PROVIDE ( strncmp = 0x40000370 );
PROVIDE ( strlen = 0x40000374 );
PROVIDE ( strstr = 0x40000378 );
PROVIDE ( bzero = 0x4000037c );
PROVIDE ( sbrk = 0x40000384 );
PROVIDE ( isalnum = 0x40000388 );
PROVIDE ( isalpha = 0x4000038c );
PROVIDE ( isascii = 0x40000390 );
PROVIDE ( isblank = 0x40000394 );
PROVIDE ( iscntrl = 0x40000398 );
PROVIDE ( isdigit = 0x4000039c );
PROVIDE ( islower = 0x400003a0 );
PROVIDE ( isgraph = 0x400003a4 );
PROVIDE ( isprint = 0x400003a8 );
PROVIDE ( ispunct = 0x400003ac );
PROVIDE ( isspace = 0x400003b0 );
PROVIDE ( isupper = 0x400003b4 );
PROVIDE ( toupper = 0x400003b8 );
PROVIDE ( tolower = 0x400003bc );
PROVIDE ( toascii = 0x400003c0 );
PROVIDE ( memccpy = 0x400003c4 );
PROVIDE ( memchr = 0x400003c8 );
PROVIDE ( memrchr = 0x400003cc );
PROVIDE ( strcasecmp = 0x400003d0 );
PROVIDE ( strcasestr = 0x400003d4 );
PROVIDE ( strcat = 0x400003d8 );
PROVIDE ( strdup = 0x400003dc );
PROVIDE ( strchr = 0x400003e0 );
PROVIDE ( strcspn = 0x400003e4 );
PROVIDE ( strcoll = 0x400003e8 );
PROVIDE ( strlcat = 0x400003ec );
PROVIDE ( strlcpy = 0x400003f0 );
PROVIDE ( strlwr = 0x400003f4 );
PROVIDE ( strncasecmp = 0x400003f8 );
PROVIDE ( strncat = 0x400003fc );
PROVIDE ( strndup = 0x40000400 );
PROVIDE ( strnlen = 0x40000404 );
PROVIDE ( strrchr = 0x40000408 );
PROVIDE ( strsep = 0x4000040c );
PROVIDE ( strspn = 0x40000410 );
PROVIDE ( strtok_r = 0x40000414 );
PROVIDE ( strupr = 0x40000418 );
PROVIDE ( longjmp = 0x4000041c );
PROVIDE ( setjmp = 0x40000420 );
PROVIDE ( abs = 0x40000424 );
PROVIDE ( div = 0x40000428 );
PROVIDE ( labs = 0x4000042c );
PROVIDE ( ldiv = 0x40000430 );
PROVIDE ( qsort = 0x40000434 );
PROVIDE ( rand_r = 0x40000438 );
PROVIDE ( rand = 0x4000043c );
PROVIDE ( srand = 0x40000440 );
PROVIDE ( utoa = 0x40000444 );
PROVIDE ( itoa = 0x40000448 );
PROVIDE ( atoi = 0x4000044c );
PROVIDE ( atol = 0x40000450 );
PROVIDE ( strtol = 0x40000454 );
PROVIDE ( strtoul = 0x40000458 );
PROVIDE ( fflush = 0x4000045c );
PROVIDE ( _fflush_r = 0x40000460 );
PROVIDE ( _fwalk = 0x40000464 );
PROVIDE ( _fwalk_reent = 0x40000468 );
PROVIDE ( __swbuf_r = 0x40000474 );
PROVIDE ( __swbuf = 0x40000478 );
/* Data (.data, .bss, .rodata) */
PROVIDE ( syscall_table_ptr = 0x3fcdffe0 );
PROVIDE ( _global_impure_ptr = 0x3fcdffdc );
