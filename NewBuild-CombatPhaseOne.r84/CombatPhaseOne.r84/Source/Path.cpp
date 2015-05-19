#include "Path.h"



Path::Path ( PathType_T NewPathType ) : ThePath ( 0 ) , PathType ( NewPathType )
{
}



unsigned Path::AddPoint ( LocationVector* NewPoint )
{
	ThePath.push_back ( *NewPoint ) ;

	return ThePath.size () - 1 ;
}



void Path::RemovePoint ( LocationVector* OldPoint )
{
}



bool Path::IncrementPoint ( unsigned* CurrentIndex )
{
	bool validPoint = true ;
	
	if ( ++*CurrentIndex >= ThePath.size() )
	{
		if ( PathType == Patrol )
		{
			*CurrentIndex = 0 ;
		}
		else
		{
			validPoint = false ;
		}
	}
	
	return validPoint ;
}



LocationVector* Path::GetPoint ( unsigned CurrentIndex )
{
	LocationVector* currentPoint = NULL ;
	
	if ( CurrentIndex < ThePath.size () )
	{
		currentPoint = &( ThePath[CurrentIndex] ) ;
	}
	
	return currentPoint ;
}



Path::~Path ( void )
{
}
