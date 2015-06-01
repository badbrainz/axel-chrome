  /********************************************************************\
  * Axel -- A lighter download accelerator for Linux and other Unices. *
  *                                                                    *
  * Copyright 2001 Wilmer van der Gaast                                *
  \********************************************************************/

/* Native messaging host main							*/

/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License with
  the Debian GNU/Linux distribution in file /usr/doc/copyright/GPL;
  if not, write to the Free Software Foundation, Inc., 59 Temple Place,
  Suite 330, Boston, MA  02111-1307  USA
*/

/* TODO*/
/* * fix bug in axel.c: all connections error leads to freeze*/

#include "axel.h"

static void stop( int signal );
static char *size_human( long long int value );
static char *time_human( int value );
static void print_alternate_output( axel_t *axel );
static void print_help();
static void print_version();
static void print_messages( axel_t *axel );
static int get_vargs( char **argv, int maxargs );

int run = 1;

#ifdef NOGETOPTLONG
#define getopt_long( a, b, c, d, e ) getopt( a, b, c )
#else
static struct option axel_options[] =
{
	/* name			has_arg	flag	val */
	{ "max-speed",		1,	NULL,	's' },
	{ "num-connections",	1,	NULL,	'n' },
	{ "output",		1,	NULL,	'o' },
	{ "search",		2,	NULL,	'S' },
	{ "no-proxy",		0,	NULL,	'N' },
	{ "quiet",		0,	NULL,	'q' },
	{ "verbose",		0,	NULL,	'v' },
	{ "help",		0,	NULL,	'h' },
	{ "version",		0,	NULL,	'V' },
	{ "alternate",		0,	NULL,	'a' },
	{ "header",		1,	NULL,	'H' },
	{ "user-agent",		1,	NULL,	'U' },
	{ NULL,			0,	NULL,	0 }
};
#endif

/* For returning string values from functions				*/
static char string[MAX_STRING];

static char message[MAX_MESSAGE];


int main( int argc, char *argv[] )
{
	char fn[MAX_STRING] = "";
	int do_search = 0;
	search_t *search;
	conf_t conf[1];
	axel_t *axel;
	int i, j, cur_head = 0, arg_c;
	char *s, **arg_v;
	double t;

	if( ( argc != 2 ) || (strcmp( argv[1], "chrome-extension://" ) < 0) )
	{
		return 1;
	}
	
#ifdef I18N
	setlocale( LC_ALL, "" );
	bindtextdomain( PACKAGE, LOCALE );
	textdomain( PACKAGE );
#endif
	
	if( !conf_init( conf ) )
	{
		return( 1 );
	}

	arg_v = malloc( 128 );
	arg_c = get_vargs( arg_v, 128 );
	if ( strcmp( arg_v[0], "axel" ) != 0 )
	{
		print_help();
		return( 1 );
	}
	
	opterr = 0;
	
	j = -1;
	while( 1 )
	{
		int option;
		
		option = getopt_long( arg_c, arg_v, "s:n:o:S::NqvhVaH:U:", axel_options, NULL );
		if( option == -1 )
			break;
		
		switch( option )
		{
		case 'U':
			strncpy( conf->user_agent, optarg, MAX_STRING);
			break;
		case 'H':
			strncpy( conf->add_header[cur_head++], optarg, MAX_STRING );
			break;
		case 's':
			if( !sscanf( optarg, "%i", &conf->max_speed ) )
			{
				print_help();
				return( 1 );
			}
			break;
		case 'n':
			if( !sscanf( optarg, "%i", &conf->num_connections ) )
			{
				print_help();
				return( 1 );
			}
			break;
		case 'o':
			strncpy( fn, optarg, MAX_STRING );
			break;
		case 'S':
			do_search = 1;
			if( optarg != NULL )
			if( !sscanf( optarg, "%i", &conf->search_top ) )
			{
				print_help();
				return( 1 );
			}
			break;
		case 'a':
			conf->alternate_output = 1;
			break;
		case 'N':
			*conf->http_proxy = 0;
			break;
		case 'h':
			print_help();
			return( 0 );
		case 'v':
			if( j == -1 )
				j = 1;
			else
				j ++;
			break;
		case 'V':
			print_version();
			return( 0 );
		case 'q':
			close( 1 );
			conf->verbose = -1;
			if( open( "/dev/null", O_WRONLY ) != 1 )
			{
				print_message( _("Can't redirect stdout to /dev/null.\n") );
				return( 1 );
			}
			break;
		default:
			print_help();
			return( 1 );
		}
	}
	conf->add_header_count = cur_head;
	if( j > -1 )
		conf->verbose = j;
	
	if( arg_c - optind == 0 )
	{
		print_help();
		return( 1 );
	}
	else if( strcmp( arg_v[optind], "-" ) == 0 )
	{
		s = malloc( MAX_STRING );
		if (scanf( "%1024[^\n]s", s) != 1) {
			print_message( _("Error when trying to read URL (Too long?).\n") );
			return( 1 );
		}
	}
	else
	{
		s = arg_v[optind];
		if( strlen( s ) > MAX_STRING )
		{
			print_message( _("Can't handle URLs of length over %d\n" ), MAX_STRING );
			return( 1 );
		}
	}
	
	print_message( _("Initializing download: %s\n"), s );
	if( do_search )
	{
		search = malloc( sizeof( search_t ) * ( conf->search_amount + 1 ) );
		memset( search, 0, sizeof( search_t ) * ( conf->search_amount + 1 ) );
		search[0].conf = conf;
		if( conf->verbose )
			print_message( _("Doing search...\n") );
		i = search_makelist( search, s );
		if( i < 0 )
		{
			print_message( _("File not found\n" ) );
			return( 1 );
		}
		if( conf->verbose )
			print_message( _("Testing speeds, this can take a while...\n") );
		j = search_getspeeds( search, i );
		search_sortlist( search, i );
		if( conf->verbose )
		{
			print_message( _("%i usable servers found, will use these URLs:\n"), j );
			j = min( j, conf->search_top );
			print_message( "%-60s %15s\n", "URL", "Speed" );
			for( i = 0; i < j; i ++ )
				print_message( "%-70.70s %5i\n", search[i].url, search[i].speed );
			print_message( "\n" );
		}
		axel = axel_new( conf, j, search );
		free( search );
		if( axel->ready == -1 )
		{
			print_messages( axel );
			axel_close( axel );
			return( 1 );
		}
	}
	else if( arg_c - optind == 1 )
	{
		axel = axel_new( conf, 0, s );
		if( axel->ready == -1 )
		{
			print_messages( axel );
			axel_close( axel );
			return( 1 );
		}
	}
	else
	{
		search = malloc( sizeof( search_t ) * ( arg_c - optind ) );
		memset( search, 0, sizeof( search_t ) * ( arg_c - optind ) );
		for( i = 0; i < ( arg_c - optind ); i++ )
			strncpy( search[i].url, arg_v[optind+i], MAX_STRING );
		axel = axel_new( conf, arg_c - optind, search );
		free( search );
		if( axel->ready == -1 )
		{
			print_messages( axel );
			axel_close( axel );
			return( 1 );
		}
	}
	print_messages( axel );
	if( s != arg_v[optind] )
	{
		free( s );
	}
	
	if( *fn )
	{
		struct stat buf;
		
		if( stat( fn, &buf ) == 0 )
		{
			if( S_ISDIR( buf.st_mode ) )
			{
				size_t fnlen = strlen(fn);
				size_t axelfnlen = strlen(axel->filename);
				
				if (fnlen + 1 + axelfnlen + 1 > MAX_STRING) {
					print_message( _("Filename too long!\n"));
					return ( 1 );
				}
				
				fn[fnlen] = '/';
				memcpy(fn+fnlen+1, axel->filename, axelfnlen);
				fn[fnlen + 1 + axelfnlen] = '\0';
			}
		}
		sprintf( string, "%s.st", fn );
		if( access( fn, F_OK ) == 0 ) if( access( string, F_OK ) != 0 )
		{
			i = 0;
			s = fn + strlen( fn );
			while( 1 )
			{
				sprintf( s, ".%i", i );
				sprintf( string, "%s.st", fn );
				if( access( fn, F_OK ) == 0 ) {
					if( access( string, F_OK ) == 0 )
						break;
				}
				else {
					if( access( string, F_OK ) != 0 )
						break;
				}
				i++;
			}
		}
		if( access( string, F_OK ) == 0 ) if( access( fn, F_OK ) != 0 )
		{
			print_message( _("State file found, but no downloaded data. Starting from scratch.\n" ) );
			unlink( string );
		}
		strcpy( axel->filename, fn );
	}
	else
	{
		/* Local file existence check					*/
		i = 0;
		s = axel->filename + strlen( axel->filename );
		while( 1 )
		{
			sprintf( string, "%s.st", axel->filename );
			if( access( axel->filename, F_OK ) == 0 )
			{
				if( axel->conn[0].supported )
				{
					if( access( string, F_OK ) == 0 )
						break;
				}
			}
			else
			{
				if( access( string, F_OK ) )
					break;
			}
			sprintf( s, ".%i", i );
			i ++;
		}
	}
	for ( i = 0; i < arg_c; i++ )
		free( arg_v[i] );
	free( arg_v );
	
	if( !axel_open( axel ) )
	{
		print_messages( axel );
		return( 1 );
	}
	print_messages( axel );
	axel_start( axel );
	print_messages( axel );

	axel->start_byte = axel->bytes_done;
	
	/* Install save_state signal handler for resuming support	*/
	signal( SIGINT, stop );
	signal( SIGTERM, stop );
	
	t = gettime() + 1;
	while( !axel->ready && run )
	{
		long long int prev;
		
		prev = axel->bytes_done;
		axel_do( axel );
		
		if( conf->alternate_output )
		{			
			if( !axel->message && prev != axel->bytes_done )
			if( gettime() > t )
			{
				print_alternate_output( axel );
				t = gettime() + 1;
			}
		}
		
		if( axel->message )
			print_messages( axel );
	}
	
	print_message( _("\nDownloaded %lld in %s. (%.2f KB/s)\n"),
		axel->bytes_done - axel->start_byte,
		time_human( gettime() - axel->start_time ),
		(double) axel->bytes_per_second / 1024 );
	
	i = axel->ready ? 0 : 2;
	
	axel_close( axel );
	
	return( i );
}

/* SIGINT/SIGTERM handler						*/
void stop( int signal )
{
	run = 0;
}

/* Convert a number of bytes to a human-readable form			*/
char *size_human( long long int value )
{
	if( value == 1 )
		sprintf( string, _("%lld byte"), value );
	else if( value < 1024 )
		sprintf( string, _("%lld bytes"), value );
	else if( value < 10485760 )
		sprintf( string, _("%.1f kilobytes"), (float) value / 1024 );
	else
		sprintf( string, _("%.1f megabytes"), (float) value / 1048576 );
	
	return( string );
}

/* Convert a number of seconds to a human-readable form			*/
char *time_human( int value )
{
	if( value == 1 )
		sprintf( string, _("%i second"), value );
	else if( value < 60 )
		sprintf( string, _("%i seconds"), value );
	else if( value < 3600 )
		sprintf( string, _("%i:%02i seconds"), value / 60, value % 60 );
	else
		sprintf( string, _("%i:%02i:%02i seconds"), value / 3600, ( value / 60 ) % 60, value % 60 );
	
	return( string );
}

static void print_alternate_output(axel_t *axel) 
{
	long long int done = axel->bytes_done;
	long long int total = axel->size;
	int i,j=0;
	double now = gettime();
	char *p;

	p = string;
	p += sprintf( p, "[%3ld%%] [", min(100,(long)(done*100./total+.5) ) );
	
	for(i=0;i<axel->conf->num_connections;i++)
	{
		for(;j<((double)axel->conn[i].currentbyte/(total+1)*50)-1;j++)
			*p++ = '.';

		if(axel->conn[i].currentbyte<axel->conn[i].lastbyte)
		{
			if(now <= axel->conn[i].last_transfer + axel->conf->connection_timeout/2 )
				*p++ = i+'0';
			else
				*p++ = '#';
		} else 
			*p++ = '.';

		j++;
		
		for(;j<((double)axel->conn[i].lastbyte/(total+1)*50);j++)
			*p++ = ' ';
	}

	if(axel->bytes_per_second > 1048576)
		p += sprintf( p, "] [%6.1fMB/s]", (double) axel->bytes_per_second / (1024*1024) );
	else if(axel->bytes_per_second > 1024)
		p += sprintf( p, "] [%6.1fKB/s]", (double) axel->bytes_per_second / 1024 );
	else
		p += sprintf( p, "] [%6.1fB/s]", (double) axel->bytes_per_second );

	if(done<total)
	{
		int seconds, minutes, hours, days;
		seconds = axel->finish_time - now;
		minutes=seconds/60;seconds-=minutes*60;
		hours=minutes/60;minutes-=hours*60;
		days=hours/24;hours-=days*24;
		if(days)
		    sprintf( p, " [%2dd%2d]",days,hours);
		else if(hours)
		    sprintf( p, " [%2dh%02d]",hours,minutes);
		else
		    sprintf( p, " [%02d:%02d]",minutes,seconds);
	}

	print_message( string );
}

void print_help()
{
#ifdef NOGETOPTLONG
	print_message( _("Usage: axel [options] url1 [url2] [url...]\n"
		"\n"
		"-s x\tSpecify maximum speed (bytes per second)\n"
		"-n x\tSpecify maximum number of connections\n"
		"-o f\tSpecify local output file\n"
		"-S [x]\tSearch for mirrors and download from x servers\n"
		"-H x\tAdd header string\n"
		"-U x\tSet user agent\n"
		"-N\tJust don't use any proxy server\n"
		"-q\tLeave stdout alone\n"
		"-v\tMore status information\n"
		"-a\tAlternate progress indicator\n"
		"-h\tThis information\n"
		"-V\tVersion information\n"
		"\n"
		"Visit http://axel.alioth.debian.org/ to report bugs\n") );
#else
	print_message( _("Usage: axel [options] url1 [url2] [url...]\n"
		"\n"
		"--max-speed=x\t\t-s x\tSpecify maximum speed (bytes per second)\n"
		"--num-connections=x\t-n x\tSpecify maximum number of connections\n"
		"--output=f\t\t-o f\tSpecify local output file\n"
		"--search[=x]\t\t-S [x]\tSearch for mirrors and download from x servers\n"
		"--header=x\t\t-H x\tAdd header string\n"
		"--user-agent=x\t\t-U x\tSet user agent\n"
		"--no-proxy\t\t-N\tJust don't use any proxy server\n"
		"--quiet\t\t\t-q\tLeave stdout alone\n"
		"--verbose\t\t-v\tMore status information\n"
		"--alternate\t\t-a\tAlternate progress indicator\n"
		"--help\t\t\t-h\tThis information\n"
		"--version\t\t-V\tVersion information\n"
		"\n"
		"Visit http://axel.alioth.debian.org/ to report bugs\n") );
#endif
}

void print_version()
{
	print_message( _("Axel version %s (%s)\n"), AXEL_VERSION_STRING, ARCH );
	print_message( "\nCopyright 2001-2002 Wilmer van der Gaast.\n" );
}

/* Print any message in the axel structure				*/
void print_messages( axel_t *axel )
{
	message_t *m;
	
	while( axel->message )
	{
		print_message( "%s\n", axel->message->text );
		m = axel->message;
		axel->message = axel->message->next;
		free( m );
	}
}

int get_vargs( char **argv, int maxargs )
{
	int msglen, argc, esc, qou;
	char *arg, *a, *p, *q;

	argc = 0;

	fread( &msglen, 4, 1, stdin );
	if( msglen < MAX_MESSAGE )
	{
		fread( message, msglen, 1, stdin );
		p = message + 1;
		q = message + msglen - 1;

		arg = malloc( msglen );
		esc = 0;
		qou = 0;
		while ( p != q )
		{
			if ( *p == ' ' )
			{
				p++;
				continue;
			}
			a = arg;
			while( p != q )
			{
				if( *p == ' ' )
				{
					if( !esc && !qou )
						break;
				}
				if( esc )
				{
					esc = 0;
					if( qou || ( *p == '"' ) )
					{
						if( *p == '"' )
						{
							qou = !qou;
							p++;
							continue;
						}
					}
				}
				else if( *p == '\\' )
				{
					esc = 1;
					p++;
					continue;
				}
				*a++ = *p++;
			}
			*a = '\0';

			if( argc < maxargs - 1 )
			{
				if( !esc && !qou )
				{
					argv[argc] = malloc( strlen( arg ) + 1 );
					strcpy( argv[argc], arg );
					argc++;
				}
			}
		}
		free( arg );
	}
	argv[argc] = NULL;

	return argc;
}

int print_message( char *format, ... )
{
	int msglen;
	char *c, *m;
	va_list params;

	c = message;
	m = message + MAX_MESSAGE / 2;

	va_start( params, format );
	vsnprintf( m, MAX_MESSAGE / 2, format, params );
	va_end( params );

	*c++ = '"';
	while( *m != '\0' )
	{
		switch( *m )
		{
		case '\n':
			*c++ = '\\';
			*c++ = 'n';
			break;
		case '\r':
			*c++ = '\\';
			*c++ = 'r';
			break;
		case '\t':
			*c++ = '\\';
			*c++ = 't';
			break;
		default:
			*c++ = *m;
			break;
		}
		m++;
	}
	*c++ = '"';
	*c = '\0';

	msglen = strlen( message );
	fwrite( &msglen, 4, 1, stdout );
	fwrite( message, msglen, 1, stdout );
	fflush( stdout );

	return( msglen );
}
