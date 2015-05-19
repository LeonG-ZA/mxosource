#ifndef MXOSIM_MODELRUNNABLE_H
#define MXOSIM_MODELRUNNABLE_H

#include "Util.h"
#include "Threading/ThreadStarter.h"
#include "WorldModel.h"

class ModelRunnable : public ThreadContext
{
public:
	ModelRunnable ()
	{
		MyWorldModel = new WorldModel () ;
	}
	~ModelRunnable ()
	{
		delete ( MyWorldModel ) ;
	}

    bool run()
	{
		SetThreadName("Model Runnable Thread");

        // Initiate the server. Start the loop
		while(m_threadRunning)
		{
			MyWorldModel->ProcessUpdates ();
			Sleep ( 10000 ) ;
		}
        // Loop stopped. server shutdown. Delete object
		return true;
    }

	WorldModel* GetWorldModel () { return MyWorldModel ; }

private:
	WorldModel* MyWorldModel ;
};

#endif