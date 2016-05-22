/*
 * Motd - pick a random tagline from an indexed plain text library.
 * 
 * This is basically an example for how far things can get out  
 * of hand, once you start using defensive, robust coding while 
 * rewriting a trivial tool.  Use at own risk!
 * 
 * Changelog:
 * 2016-05-22   Auto-create cache dir; added clear cache option; bugfixes.
 * 2016-05-22   Added prng state caching; changed default paths.
 * 2016-02-05   Added UTF-8 support, changed default separator.
 * 2016-02-04   Added a more decent PRNG.
 * 2016-02-03   Rewrote large portions of code w/ several improvements.
 * 2010-ish     Ported to Linux.
 * 1990-ish     Original DOS version.
 */

#include <linux/limits.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <locale.h>
#include <wchar.h>

#include "prng.h"
#include "config.h"


static random_ctx_t rng_ctx = RANDOM_CTX_INITIALIZER;

static void print_usage( const char *argv0 )
{
    const char *p = ( NULL == ( p = strrchr( argv0, '/' ) ) ) ? argv0 : p+1;
    fprintf( stderr, 
        "%s: Randomly pick motto of the day from an indexed plain text file.\n"
        "Usage: %s [-h] [-r] [-d c] [-D n] [-t mottofile] [-i indexfile] [-s rngfile]\n"
        "  -d : Set record delimiter used in mottofile to single character c;\n"
        "       default: '÷' (division sign, codepoint 247).\n"
        "  -D : Same as -d, except set delimiter to codepoint number n.\n"
        "  -t : Motto text file; default: $HOME/%s;\n"
        "       fall-back system path: %s.\n"
        "  -i : Motto index file; default: $HOME/%s.\n"
        "  -s : PRNG state cache file; default: $HOME/%s.\n"
        "  -r : Force index file to be rebuilt.\n"
        "       Note: Missing or outdated index files are automatically (re)generated.\n"
        "  -c : Remove index and PRNG cache dir and exit.\n"
        "  -h : Display this help text and exit.\n"
        , 
        p, 
        p,
        TXT_FILE, TXT_FILE2,
        IDX_FILE,
        RNG_FILE
    );
}

static void err_exit( const char *fmt, ... )
{
    int en = errno;
    va_list arglist;

    va_start( arglist, fmt );
    fprintf( stderr, "motd: " );
    vfprintf( stderr, fmt, arglist );
    if ( en )
        fprintf( stderr, ": %s\n", strerror( en ) );
    else
        fputc( '\n', stderr );
    va_end( arglist );
    exit( EXIT_FAILURE );   
}

static long create_idx( FILE *txt_fp, const char *idx_path, const wint_t delim )
{
    wint_t wc = !WEOF;
    long motto_cnt = 0;
    struct stat idx_stat;
    FILE *idx_fp = NULL;

    /* Only attempt to write to somewhat sane locations.  Ignore 
     * errors, the index file is not essential - we still want the 
     * side effect of counting mottos!
     */
    if ( 0 != stat( idx_path, &idx_stat ) || S_ISREG( idx_stat.st_mode ) )
        idx_fp = fopen( idx_path, "w" );
    
    rewind( txt_fp );
    while ( WEOF != wc )
    {
        long off;
        off = ftell( txt_fp );
        while ( ( wc = fgetwc( txt_fp ) ) != WEOF && delim != wc )
            ;
        /* Note: We Increment at least once, even for empty files.
         * Accordingly the index will always have at least one entry.
         */
        ++motto_cnt;    
        if ( NULL != idx_fp )
            fwrite( &off, sizeof off, 1, idx_fp );
    }
    if ( NULL != idx_fp )
        fclose( idx_fp );
    return motto_cnt;
}

static void motd( const char *txt_path, const char *idx_path, wint_t delim, const int regen_idx )
{
    wint_t wc;
    long off = -1;
    long recno = -1;
    long motto_cnt = -1;
    long index_cnt = -1;
    FILE *txt_fp;
    FILE *idx_fp;
    struct stat txt_stat;
    struct stat idx_stat;

    /* Assert motto text file exists and is readable. */
    if (   0 != stat( txt_path, &txt_stat )
        || !S_ISREG( txt_stat.st_mode )
        || NULL == ( txt_fp = fopen( txt_path, "r" ) ) )
    {
        err_exit( "Opening %s for reading", txt_path );
    }
    
    /* Regenerate index, if indicated. */
    if (   regen_idx 
        || 0 != stat( idx_path, &idx_stat )
        || !S_ISREG( idx_stat.st_mode )
        || idx_stat.st_mtime < txt_stat.st_mtime )
    {
        motto_cnt = create_idx( txt_fp, idx_path, delim );
    }

    /* Try to read one offset entry picked at random from index file. */
    if (   0 == stat( idx_path, &idx_stat )
        && S_ISREG( idx_stat.st_mode )
        && idx_stat.st_mtime >= txt_stat.st_mtime
        && NULL != ( idx_fp = fopen( idx_path, "r" ) ) )
    {
        if (   0 == fseek( idx_fp, 0, SEEK_END )
            && 0 < ( index_cnt = ftell( idx_fp ) / sizeof off ) )
        {
            recno = random_uni_r( &rng_ctx, index_cnt );
            if (   0 != fseek( idx_fp, recno * sizeof off, SEEK_SET )
                || 1 != fread( &off, sizeof off, 1, idx_fp ) )
            {
                /* Note: Do NOT reset index_cnt, we'll need it below! */
                off = -1;
                recno = -1;
            }
        }
        fclose( idx_fp );
    }
    
    /*
     * We may, or may not, have called create_idx, which may, or may 
     * not, have created an index file, which we then have attempted 
     * to read from (or not), which may have been successful. Or not.  
     * Plus, we may need motto_cnt later in the fall-back code, offset 
     * validity notwithstanding.
     * 
     * What a mess; do some validation and assert a sane motto count!
     */
    errno = 0;
    if ( 0 >= motto_cnt )
    {
        if ( 0 >= index_cnt )
            err_exit( "No motto candidate eligible; check your text and index files and try -r!" );
        motto_cnt = index_cnt;
        /* This is the normal path of execution with a proper index 
         * file. Still, a fabricated index file might not match the 
         * text file, leading to unexpected results. No easy way to 
         * check, though. 
         */
    }
    else
    {
        /* If both counts are valid, they'd better be equal.
         * Otherwise something weird has happened to the index file. 
         */
        if ( 0 <= index_cnt && motto_cnt != index_cnt )
            err_exit( "Motto index corrupt; check your index file and try -r!" );
    }

    /* Wind text file to motto offset. */
    if ( 0 > off || 0 != fseek( txt_fp, off, SEEK_SET ) )
    {
        /* Inline naive and slow fall-back implementation. */
        if ( 0 > recno )
            recno = random_uni_r( &rng_ctx, motto_cnt );
        rewind( txt_fp );
        for ( long r = recno; r--; )
        {
            while ( ( wc = fgetwc( txt_fp ) ) != WEOF && delim != wc )
                ;
            /* Wrap around EOF, just in case. */
            if ( WEOF == wc )
                rewind( txt_fp );
        }
        /* Not used below, just in case. */
        off = ftell( txt_fp );
    }

    /* Write motto to stdout; skip one leading line ending, if present. */
    wc = fgetwc( txt_fp );
    if ( WEOF != wc && delim != wc )
    {
        if ( '\r' == wc )
            wc = fgetwc( txt_fp );
        if ( WEOF != wc && delim != wc && '\n' != wc )
            putwchar( wc );
    }
    while ( ( wc = fgetwc( txt_fp ) ) != WEOF && delim != wc )
        putwchar( wc );
        
    fclose( txt_fp );
}

int main( int argc, char *argv[] )
{
    int opt;
    const char *optstr = ":d:D:i:s:t:chr";
    char txt_path[PATH_MAX] = "";
    char idx_path[PATH_MAX] = "";
    char rng_path[PATH_MAX] = "";
    char cache_dir[PATH_MAX] = "";
    char *home_dir = ".";
    wchar_t delim = MOT_DELIM;
    int regen_idx = 0;
    int clear_cache = 0;
    char *p;
    int n;
    FILE *rng_fp;
    

    setlocale( LC_CTYPE, "" );
    opterr = 0;
    errno = 0;
    while ( -1 != ( opt = getopt( argc, argv, optstr ) ) )
    {
        switch ( opt )
        {
        case 'c':
            clear_cache = 1;
            break;
        case 'd':
            if ( 0 >= mbtowc( &delim, optarg, strlen( optarg ) ) )
            {
                print_usage( argv[0] );
                err_exit( "Invalid multibyte character argument for option '-%c %s'.", opt, optarg );
            }
            break;
        case 'D':
            delim = strtoul( optarg, &p, 0 );
            if ( *p )
            {
                print_usage( argv[0] );
                err_exit( "Invalid numerical argument for option '-%c %s'.", opt, optarg );
            }
            break;
        case 'i':
            n = snprintf( idx_path, sizeof idx_path, "%s", optarg );
            if ( 0 > n )
                err_exit( "Encoding error in snprintf." );
            if ( (int)sizeof idx_path <= n )
                err_exit( "Index file name length exceeds %d.", sizeof idx_path );
            break;
        case 's':
            n = snprintf( rng_path, sizeof rng_path, "%s", optarg );
            if ( 0 > n )
                err_exit( "Encoding error in snprintf." );
            if ( (int)sizeof rng_path <= n )
                err_exit( "PRNG state file name length exceeds %d.", sizeof rng_path );
            break;
        case 't':
            n = snprintf( txt_path, sizeof txt_path, "%s", optarg );
            if ( 0 > n )
                err_exit( "Encoding error in snprintf." );
            if ( (int)sizeof txt_path <= n )
                err_exit( "Motto file name length exceeds %d.", sizeof txt_path );
            break;
        case 'r':
            regen_idx = 1;
            break;
        case '?':
            print_usage( argv[0] );
            err_exit( "Unrecognized option '-%c'.", optopt );
            break;
        case ':':
            print_usage( argv[0] );
            err_exit( "Missing argument for option '-%c'.", optopt );
            break;
        case 'h':
            print_usage( argv[0] );
            exit( EXIT_SUCCESS );
        default:
            print_usage( argv[0] );
            exit( EXIT_FAILURE );
            break;
        }
    }
    if ( optind < argc )
    {
        print_usage( argv[0] );
        err_exit( "Excess non-option command line argument '%s'.", argv[optind] );
    }
    
    if ( NULL == ( home_dir = getenv( "HOME" ) ) )
        home_dir = ".";
    
    n = snprintf( cache_dir, sizeof cache_dir, "%s/%s", home_dir, CACHE_PATH );
    if ( 0 > n )
        err_exit( "Encoding error in snprintf." );
    if ( (int)sizeof cache_dir <= n )
        err_exit( "Cache directory name length exceeds %d.", sizeof cache_dir );
    if ( clear_cache )
    {
        unlink( idx_path );
        unlink( rng_path );
        if ( 0 != rmdir( cache_dir ) )
			err_exit( "Could not remove cache directory %s.", cache_dir );
        exit( EXIT_SUCCESS );
    }
    /* Errors creating cache dir will effectively caught by subsequent file operations! */
    mkdir( cache_dir, 0700 );    
    if ( '\0' == *idx_path )
    {
        n = snprintf( idx_path, sizeof idx_path, "%s/%s", home_dir, IDX_FILE );
        if ( 0 > n )
            err_exit( "Encoding error in snprintf." );
        if ( (int)sizeof idx_path <= n )
            err_exit( "Index file name length exceeds %d.", sizeof idx_path );
    }
    if ( '\0' == *rng_path )
    {
        n = snprintf( rng_path, sizeof rng_path, "%s/%s", home_dir, RNG_FILE );
        if ( 0 > n )
            err_exit( "Encoding error in snprintf." );
        if ( (int)sizeof rng_path <= n )
            err_exit( "PRNG state file name length exceeds %d.", sizeof rng_path );
    }
    /* Initialize PRNG state from file; use time + pid for failsafe. */
    if ( NULL != ( rng_fp = fopen( rng_path, "r" ) ) )
    {
        fread( &rng_ctx, sizeof rng_ctx, 1, rng_fp );
        fclose( rng_fp );
    }
    else
        srandom_r( &rng_ctx, time( NULL ) + clock() + getpid() );
    
    if ( '\0' == *txt_path )
    {
        n = snprintf( txt_path, sizeof txt_path, "%s/%s", home_dir, TXT_FILE );
        if ( 0 > n )
            err_exit( "Encoding error in snprintf." );
        if ( (int)sizeof txt_path <= n )
            err_exit( "Motto file name length exceeds %d.", sizeof txt_path );
        if ( 0 != access( txt_path, R_OK ) )
        {   /* Fall back to alternative absolute system path. */
            n = snprintf( txt_path, sizeof txt_path, "%s", TXT_FILE2 );
            if ( 0 > n )
                err_exit( "Encoding error in snprintf." );
            if ( (int)sizeof txt_path <= n )
                err_exit( "Motto file name length exceeds %d.", sizeof txt_path );
        }
    }
    
    errno = 0;
    motd( txt_path, idx_path, delim, regen_idx );

    /* Save PRNG state to file. */
    if ( NULL == ( rng_fp = fopen( rng_path, "w" ) )
		|| 1 != fwrite( &rng_ctx, sizeof rng_ctx, 1, rng_fp ) )
	{
		err_exit( "Could not write PRNG state to %s.", rng_path );
    }
    fclose( rng_fp );

    exit( EXIT_SUCCESS );
}

/* EOF */
