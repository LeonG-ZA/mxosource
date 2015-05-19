#include "Route.h"



Route::Route ( Path* ThePath ) : MyPath ( ThePath )
{
	CurrentPoint = 0 ;
}



bool Route::IncrementRoute ( void )
{
	return MyPath->IncrementPoint ( &CurrentPoint ) ;
}



LocationVector* Route::GetCurrentRoutePoint ( void )
{
	return MyPath->GetPoint ( CurrentPoint ) ;
}



Route::~Route ( void )
{
}
