/*
 * Motd - pick a random tagline from an indexed plain text library.
 * 
 * Compile-time default configuration presets.
 * 
 * Changelog:
 * 2016-05-22   Initial version.
 */

#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

/* Motto file path; second one is used as fallback: */
#define TXT_FILE    ".local/share/motd/motd.txt" /* Relative to $HOME. */
#define TXT_FILE2   "/usr/share/motd/motd.txt"   /* Absolute path. */

/* Index and PRNG state cache file paths: */
#define IDX_FILE    ".cache/motd/motd.idx" /* Relative to $HOME. */
#define RNG_FILE    ".cache/motd/motd.rng" /* Relative to $HOME. */

/* Motto delimiter used in motto text file: */
#define MOT_DELIM   L'÷'    /* Division sign, unicode codepoint 247.*/

#endif //ndef CONFIG_H_INCLUDED

/* EOF */
