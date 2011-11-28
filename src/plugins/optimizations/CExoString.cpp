#include "CExoString.h"

CExoString::CExoString()
{
	Text = NULL;
	Length = 0;
}

CExoString::CExoString(const char *str)
{
	if(str && *str != 0)
	{
		Length = strlen(str);
		Text = (char *) borland_new(Length+1);
		strcpy(Text, str);
	}
	else
	{
		Text = NULL;
		Length = 0;
	}
}

bool CExoString::operator==(const char *str)
{
	if(this->Text && str)
	{
		return strcmp(this->Text, str) == 0;
	}
	else
	{
		if( !Text && !str || Text && !*str || str && !*str)
			return 1;
		else
			return 0;
	}
}

CExoString *CExoString::operator=(char const *str)
{
	if(this->Text)
	{
		if(!str || strlen(str) > this->Length)
		{
			Length = 0;
			borland_close(Text);
			Text = NULL;
		}
	}
	if(str)
	{
		if(strlen(str) > 0)
		{
			if(!Text)
			{
				Length = strlen(str);
				Text = (char *) borland_new(Length+1);
			}
			strcpy(Text, str);
			return this;
		}
	}
	if(Text)
	{
		borland_close(Text);
		Text = NULL;
		Length = 0;
	}
	return this;
}

CExoString::~CExoString()
{
	Length = 0;
	if(Text)
		borland_close(Text);
	Text = NULL;
}

