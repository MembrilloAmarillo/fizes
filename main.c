#include <windows.h>
#include <fileapi.h>
#include <minwinbase.h>
#include <handleapi.h>
#include <errhandlingapi.h>
#include <timezoneapi.h>

#include <stdio.h>
#include <stdlib.h>

inline DWORD get_file_size( DWORD size_high, DWORD size_low )
{
    DWORD file_size = ( size_high * (MAXWORD + 1) ) + size_low;
    file_size = ( file_size >> 20 ); // From B to MiB

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
    UINT64 minimum_file_size;
    LPCSTR file_name;
    WIN32_FIND_DATAA file_info;
    HANDLE file_found;
    BOOL next = 1;
    
    if( argc != 3 ) {
        fprintf( stderr, "Given %d values, expected 3", argc );
        exit( 1 );
    } else {
        fprintf( stdout, "Starting from path => %s\n", argv[1] );
        fprintf( stdout, "Minimum file size : %llu\n", atoll( argv[2] ) );
        fprintf( stdout, "******************************\n" );
        minimum_file_size = atoll( argv[2] );
        file_name = argv[1];
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