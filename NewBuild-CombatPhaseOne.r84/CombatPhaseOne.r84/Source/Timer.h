#ifndef MXOSIM_TIMER_H
#define MXOSIM_TIMER_H

#include <time.h>



class Timer
{

	public :
	
		typedef enum TimerType_T
		{
			NON_REPEATING ,
			REPEATING ,
		} ;
	
		Timer ( unsigned TimeoutLength , TimerType_T TimerType = NON_REPEATING , bool StartTimedOut = false , int Offset = 0 ) ;

		void Restart ( void ) ; // Need offset?
		void Stop ( void ) ; // Note, a stopped timer is always timed out.
		
		bool TimedOut ( void ) ;

	private :

			// Start time in milliseconds
		clock_t StartTime ;
		
			// Timer length in milliseconds.
		unsigned MyTimeout ;
		
			// Timer resets after reporting timed out.
		TimerType_T Type ;
		
		bool Running ;
} ;

#endif MXOSIM_TIMER_H