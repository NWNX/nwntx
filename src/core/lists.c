/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2007 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "commonheaders.h"

/* a simple sorted list implementation */

SortedList* List_Create( int p_limit, int p_increment )
{
	SortedList* result = ( SortedList* )malloc( sizeof( SortedList ));
	if ( result == NULL )
		return(NULL);

	result->increment = p_increment;
	result->limit = p_limit;
	return(result);
}

void List_Destroy( SortedList* p_list )
{
	if ( p_list == NULL )
		return;

	if ( p_list->items != NULL ) {
		free( p_list->items );
		p_list->items = NULL;
	}

	p_list->realCount = p_list->limit = 0;
}

void* List_Find( SortedList* p_list, void* p_value )
{
	int index;
	
	if ( !List_GetIndex( p_list, p_value, &index ))
		return(NULL);

	return(p_list->items[ index ]);
}

int List_GetIndex( SortedList* p_list, void* p_value, int* p_index )
{
	if ( p_list->sortFunc != NULL )
	{
		int low  = 0;
		int high = p_list->realCount-1;

		while( low <= high )
		{  
			int i = ( low+high )/2;
			int result = p_list->sortFunc( p_list->items[ i ], p_value );
			if ( result == 0 )
			{	*p_index = i;
				return 1;
			}

			if ( result < 0 )
				low = i+1;
			else
				high = i-1;
		}

		*p_index = low;
	}
   return 0;
}

int List_IndexOf( SortedList* p_list, void* p_value )
{
	int i;
	for ( i=0; i < p_list->realCount; i++ )
		if ( p_list->items[i] == p_value )
			return i;

	return -1;
}

int List_Insert( SortedList* p_list, void* p_value, int p_index) 
{
	if ( p_value == NULL || p_index > p_list->realCount )
		return 0;

   if ( p_list->realCount == p_list->limit )
	{
		p_list->items = ( void** )realloc( p_list->items, sizeof( void* )*(p_list->realCount + p_list->increment));
		p_list->limit += p_list->increment;
	}

	if ( p_index < p_list->realCount )
		memmove( p_list->items+p_index+1, p_list->items+p_index, sizeof( void* )*( p_list->realCount-p_index ));

   p_list->realCount++;

   p_list->items[ p_index ] = p_value;
   return 1;
}

int List_InsertPtr( SortedList* list, void* p )
{
	int idx = list->realCount;
	List_GetIndex( list, p, &idx );
	return List_Insert( list, p, idx );
}

int List_Remove( SortedList* p_list, int index )
{
	if ( index < 0 || index > p_list->realCount )
		return(0);

   p_list->realCount--;
   if ( p_list->realCount > index )
	{
		memmove( p_list->items+index, p_list->items+index+1, sizeof( void* )*( p_list->realCount-index ));
      p_list->items[ p_list->realCount ] = NULL;
	}

   return 1;
}

int List_RemovePtr( SortedList* list, void* p )
{
	int idx = -1;
	if ( List_GetIndex( list, p, &idx ))
		List_Remove( list, idx );

	return idx;
}
