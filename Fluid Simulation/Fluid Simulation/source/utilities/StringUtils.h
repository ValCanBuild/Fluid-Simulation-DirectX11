/********************************************************************
StringUtils: Contains various string utilies

Author:	Valentin Hinov
Date: 18/3/2014
*********************************************************************/

#ifndef _STRINGUTILS_H
#define	_STRINGUTILS_H

#include <string>
#include <Windows.h>

std::wstring StrToWidestr(const std::string& str)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo( size_needed, 0 );
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}


#endif