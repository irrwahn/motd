/*
 * This file is part of the motd project.
 *
 * Copyright 2016 Urban Wallasch <irrwahn35@freenet.de>
 * See LICENSE file for more details.
 *
 */
/*
 * Motd - pick a random tagline from an indexed plain text library.
 *
 * Compile-time default configuration presets.
 *
 * Changelog:
 * 2016-05-22   Separated cache dir path from cache file paths.
 * 2016-05-22   Initial version.
 */

#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

/* Motto file paths: */
#define TXT_FILE    ".local/share/motd/motd.txt" /* Relative to $HOME. */
/* Fall-back to absolute system path. */
#define TXT_FILE2   "/usr/share/motd/motd.txt"

/* Index and PRNG state caches: */
#define CACHE_PATH	".cache/motd" /* Relative to $HOME. */
#define IDX_FILE    CACHE_PATH "/motd.idx"
#define RNG_FILE    CACHE_PATH "/motd.rng"

/* Motto delimiter used in motto text file: */
#define MOT_DELIM   L'÷'    /* Division sign, unicode codepoint 247. */

#endif //ndef CONFIG_H_INCLUDED

/* EOF */
