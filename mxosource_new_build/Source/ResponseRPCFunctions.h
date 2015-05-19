// ***************************************************************************
//
// Reality - The Matrix Online Server Emulator
// Copyright (C) 2011 - Michael Lingg
// http://mxoemu.info
//
// ---------------------------------------------------------------------------
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ---------------------------------------------------------------------------
//
//	Defines the reqest RPC functions define the function calls that the
//	controll process can request the view process to perform.  The virtual
//	base class declares the handling function which the children class
//	implement for their particular function.
//
// ***************************************************************************

#ifndef MXOSIM_ResponseRPCFUNCTIONS_H
#define MXOSIM_ResponseRPCFUNCTIONS_H

#include "CharacterModel.h"
#include "LocationVector.h"
#include "ResponseHandler.h"



class ResponseRPCFunction
{
	public :
		ResponseRPCFunction ( void ) ;
		virtual void ProcessRPC ( ResponseHandler* TheResponseHandler ) = 0 ;
	protected :
} ;

class BroadcastPositionRPC : ResponseRPCFunction
{
	public :
		BroadcastPositionRPC ( CharacterModel* NewCharacter , bool ExcludeSource ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		CharacterModel* TheCharacter ;
		bool Exclude ;
} ;

class BroadcastWorldChatRPC : ResponseRPCFunction
{
	public :
		BroadcastWorldChatRPC ( CharacterModel* MessageCharacter , std::string NewMsg ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		CharacterModel* SourceCharacter ;
		std::string TheMsg ;
} ;

#endif MXOSIM_ResponseRPCFUNCTIONS_H