#ifndef MXOSIM_CONTROLRUNNABLE_H
#define MXOSIM_CONTROLRUNNABLE_H

#include "Util.h"
#include "Threading/ThreadStarter.h"
#include "ControlServer.h"

class ControlRunnable : public ThreadContext
{
public:
	ControlRunnable ( HandleRequestRPC* NewRequestRPC , ProduceResponseRPC* NewResponseRPC )
	{
		MyControlServer = new ControlServer ( NewRequestRPC , NewResponseRPC ) ;
	}
	~ControlRunnable ()
	{
		delete ( MyControlServer ) ;
	}

    bool run()
	{
		SetThreadName("Control Runnable Thread");

        // Initiate the server. Start the loop
		while(m_threadRunning)
		{
			MyControlServer->ProcessRequests ();
			Sleep ( 10 ) ;
		}
        // Loop stopped. server shutdown. Delete object
		return true;
    }

	ControlServer* GetControlServer () { return MyControlServer ; }

private:
	ControlServer* MyControlServer ;
};

#endif