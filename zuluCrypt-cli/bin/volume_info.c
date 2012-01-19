/*
 * 
 *  Copyright (c) 2011
 *  name : mhogo mchungu 
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "includes.h"

int volume_info( const char * mapper,const char * device )
{
	char * output ;	
	int xt ;
	
	string_t p ;
	
	p = String( mapper ) ;		
	
	replace_bash_special_chars( &p ) ;
	
	StringPrepend( p,"/dev/mapper/zuluCrypt-" ) ;
	
	if( is_path_valid( StringContent( p ) ) == -1 ) {
		printf( "\"%s\" is inactive\n",device ) ;
		xt =  1 ;
	}else{
		output = status( StringContent( p ) ) ;
		printf( "%s\n",output );
		free( output ) ;
		xt = 0 ;
	}
	StringDelete( p );
	return xt ;
}

