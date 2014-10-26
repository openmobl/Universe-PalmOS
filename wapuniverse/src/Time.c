/********************************************************************************
 * Universe Web Browser                                                         *
 * Copyright (c) 2007 OpenMobl Systems                                          *
 * Copyright (c) 2006-2007 Donald C. Kirker                                     *
 * Portions Copyright (c) 1999-2007 Filip Onkelinx                              *
 *                                                                              *
 * http://www.openmobl.com/                                                     *
 * dev-support@openmobl.com                                                     *
 *                                                                              *
 * This program is free software; you can redistribute it and/or                *
 * modify it under the terms of the GNU General Public License                  *
 * as published by the Free Software Foundation; either version 2               *
 * of the License, or (at your option) any later version.                       *
 *                                                                              *
 * This program is distributed in the hope that it will be useful,              *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of               *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                 *
 * GNU General Public License for more details.                                 *
 *                                                                              *
 * You should have received a copy of the GNU General Public License            *
 * along with this program; if not, write to the Free Software                  *
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. 	*
 *                                                                              *
 ********************************************************************************/
#include <PalmOS.h>
#include "PalmUtils.h"

#define MAX_FRIENDLY	80

static Char *MonthString[] = {
	"january",
	"february",
	"march",
	"april",
	"may",
	"june",
	"july",
	"august",
	"september",
	"october",
	"november",
	"december"
};

static Char* TimeGetInteger( Char* s, Int16* xP, UInt16 digits )
{
     *xP = 0;

     while ( digits-- ) {
         if ( *s < '0' || '9' < *s )
             return NULL;

         *xP = *xP * 10 + *s - '0';
         s++;
     }

     return s;
}

static Boolean TimeIsWhiteSpace( Char c )
{
	return ( (c == ' ') || (c == '\t') ||
		 (c == '\r') || (c == '\n') );
}

static Char *TimeGetFragment( Char* s )
{
	while ( (*s < '0') || (*s > '9') )
			s++;

	return s;
}

static Int16 TimeStringToMonth( Char* s )
{
	Int16	i = 0;

	while ( StrNCaselessCompare(s, MonthString[i], StrLen( s ) ) )
		i++;
	if ( StrNCaselessCompare(s, MonthString[i], StrLen( s ) ) )
		return 0;

	return i + 1;
}

static Char *TimeGetMonth( Char* s, Int16* month )
{
	Char	temp[ 50 ];
	int	i = 0;

	while ( TimeIsWhiteSpace( *s ) && (*s != '\0') )
		s++;

	while ( ((*s >= 'A') && (*s <= 'Z')) ||
		((*s >= 'a') && (*s <= 'z')) ) {
			temp[i] = *s++;
			i++;
	}
	temp[i] = '\0';

	while ( TimeIsWhiteSpace( *s ) && (*s != '\0') )
		s++;

	*month = TimeStringToMonth( temp );
	if ( !*month )
		return NULL;

	return s;
}

static Char *TimeGetZone( Char* s, Int16 *offset )
{
	Char	temp[ 10 ];
	int	i = 0;

	while ( (*s == ' ') && (*s != '\0') ) s++;

	if ( (*s != '-') && (*s != '+') )
		return NULL;

	while ( (*s >= '0') && (*s <= '9') ) {
		temp[i] = *s++;
		i++;
	}
	temp[i] = '\0';
	
	*offset = (UInt16)StrAToI(temp);

	return s;
}

static Boolean TimeGetDate( Char* s, DateTimeType* dtP )
{
     Int16 zone = 0;

     s = TimeGetFragment( s );
     if (s == NULL )
         return false;

     s = TimeGetInteger( s, &dtP->day, 2 );
     if ( s == NULL )
         return false;

     s = TimeGetMonth( s, &dtP->month );
     if ( s == NULL )
         return false;

     s = TimeGetInteger( s, &dtP->year, 4 );
     if ( s == NULL )
         return false;

     s = TimeGetFragment( s );
     if (s == NULL )
         return false;

     s = TimeGetInteger( s, &dtP->hour, 2 );
     if ( s == NULL )
         return false;

     s = TimeGetFragment( s );
     if (s == NULL )
         return false;

     s = TimeGetInteger( s, &dtP->minute, 2 );
     if ( s == NULL )
         return false;

     s = TimeGetFragment( s );
     if (s == NULL )
         return false;

     s = TimeGetInteger( s, &dtP->second, 2 );
     if ( s == NULL )
         return false;

     s = TimeGetZone( s, &zone );
     if ( s == NULL )
         return false;

     /*s = TimeGetInteger( s, &dtP->year, 4 );
     if ( s == NULL )
         return false;

     s = TimeGetInteger( s, &dtP->month, 2 );
     if ( s == NULL )
         return false;

     s = TimeGetInteger( s, &dtP->day, 2 );
     if ( s == NULL )
         return false;

     s = TimeGetInteger( s, &dtP->hour, 2 );
     if ( s == NULL )
         return false;

     s = TimeGetInteger( s, &dtP->minute, 2 );
     if ( s == NULL )
         return false;

     s = TimeGetInteger( s, &dtP->second, 2 );
     if ( s == NULL )
         return false;*/

     return true;
}

static void TimeGetTimeFormat( TimeFormatType* tfP )
{
     UInt16 index;
     LmLocaleType loc;

     OmGetSystemLocale( &loc );

     if ( errNone != LmLocaleToIndex( &loc, &index ) ||
          errNone != LmGetLocaleSetting( index, lmChoiceTimeFormat, tfP, sizeof( TimeFormatType ) ) ) {
         *tfP = tfColonAMPM;
     }
}

static void TimeGetDateFormat( DateFormatType* tfP )
{
     UInt16 index;
     LmLocaleType loc;

     OmGetSystemLocale( &loc );

     if ( errNone != LmLocaleToIndex( &loc, &index ) ||
          errNone != LmGetLocaleSetting( index, lmChoiceDateFormat, tfP, sizeof( DateFormatType ) ) ) {
         *tfP = dfMDYLongWithComma;
     }
}

Char* TimeToFriendly( Char* lastPart )
{
     Char s[ MAX_FRIENDLY + 1 ];
     DateTimeType dt;
     DateType date;
     DateType curDate;
     UInt32   days;
     UInt32   curDays;
     TimeFormatType tf;

     if ( ! TimeGetDate( lastPart, &dt ) ) {
         return StrDup( lastPart );
     }

     date.month = dt.month;
     date.day   = dt.day;
     date.year  = dt.year - 1904;

     days = DateToDays( date );
     curDays = TimGetSeconds() / 86400l;
     DateDaysToDate( curDays, &curDate );

     if ( curDays == days ) {
         StrCopy( s, "Today" );
     } else if ( curDays - 1 == days ) {
         StrCopy( s, "Yesterday" );
     } else if ( curDays + 1 == days ) {
         StrCopy( s, "Tomorrow" );
     } else {
         DateToAscii( dt.month, dt.day, dt.year, dfMDYLongWithComma, s );

         if ( curDate.year + 1904 == dt.year ) {
             Char yearComp[ 12 ];

             StrPrintF( yearComp, ", %d", curDate.year + 1904 );

             if ( ! StrCompare( s + StrLen( s ) - 6, yearComp ) )
                 s[ StrLen( s ) - 6 ] = 0;
         }
     }

     StrCat( s, ", " );

     TimeGetTimeFormat( &tf );

     TimeToAscii( dt.hour, dt.minute, tf, s + StrLen( s ) );

     return StrDup( s );

}
