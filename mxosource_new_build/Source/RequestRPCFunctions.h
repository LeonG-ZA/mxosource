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
//	Defines the reqest RPC functions define the function calls that the view
//	process can request the control process to perform.  The virtual base
//	class declares the handling function which the children class implement
//	for their particular function.
//
// ***************************************************************************

#ifndef MXOSIM_REQUESTRPCFUNCTIONS_H
#define MXOSIM_REQUESTRPCFUNCTIONS_H

#include "CharacterModel.h"
#include "LocationVector.h"
#include "RequestHandler.h"



class RequestRPCFunction
{
	public :
		RequestRPCFunction ( void ) ;
		virtual void ProcessRPC ( RequestHandler* TheRequestHandler ) = 0 ;
	protected :
} ;

class JumpRPC : RequestRPCFunction
{
	public :
		JumpRPC ( CharacterModel* NewCharacter , LocationVector NewCoords ) ;
		void ProcessRPC ( RequestHandler* TheRequestHandler ) ;
	protected :
		CharacterModel* JumpCharacter ;
		LocationVector TargetCoords ;
} ;

class ChatRPC : RequestRPCFunction
{
	public :
		ChatRPC ( CharacterModel* MessageCharacter , std::string NewMsg ) ;
		void ProcessRPC ( RequestHandler* TheRequestHandler ) ;
	protected :
		CharacterModel* SourceCharacter ;
		std::string TheMsg ;
} ;

#endif MXOSIM_REQUESTRPCFUNCTIONS_H