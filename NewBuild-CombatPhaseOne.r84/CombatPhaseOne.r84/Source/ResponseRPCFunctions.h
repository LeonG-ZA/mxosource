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

#ifndef MXOSIM_RESPONSERPCFUNCTIONS_H
#define MXOSIM_RESPONSERPCFUNCTIONS_H

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
		BroadcastPositionRPC ( CharacterModel* NewCharacter , CharacterModel* ExcludePlayer ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		CharacterModel* TheCharacter ;
		CharacterModel* ExcludedPlayer ;
} ;

class WorldChatRPC : ResponseRPCFunction
{
	public :
		WorldChatRPC ( CharacterModel* MessageCharacter , CharacterModel* ExcludePlayer , std::string NewMsg ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		CharacterModel* SourceCharacter ;
		CharacterModel* ExcludedPlayer ;
		std::string TheMsg ;
} ;

class SystemMessageRPC : ResponseRPCFunction
{
	public :
		SystemMessageRPC ( CharacterModel* ToCharacter , std::string NewMsg ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		CharacterModel* TheCharacter ;
		std::string TheMsg ;
} ;


class SpawnCharacterRPC : ResponseRPCFunction
{
	public :
		SpawnCharacterRPC ( PlayerCharacterModel* SpawnCharacter ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		PlayerCharacterModel* TheCharacter ;
} ;

class SpawnNPCRPC : ResponseRPCFunction
{
	public :
		SpawnNPCRPC ( NPCModel* SpawnCharacter ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		NPCModel* TheCharacter ;
} ;



class StartRotatePlayerRPC : ResponseRPCFunction
{
	public :
		StartRotatePlayerRPC ( CharacterModel* RotatePlayeracter , StartPlayerRotateMsg::TurnDir_T TurnDirection ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		CharacterModel* TheCharacter ;
		StartPlayerRotateMsg::TurnDir_T TheDirection ;
} ;

class UpdateRotatePlayerRPC : ResponseRPCFunction
{
	public :
		UpdateRotatePlayerRPC ( CharacterModel* RotatePlayeracter ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		CharacterModel* TheCharacter ;
} ;

class StopRotatePlayerRPC : ResponseRPCFunction
{
	public :
		StopRotatePlayerRPC ( CharacterModel* RotatePlayeracter ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		CharacterModel* TheCharacter ;
} ;

class StartCharStateRPC : ResponseRPCFunction
{
	public :
		StartCharStateRPC ( CharacterModel* StateCharacter , StartPlayerStateMsg::State_T NewState ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		CharacterModel* TheCharacter ;
		StartPlayerStateMsg::State_T TheState ;
} ;

class UpdateCharStateRPC : ResponseRPCFunction
{
	public :
		UpdateCharStateRPC ( CharacterModel* StateCharacter ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		CharacterModel* TheCharacter ;
} ;

class StopCharStateRPC : ResponseRPCFunction
{
	public :
		StopCharStateRPC ( CharacterModel* StateCharacter ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		CharacterModel* TheCharacter ;
} ;

class UpdateNPCStateRPC : ResponseRPCFunction
{
	public :
		UpdateNPCStateRPC ( CharacterModel* StateCharacter , UpdateNPCStateMsg::State_T NewState ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		CharacterModel* TheCharacter ;
		UpdateNPCStateMsg::State_T TheState ;
} ;

class ChangeNPCStateRPC : ResponseRPCFunction
{
	public :
		ChangeNPCStateRPC ( CharacterModel* StateCharacter , ChangeNPCStateMsg::State_T NewState ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		CharacterModel* TheCharacter ;
		ChangeNPCStateMsg::State_T TheState  ;
} ;

class RunAnimationCodeRPC : ResponseRPCFunction
{
	public :
		RunAnimationCodeRPC ( CharacterModel* AnimateCharacter , byte AnimationCode[3] ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		CharacterModel* TheCharacter ;
		byte TheAnimation[3] ;
} ;

class RunAnimationRPC : ResponseRPCFunction
{
	public :
		RunAnimationRPC ( CharacterModel* AnimateCharacter , unsigned AnimationIndex ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		CharacterModel* TheCharacter ;
		unsigned TheAnimationIndex ;
} ;

class HealthUpdateRPC : ResponseRPCFunction
{
	public :
		HealthUpdateRPC ( CharacterModel* UpdateCharacter ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		CharacterModel* TheCharacter ;
} ;

class MoodUpdateRPC : ResponseRPCFunction
{
	public :
		MoodUpdateRPC ( CharacterModel* UpdateCharacter ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		CharacterModel* TheCharacter ;
} ;

class SpawnInterlockRPC : ResponseRPCFunction
{
	public :
		SpawnInterlockRPC ( CharacterModel* FirstCharacter , unsigned InterlockId ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		CharacterModel* First ;
		unsigned TheInterlockId ;
} ;

class ViewInterlockRPC : ResponseRPCFunction
{
	public :
		ViewInterlockRPC ( CharacterModel* FirstCharacter , CharacterModel* SecondCharacter , unsigned InterlockID , unsigned InterlockIndex , unsigned CombatAnimationIndex ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		CharacterModel* First ;
		CharacterModel* Second ;
		unsigned TheInterlockID ;
		unsigned TheInterlockIndex ;
		unsigned TheAnimationIndex ;
} ;

class InterlockAnimationRPC : ResponseRPCFunction
{
	public :
		InterlockAnimationRPC ( CharacterModel* FirstCharacter , CharacterModel* SecondCharacter , unsigned InterlockID , unsigned InterlockIndex , byte AnimationCode[2] ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		CharacterModel* First ;
		CharacterModel* Second ;
		unsigned TheInterlockID ;
		unsigned TheInterlockIndex ;
		byte TheAnimationCode[2] ;
} ;

class DespawnInterlockRPC : ResponseRPCFunction
{
	public :
		DespawnInterlockRPC ( unsigned RemoveInterlock ) ;
		void ProcessRPC ( ResponseHandler* TheResponseHandler ) ;
	protected :
		unsigned TheInterlock ;
} ;

#endif MXOSIM_RESPONSERPCFUNCTIONS_H