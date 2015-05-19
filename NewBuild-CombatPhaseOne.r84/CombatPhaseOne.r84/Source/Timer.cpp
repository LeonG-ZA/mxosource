#include "Timer.h"
#include "Log.h"


Timer::Timer ( unsigned TimeoutLength , TimerType_T TimerType , bool StartTimedOut , int Offset )
{
	StartTime = clock () + Offset ;
	if ( StartTimedOut == true )
	{
		StartTime -= TimeoutLength ;
	}

	MyTimeout = TimeoutLength ;

	Type = TimerType ;
	
		// Timer starts on creation.
	Running = true ;
}



void Timer::Restart ( void )
{
	StartTime = clock () ;
	Running = true ;
}



void Timer::Stop ( void )
{
	Running = false ;
}



bool Timer::TimedOut ( void )
{
	bool timedOut = false ;
	
	if ( Running == true )
	{
		clock_t currentTime = clock () ;

		if ( ( ( ( currentTime - StartTime ) * 1000 / CLOCKS_PER_SEC ) > MyTimeout ) ||
	           // Clock rollover.
		     ( currentTime < StartTime ) )
		{
			timedOut = true ;
			if ( Type == REPEATING )
			{
				StartTime += MyTimeout ;
			}
		}
	}
	else
	{
		timedOut = true ;
	}

	return timedOut ;
}
