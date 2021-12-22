#include <windows.h>
#include <fileapi.h>
#include <minwinbase.h>
#include <handleapi.h>
#include <errhandlingapi.h>
#include <timezoneapi.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>

#define BUFF_SIZE MAX_PATH
#define MAGNITUD_OFFSET 0 // B as default

size_t my_strlen( char const* buffer )
{
  assert( buffer != NULL );
  size_t i = 0;

  do {
    i++;
  } while ( *(buffer + i) != '\0' );

  return i;
}


inline DWORD get_file_size( DWORD size_high, DWORD size_low )
{
    DWORD file_size = ( size_high * (MAXWORD + 1) ) + size_low;
    file_size = ( file_size >> MAGNITUD_OFFSET ); // From B to MiB

    return file_size;
}

void print_file_specification( HANDLE file_found, WIN32_FIND_DATAA file_info )
{
    DWORD file_size = get_file_size( file_info.nFileSizeHigh, file_info.nFileSizeLow );
    printf( "File Size: [ %lu ]\n",  file_size );
    printf( "cFileName: [ %s ]\n", file_info.cFileName );
    SYSTEMTIME time;
    if ( FileTimeToSystemTime( &file_info.ftCreationTime, &time ) ) {
        printf( "Creation time: [ Year %d, Month %d, Day %d ]\n", time.wYear, time.wMonth, time.wDay );
    } else {
        printf( "[ERROR] %ld\n", GetLastError() );
    }
}

int main( int argc, char *argv[] )
{
    UINT64           minimum_file_size;
    LPCSTR           file_name;
    WIN32_FIND_DATAA file_info;
    HANDLE           file_found;
    BOOL             next = 1;
    DWORD            directory_ret;
    char             buffer[ BUFF_SIZE ];
    LPSTR            path_name;
    
    if( argc != 3 ) {
        fprintf( stderr, "Given %d values, expected 3", argc );
        exit( 1 );
    } else {
        fprintf( stdout, "Selected path     => %s\n", argv[1] );
        fprintf( stdout, "Minimum file size : %llu\n", atoll( argv[2] ) );
        minimum_file_size = atoll( argv[2] );
        file_name = argv[1];
        path_name = (LPSTR)malloc( strlen( file_name ) * sizeof( CHAR ) );
        if( path_name == NULL ) {
            fprintf( stderr, "[ERROR] Malloc string not valid\n" );
            exit( 1 );
        }
        strcpy( path_name, file_name );
        // Change to valid directory
        if( file_name[ my_strlen(file_name) - 1 ] == '*' ) {
            path_name[ my_strlen( file_name ) - 1 ] = '\0';
        }
        fprintf( stdout, "Starting from path => %s\n", path_name );
    }

    directory_ret = GetCurrentDirectory( BUFF_SIZE, buffer );
    if( directory_ret == 0 ) {
        fprintf( stderr, "GetCurrentDirectory failed with code: [ %ld ]\n", GetLastError() );
        exit( 1 );
    } else if ( directory_ret > BUFF_SIZE ) {
        fprintf( stderr, "Buffer too small, need at least [ %ld ]\n", directory_ret );
        exit( 1 );
    } else {
        if( !SetCurrentDirectory( path_name ) ) {
            fprintf( stderr, "Directory not found '%s', error [ %ld ]\n", path_name, GetLastError() );
            exit( 1 );
        } else {
            fprintf( stdout, "Directory set in [ %s ]\n", path_name );
            fprintf( stdout, "******************************\n" );
        }
    }

    file_found = FindFirstFileA( file_name, &file_info );
    while( file_found != INVALID_HANDLE_VALUE && next ) {
        if( get_file_size( file_info.nFileSizeHigh, file_info.nFileSizeLow ) >= minimum_file_size ) {
            print_file_specification( file_found, file_info );
        }
        next = FindNextFileA( file_found, &file_info );
    }
    fprintf( stdout, "Search terminated...\nExiting" );
    FindClose( file_found );
    return 0;
}