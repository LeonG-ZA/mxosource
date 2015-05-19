// ***************************************************************************
//
// Reality - The Matrix Online Server Emulator
// Copyright (C) 2006-2010 Rajko Stojadinovic
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
// ***************************************************************************

#include "Common.h"
#include "Util.h"
#include "Log.h"
#include "MarginSocket.h"
#include "TCPVariableLengthPacket.h"
#include "AuthServer.h"
#include "SignedDataStruct.h"
#include "Timer.h"
#include "Database/Database.h"
#include "MersenneTwister.h"
#include "MarginSocket.h"
#include "MarginServer.h"
#include "Database/Database.h"
#include "GameClient.h"
#include "GameServer.h"
#include "EncryptedPacket.h"
#include "Config.h"

#include <boost/algorithm/string.hpp>
using boost::iequals; 
using boost::erase_all;

MarginSocket::MarginSocket(ISocketHandler& h) : TCPVarLenSocket(h)
{
	memset(challenge,0,sizeof(challenge));
	charId = 0;
	sessionId = 0;
	worldCharId = 0;
	numCharacterReplies = 0;
	readyForUdp = false;
//	this->SetWillBeHalfClosed(true);

	INFO_LOG("Margin socket constructed");
}

MarginSocket::~MarginSocket()
{
	INFO_LOG("Margin socket deconstructed");
}

void MarginSocket::OnDisconnect( short info, int code )
{
/*	if (GameClient *udpClient = sGame.GetClientWithSessionId(sessionId))
		udpClient->Invalidate();
*/

	INFO_LOG("Margin socket with disconnected");
}

void MarginSocket::SendCrypted( TwofishEncryptedPacket &cryptedPacket )
{
	if (m_tfEngine.IsValid())
	{
		SendPacket(TCPVariableLengthPacket(cryptedPacket.toCipherText(m_tfEngine)));
	}
}

enum MarginOpcode
{
	CERT_ConnectRequest = 0x01,
	CERT_Challenge = 0x02,
	CERT_ChallengeResponse = 0x03,
	CERT_ConnectReply = 0x04,
	CERT_NewSessionKey = 0x05,
	MS_ConnectRequest = 0x06,
	MS_ConnectChallenge = 0x07,
	MS_ConnectChallengeResponse = 0x08,
	MS_ConnectReply = 0x09,
	MS_ClaimCharacterNameRequest = 0x0A,
	MS_ClaimCharacterNameReply = 0x0B,
	MS_CreateCharacterRequest = 0x0C,
	MS_DeleteCharacterRequest = 0x0D,
	MS_DeleteCharacterReply = 0x0E,
	MS_LoadCharacterRequest = 0x0F,
	MS_LoadCharacterReply = 0x10,
	MS_EstablishUDPSessionReply = 0x11,
	MS_RefreshSessionKeyRequest = 0x12,
	MS_RefreshSessionKeyReply = 0x13,
	MS_ServerShuttingDown = 0x14,
	MS_FailoverRequest = 0x15,
	MS_FailoverReply = 0x16,
	MS_AdjustFloodgateRequest = 0x17,
	MS_AdjustFloodgateReply = 0x18,
	MS_GetPlayerListRequest = 0x19,
	MS_GetPlayerListReply = 0x1A,
	MS_FakePopulationRequest = 0x1B,
	MS_UnloadCharacterRequest = 0x1C,
	MS_GetClientIPRequest = 0x1D,
	MS_GetClientIPReply  = 0x1E
};

void MarginSocket::ProcessData( const byte *buf,size_t len )
{
	ByteBuffer packetContents(buf,len);

	//DEBUG_LOG(format("Margin Receieved |%1%|") % Bin2Hex(packetContents) );

	bool encrypted = true;

	byte firstByte;
	packetContents >> firstByte;
	if (firstByte >= CERT_ConnectRequest && firstByte <= CERT_NewSessionKey)
	{
		uint16 firstShort;
		packetContents >> firstShort;

		if (firstShort == 3)
		{
			encrypted = false;
		}
	}
	//reset position (since we just read 3 bytes)
	packetContents.rpos(0);

	ByteBuffer packetData;

	if (encrypted == true && m_tfEngine.IsValid())
	{
		TwofishEncryptedPacket packetTodecrypt(packetContents,m_tfEngine);
		packetData = packetTodecrypt;

		//DEBUG_LOG(format("Margin Decrypted |%1%|") % Bin2Hex(packetData) );
	}
	else
	{
		packetData = packetContents;
	}

	byte packetOpcode;
	packetData >> packetOpcode;
	MarginOpcode opcode = MarginOpcode(packetOpcode);

	switch (opcode)
	{
	default:
		{
			break;
		}
	case CERT_ConnectRequest:
		{
			uint16 firstNumber;
			if (packetData.remaining() < sizeof(firstNumber))
			{
				SetCloseAndDelete(true);
				return;
			}
			packetData >> firstNumber;

			if (firstNumber != 3)
			{
				WARNING_LOG(format("CERT_ConnectRequest first num not 3 (actually %1%)") % firstNumber);
			}

			uint16 authStart;
			if (packetData.remaining() < sizeof(authStart))
			{
				SetCloseAndDelete(true);
				return;
			}
			packetData >> authStart;

			if (authStart != swap16(0x3601))
			{
				WARNING_LOG("CERT_ConnectRequest auth start not 36 01");
			}

			byte signature[128];
			if (packetData.remaining() < sizeof(signature))
			{
				SetCloseAndDelete(true);
				return;
			}
			packetData.read(signature,sizeof(signature));

			signedDataStruct signedData;
			if (packetData.remaining() < sizeof(signedData))
			{
				SetCloseAndDelete(true);
				return;
			}
			packetData.read((byte*)&signedData,sizeof(signedData));

			//verify signature, but first we need to md5
			CryptoPP::Weak::MD5 md5Object;
			md5Object.Update((const byte*)&signedData,sizeof(signedData));
			byte verifyMePlease[16];
			md5Object.Final(verifyMePlease);
			bool signatureValid = sAuth.VerifyWith1024Bit(verifyMePlease,sizeof(verifyMePlease),signature,sizeof(signature));

			if (signatureValid == false)
			{
				ERROR_LOG("CERT_ConnectRequest signature invalid, packet has been tampered, disconnecting");
				SetCloseAndDelete(true);
				return;
			}

			uint32 currTime = getTime();
			if (signedData.expiryTime < currTime) //the authentication session has expired
			{
				ERROR_LOG("CERT_ConnectRequest timestamp too old, disconnecting");
				SetCloseAndDelete(true);
				return;
			}

			m_userId = signedData.userId1;
			m_username = signedData.userName;

			//scope for db ptr
			{
				scoped_ptr<QueryResult> result(sDatabase.Query(format("SELECT `userId`, `username` FROM `users` WHERE `username` = '%1%' LIMIT 1") % m_username) );
				if (result == NULL)
				{
					INFO_LOG(format("CERT_ConnectRequest: Username %1% doesn't exist, disconnecting.") % m_username );
					SetCloseAndDelete(true);
					return;
				}

				Field *field = result->Fetch();
				uint32 dbUserId = field[0].GetUInt32();
				if (m_userId != dbUserId)
				{
					ERROR_LOG(format("CERT_ConnectRequest: UserId from packet %1% mismatches one from DB %2%, disconnecting.") % m_userId % dbUserId);
					SetCloseAndDelete(true);
					return;					
				}
			}

			//we need to generate a twofish key for usage for encrypted margin/world, and a challenge so we can verify that client can encrypt fine
			CryptoPP::AutoSeededRandomPool randPool;
			//generate random twofish key and challenge
			randPool.GenerateBlock(twofishKey,sizeof(twofishKey));
			randPool.GenerateBlock(challenge,sizeof(challenge));

			//since we now have key, lets initialize our encryptor/decryptor
			m_tfEngine.Initialize(twofishKey,sizeof(twofishKey));

			//the rsa encrypted packet is 00 then twofish key then challenge, so its 31 bytes
			ByteBuffer tobeRSAd;
			tobeRSAd << uint8(0);
			tobeRSAd.append(twofishKey,sizeof(twofishKey));
			tobeRSAd.append(challenge,sizeof(challenge));

			//make CryptoPP integers out of our exponent and modulus
			CryptoPP::Integer exponent( uint32( swap16(signedData.publicExponent) ) );

			CryptoPP::Integer modulus;
			modulus.Decode(signedData.modulus,sizeof(signedData.modulus));

			CryptoPP::RSA::PublicKey userPubKey;
			userPubKey.Initialize(modulus,exponent);

			CryptoPP::RSAES_OAEP_SHA_Encryptor rsaEncryptor(userPubKey);

			string encryptedOutput;
			CryptoPP::StringSource(string(tobeRSAd.contents(),tobeRSAd.size()),
				true, 
				new CryptoPP::PK_EncryptorFilter(randPool, rsaEncryptor, new CryptoPP::StringSink(encryptedOutput)));

			//now that we have the encrypted keys, we can respond
			TCPVariableLengthPacket response;
			response << uint8(CERT_Challenge)
				<< uint16(3)
				<< uint16(encryptedOutput.size());
			response.append(encryptedOutput);

			SendPacket(response);
			
			//DEBUG_LOG(format("Sending CERT_Challenge: |%1%|") % Bin2Hex(response) );
			break;
		}
	case CERT_ChallengeResponse:
		{
			byte clientsChallenge[16];
			if (packetData.remaining() < sizeof(clientsChallenge))
			{
				SetCloseAndDelete(true);
				return;
			}
			packetData.read(clientsChallenge,sizeof(clientsChallenge));

			if (!memcmp(clientsChallenge,challenge,sizeof(challenge)))
			{
				//DEBUG_LOG("CERT_ChallengeResponse from client correct!");
			}
			else
			{
				ERROR_LOG("CERT_ChallengeResponse from client INcorrect!");
				return;
			}

			TwofishEncryptedPacket response; // 04 00 00 00 00 response from real server
			response << uint8(CERT_ConnectReply)
				<< uint32(0); // error code possibly ?

			SendCrypted(response);

			//DEBUG_LOG(format("Sending CERT_ConnectReply: |%1%|") % Bin2Hex(response) );
			break;
		}
	case MS_ConnectRequest:
		{
			if (packetData.remaining() < 2*sizeof(uint32))
				break;

			uint32 matrixVersion;
			packetData >> matrixVersion;
			uint32 clientDllVersion;
			packetData >> clientDllVersion;

			//skip 9 bytes as we dont know what it is
			if (packetData.remaining() < 9)
				break;
			packetData.rpos(packetData.rpos()+9);

			//16 bytes weird sequence of bytes
			if (packetData.remaining() < sizeof(weirdSequenceOfBytes))
				break;
			packetData.read(weirdSequenceOfBytes,sizeof(weirdSequenceOfBytes));

			//ending 00
			if (packetData.remaining() < sizeof(uint8))
				break;

			uint8 lastByte;
			packetData >> lastByte;
			if (lastByte != 0)
			{
				WARNING_LOG(format("MS_ConnectRequest: last byte is not 0, but %1%") % uint32(lastByte));
			}
			
			/*DEBUG_LOG(format("MS_ConnectRequest: matrixVersion: %2% clientDllVersion: %2% rest: %3%")
				% ClientVersionString(matrixVersion)
				% ClientVersionString(clientDllVersion)
				% Bin2Hex(&packetData.contents()[packetData.rpos()],packetData.remaining()));*/

			//real server response with MS_ConnectChallenge which has 16 bytes some data, then 00 00 01 00
			TwofishEncryptedPacket response;
			response << uint8(MS_ConnectChallenge);
			//lets use a buffer full of FFs
			byte md5Buf[16];
			memset(md5Buf,0xFF,sizeof(md5Buf));
			response.append(md5Buf,sizeof(md5Buf));
			response << uint16(0); //md5 param 1
			response << uint16(1); //md5 param 2

			SendCrypted(response);

			//DEBUG_LOG(format("Sending MS_ConnectChallenge: |%1%|") % Bin2Hex(response) );
			break;
		}
	case MS_ConnectChallengeResponse:
		{
			vector<byte> gameFilesMd5(CryptoPP::Weak::MD5::DIGESTSIZE);
			if (packetData.remaining() < gameFilesMd5.size())
				break;

			packetData.read(&gameFilesMd5[0],gameFilesMd5.size());

			//always accept
			//we need to send something like 09 00 00 00 00 00 00 00 00 29 A6 34 E9 0F 00 03 00 0C 00 07 00 A9 00
			//                                                          ?? ?? ?? ?? <- this part changes, its margin/world session id
			TwofishEncryptedPacket response;
			response << uint8(MS_ConnectReply);
			response << uint32(0);
			response << uint32(0);
			uint32 randMax = 0xFFFFFFFF;
			sessionId = MTRand::getSingleton().randInt(randMax);
			response << sessionId;
			response << uint16(0x0F);
			response << uint16(3);
			response << uint16(0x0C);
			response << uint16(7);
			response << uint16(0xA9);

			SendCrypted(response);

			readyForUdp = true;

			//DEBUG_LOG(format("Sending MS_ConnectReply: |%1%|") % Bin2Hex(response) );
			break;
		}
	case MS_ClaimCharacterNameRequest:
		{
			vector<byte> gameFilesMd5(CryptoPP::Weak::MD5::DIGESTSIZE);
			if (packetData.remaining() < gameFilesMd5.size())
				break;

			packetData.read(&gameFilesMd5[0],gameFilesMd5.size());

			//DEBUG_LOG(format("MS_ClaimCharacterNameRequest [...] "));
			//DEBUG_LOG(format("WHAT? MS_ClaimCharacterNameRequest 1 : |%1%|") % Bin2Hex(&packetData.contents()[packetData.rpos()],packetData.remaining()) );
			//DEBUG_LOG(format("WHAT? MS_ClaimCharacterNameRequest 2 : |%1%|") % Bin2Hex(packetData) );
			//DEBUG_LOG(format("WHAT? MS_ClaimCharacterNameRequest 3 : |%1%|") % Bin2Hex(gameFilesMd5) );

			//request>0a 03 00 07 00 6d 78 6f 65 6d 75 00
			//reply>0b 0f 00 00 00 00 00 44 87 1f 00 00 00 00 00 07 00 6d 78 6f 65 6d 75 00
			/**/TwofishEncryptedPacket response;
			response << uint8(MS_ClaimCharacterNameReply);
			response << uint8(gameFilesMd5[2]);
			response << uint8(gameFilesMd5[3]);
				response << uint8(0x00);
				response << uint8(0x00);
				response << uint8(0x00);
				response << uint8(0x00);
			response << uint8(0x44);
			response << uint8(0x87);
			response << uint8(0x1f);
				response << uint8(0x00);
				response << uint8(0x00);
				response << uint8(0x00);
				response << uint8(0x00);
				response << uint8(0x00);

			string BYTE_PACKET = ""+(format("%1%") % Bin2Hex(packetData) ).str();
			std::vector<std::string> def;
			boost::split(def, BYTE_PACKET, boost::is_any_of(" "));

			string handle_str;
			for(int i = 3; i<packetData.size(); i++){
				response << uint8(packetData[i]);
				if(i>=5){
					char *offset;
					string hex = "0x"+ def[i] ;
					int charHandle = atoi( ((format("%1%")%strtol(hex.c_str( ), &offset, 0)).str()).c_str() );
					handle_str += (char)charHandle;
				}
			}

			erase_all(handle_str,"0");
			handle_str = handle_str.substr(0, handle_str.length()-1);

			SendCrypted(response);

			//DEBUG_LOG(format("Sending MS_ClaimCharacterNameReply: |%1%|") % Bin2Hex(&response.contents()[response.rpos()],response.remaining()) );
			//update database with HANDLE info (...)
			DEBUG_LOG( "USER -> "+m_username+" WANTED TO CREATE CHARACTER -> "+handle_str  );
			bool store_data_HANDLEOFCHARACTER = sDatabase.Execute(format("UPDATE `myinstallation` SET `handle` = '%2%' WHERE `userName` = '%1%'")
				% m_username 
				% sDatabase.EscapeString(handle_str) );

			//verify HANDLE presence
			//get UserID
				scoped_ptr<QueryResult> valueID(sDatabase.Query(format("SELECT `userId` FROM `users` WHERE `username` = '%1%' LIMIT 1") % m_username) );
				Field *field_valueID = valueID->Fetch();
				string userID = ""+format(field_valueID[0].GetString()).str();
			//check presence HANDLE -> (NO) proceed
			scoped_ptr<QueryResult> valueHANDLE(sDatabase.Query(format("SELECT `charId` FROM `characters` WHERE `handle` = '%1%' LIMIT 1") % sDatabase.EscapeString(handle_str)) );
			if (valueHANDLE == NULL){
				//continue (buffer)
				bool store_data_ISCREATINGCHARACTER = sDatabase.Execute(format("UPDATE `myinstallation` SET `isCreatingCharacter` = '1' WHERE `userName` = '%1%'")% m_username );
				DEBUG_LOG( "LOADING..." ); //switch -> (end of) 'MS_CreateCharacterRequest'
			}else{
				DEBUG_LOG( "ERROR -> CHARACTER "+ sDatabase.EscapeString(handle_str) +" ALREADY EXISTS.");
			}

			break;
		}
	case MS_CreateCharacterRequest:
		{
			vector<byte> gameFilesMd5(CryptoPP::Weak::MD5::DIGESTSIZE);
			if (packetData.remaining() < gameFilesMd5.size())
				break;

			packetData.read(&gameFilesMd5[0],gameFilesMd5.size());

			//DEBUG_LOG(format("WHAT? MS_CreateCharacterRequest: |%1%|") % Bin2Hex(&packetData.contents()[packetData.rpos()],packetData.remaining()) );

			string BYTE_PACKET = ""+(format("%1%") % Bin2Hex(&packetData.contents()[packetData.rpos()],packetData.remaining()) ).str();
			std::vector<std::string> def;
			boost::split(def, BYTE_PACKET, boost::is_any_of(" "));

			//prepare inventory
			string clothes;
			int K = 0;
			for(int i = 0; i<48; i++){
				if(i==16 || i==17 || i==20 || i==21 || i==24 || i==25 || i==28 || i==29 || i==32 || i==33 || i==36 || i==37 || i==44 || i==45){
					if(K<2){
						clothes+=""+def[i];
						K++;
					}else{
						clothes+=","+def[i];
						K=1;
					}
				}
			}

			//make order in inventory
			int SPACES = 0;
			int genetic = 0; string GENDER = "0";
			bool setting_female = false;

			std::vector<std::string> CLOTHES;
			boost::split(CLOTHES, clothes, boost::is_any_of(","));
			string new_clothes;
			for(int i=0; i<(int)CLOTHES.size(); i++){
				if(CLOTHES[i] != "0000"){
					new_clothes+=CLOTHES[i]+"0000,";
					//if(genetic==0){
						//scan for gender
						scoped_ptr<QueryResult> valueGENDER(sDatabase.Query(format("SELECT `clothGender` FROM `myclothes` WHERE `clothId` = '%1%' LIMIT 1") % CLOTHES[i] ) );
						if (valueGENDER == NULL){
							//DEBUG_LOG("CORRUPTED CLOTH -> "+CLOTHES[i]);
						}else{
							Field *field_valueGENDER = valueGENDER->Fetch();
							string trace = ""+format(field_valueGENDER[0].GetString()).str();
							if(trace == "FEMALE" && setting_female == false){
								GENDER = "1";
								setting_female = true;
								//DEBUG_LOG( GENDER ); // -1! (last one was '00')
							}
							//genetic++;
						}
					//}
				}else{
					SPACES++;
				}
			}
			for(int i=0; i<=SPACES; i++){
				new_clothes+="00000000,";
			}
			new_clothes = new_clothes.substr(0, new_clothes.length()-1);

			//                                                           00180380,00440280,00a40180,00140380,00140480,00240480,000c0480,00500880,00b40680,00ac0680,00180480,00b00080,
			clothes="13020000,77a70000,"+new_clothes+",00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000";
			//DEBUG_LOG("INVENTORY -> "+clothes);
			
			int INDEX = 0;
			//
			int lengthName;
			if(def[60]!="00"){
				char *offset;
				string hex = "0x"+def[61]+def[60];
				lengthName = atoi( ((format("%1%")%strtol(hex.c_str( ), &offset, 0)).str()).c_str() );
			}
			//DEBUG_LOG( (format("LENGTH NAME -> %1%")%lengthName).str()  );

			INDEX += 62; //first byte -> FirstName

			string firstName_str;
			for(int i = INDEX; i<(INDEX+lengthName); i++){
				char *offset;
				string hex = "0x"+def[i];
				int charName = atoi( ((format("%1%")%strtol(hex.c_str( ), &offset, 0)).str()).c_str() );
				firstName_str += (char)charName;
			}
			firstName_str = firstName_str.substr(0, firstName_str.length()-1);
			DEBUG_LOG( (format("TEXT NAME -> %1%")%firstName_str).str()  ); // -1! (last one was '00')

			INDEX += lengthName;

			//
			int lengthSurName;
			if(def[INDEX]!="00"){
				char *offset;
				string hex = "0x"+def[INDEX+1]+def[INDEX];
				lengthSurName = atoi( ((format("%1%")%strtol(hex.c_str( ), &offset, 0)).str()).c_str() );
			}
			//DEBUG_LOG( (format("LENGTH SURNNAME -> %1%")%lengthSurName).str()  );

			INDEX += 2; //first byte -> LastName

			string lastName_str;
			for(int i = INDEX; i<(INDEX+lengthSurName); i++){
				char *offset;
				string hex = "0x"+def[i];
				int charSurName = atoi( ((format("%1%")%strtol(hex.c_str( ), &offset, 0)).str()).c_str() );
				lastName_str += (char)charSurName;
			}
			lastName_str = lastName_str.substr(0, lastName_str.length()-1);
			DEBUG_LOG( (format("TEXT SURNNAME -> %1%")%lastName_str).str()  ); // -1! (last one was '00')

			INDEX += lengthSurName; 

			//
			int lengthDescription;
			if(def[INDEX]!="00"){
				char *offset;
				string hex = "0x"+def[INDEX+1]+def[INDEX];
				lengthDescription = atoi( ((format("%1%")%strtol(hex.c_str( ), &offset, 0)).str()).c_str() );
			}
			//DEBUG_LOG( (format("LENGTH DESCRIPTION -> %1%")%lengthDescription).str()  );

			INDEX += 2; //first byte -> Description

			string lastDescription_str;
			for(int i = INDEX; i<(INDEX+lengthDescription); i++){
				char *offset;
				string hex = "0x"+def[i];
				int charDescription = atoi( ((format("%1%")%strtol(hex.c_str( ), &offset, 0)).str()).c_str() );
				lastDescription_str += (char)charDescription;
			}
			lastDescription_str = lastDescription_str.substr(0, lastDescription_str.length()-1);
			DEBUG_LOG( (format("TEXT DESCRIPTION -> %1%")%lastDescription_str).str()  ); // -1! (last one was '00')

			//verify HANDLE presence
			//get UserID
				scoped_ptr<QueryResult> valueID(sDatabase.Query(format("SELECT `userId` FROM `users` WHERE `username` = '%1%' LIMIT 1") % m_username) );
				Field *field_valueID = valueID->Fetch();
				string userID = ""+format(field_valueID[0].GetString()).str();
			//HANDLE was available -> refer to MS_ClaimCharacterNameRequest
				scoped_ptr<QueryResult> valueCANADDCHAR(sDatabase.Query(format("SELECT `isCreatingCharacter` FROM `myinstallation` WHERE `userName` = '%1%' LIMIT 1") % m_username) );
				Field *field_valueCANADDCHAR = valueCANADDCHAR->Fetch();
				string CANADDCHAR = ""+format(field_valueCANADDCHAR[0].GetString()).str();
				if(CANADDCHAR == "1"){
					//HOW MANY CHARACTERS WERE CREATED?
					scoped_ptr<QueryResult> valueHMC(sDatabase.Query("SELECT `npcNumber` FROM `mytotalnpcnumber` WHERE `command` = 'PLAYERS' LIMIT 1") );
					Field *field_valueHMC = valueHMC->Fetch();
					int HMC = 0+ atoi( ( format(field_valueHMC[0].GetString()).str() ).c_str() );
					int NEW_CHAR_ID = HMC+1; //new character => +1 IDs
					
					//get HANDLE
					scoped_ptr<QueryResult> valueHANDLE(sDatabase.Query(format("SELECT `handle` FROM `myinstallation` WHERE `userName` = '%1%' LIMIT 1") % m_username) );
					Field *field_valueHANDLE = valueHANDLE->Fetch();
					string HANDLE = ""+format(field_valueHANDLE[0].GetString()).str();

					sDatabase.Execute(format("INSERT INTO `characters` SET `charId`='%1%', `userId`='%2%', `worldId`='%3%', `handle`='%4%', `firstName`='%5%', `lastName`='%6%', `background`='%7%', `level`='%8%' ") 
						% NEW_CHAR_ID
						% sDatabase.EscapeString(userID) % sDatabase.EscapeString("1")
						% sDatabase.EscapeString(HANDLE)
						% sDatabase.EscapeString(firstName_str)
						% sDatabase.EscapeString(lastName_str)
						% sDatabase.EscapeString(lastDescription_str)
						% sDatabase.EscapeString("1"));

				INFO_LOG("<Updating database. - This process can take some several minutes.>");

					sDatabase.Execute(format("INSERT INTO `pvpmode` SET `handle`='%1%', `pvp`='%2%', `hp`='%3%', `is`='%4%', `lvl`='%5%', `animForMe`='%6%', `animForOpponent`='%7%', `damageForOpponent`='%8%', `HandleOpponent`='%9%', `buffer`='%10%', `X_POS`='%11%', `Y_POS`='%12%', `Z_POS`='%13%', `STATE`='%14%', `STATEB`='%15%', `pve`='%16%', `IdOpponentPVE`='%17%'") 
						% sDatabase.EscapeString(HANDLE)
						% sDatabase.EscapeString("0") % sDatabase.EscapeString("10") % sDatabase.EscapeString("10") % sDatabase.EscapeString("2") % sDatabase.EscapeString("000000") % sDatabase.EscapeString("000000") % sDatabase.EscapeString("0") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("0") % sDatabase.EscapeString("NONE") );

				INFO_LOG("<PVP mode enabled....>");

					sDatabase.Execute(format("INSERT INTO `myinventory` SET `handle`='%1%', `inventory`='%2%'") 
						% sDatabase.EscapeString(HANDLE)
						% sDatabase.EscapeString(clothes) );

				INFO_LOG("<Inventory enabled...>");

				if(GENDER=="0"){
					sDatabase.Execute(format("INSERT INTO `rsivalues` SET `charId`='%1%', `sex`='%2%', `body`='%3%', `hat`='%4%', `face`='%5%', `shirt`='%6%', `coat`='%7%', `pants`='%8%', `shoes`='%9%', `gloves`='%10%', `glasses`='%11%', `hair`='%12%', `facialdetail`='%13%', `shirtcolor`='%14%', `pantscolor`='%15%', `coatcolor`='%16%', `shoecolor`='%17%', `glassescolor`='%18%', `haircolor`='%19%', `skintone`='%20%', `tattoo`='%21%', `facialdetailcolor`='%22%', `leggings`='%23%'") 
						% NEW_CHAR_ID
						% sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") );
				}else{
					//fix bug NO RSI if female
					sDatabase.Execute(format("INSERT INTO `rsivalues` SET `charId`='%1%', `sex`='%2%', `body`='%3%', `hat`='%4%', `face`='%5%', `shirt`='%6%', `coat`='%7%', `pants`='%8%', `shoes`='%9%', `gloves`='%10%', `glasses`='%11%', `hair`='%12%', `facialdetail`='%13%', `shirtcolor`='%14%', `pantscolor`='%15%', `coatcolor`='%16%', `shoecolor`='%17%', `glassescolor`='%18%', `haircolor`='%19%', `skintone`='%20%', `tattoo`='%21%', `facialdetailcolor`='%22%', `leggings`='%23%'") 
						% NEW_CHAR_ID
						% sDatabase.EscapeString("1") % sDatabase.EscapeString("1") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") );
				}

				INFO_LOG("<RSI enabled...>");

					sDatabase.Execute(format("INSERT INTO `myappearence` SET `handle`='%1%', `hat`='%2%', `glasses`='%3%', `shirt`='%4%', `gloves`='%5%', `coat`='%6%', `pants`='%7%', `shoes`='%8%', `weapon`='%9%', `tool`='%10%'") 
						% sDatabase.EscapeString(HANDLE)
						% sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") );

				INFO_LOG("<Appearence enabled...>");

					sDatabase.Execute(format("INSERT INTO `myattributes` SET `handle`='%1%', `belief`='%2%', `percepition`='%3%', `reason`='%4%', `focus`='%5%', `vitality`='%6%', `total`='%7%'") 
						% sDatabase.EscapeString(HANDLE)
						% sDatabase.EscapeString("8") % sDatabase.EscapeString("11") % sDatabase.EscapeString("5") % sDatabase.EscapeString("8") % sDatabase.EscapeString("8") % sDatabase.EscapeString("0") );

				INFO_LOG("<Attributes editing enabled...>");

					sDatabase.Execute(format("INSERT INTO `myloot` SET `handle`='%1%', `loot`='%2%'") 
						% sDatabase.EscapeString(HANDLE)
						% sDatabase.EscapeString("NONE") );

				INFO_LOG("<Loots support enabled...>");

				sDatabase.Execute(format("INSERT INTO `myquests` SET `handle`='%1%', `MISSION_STATE`='%2%', `MISSION_ID`='%3%', `TOTAL_OBJECTIVES`='%4%', `MISSION_OBJECTIVE`='%5%', `MISSION_ORGANIZATION`='%6%', `MISSION_DATA`='%7%', `NPC_FOR_ME`='%8%'") 
					% sDatabase.EscapeString(HANDLE)
					% sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("NONE") );

				INFO_LOG("<Mission support enabled...>");

					sDatabase.Execute(format("INSERT INTO `myteam` SET `handle`='%1%', `IN_TEAM`='%2%', `COMMAND`='%3%', `SLOTS`='%4%', `MEMBERS`='%5%', `CAPTAIN`='%6%', `STATE`='%7%'") 
					% sDatabase.EscapeString(HANDLE)
					% sDatabase.EscapeString("0") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("50") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("NONE") );

				INFO_LOG("<Team support enabled...>");

					sDatabase.Execute(format("INSERT INTO `myreputation` SET `handle`='%1%', `ZION`='%2%', `MACHINIST`='%3%', `MEROVINGIAN`='%4%', `EPN`='%5%', `CYPH`='%6%'") 
						% sDatabase.EscapeString(HANDLE)
						% sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") );

				INFO_LOG("<Reputation System support enabled...>");

					sDatabase.Execute(format("INSERT INTO `myplayers` SET `handle`='%1%', `fx`='%2%', `fx2`='%3%', `PAL`='%4%', `RSI`='%5%', `hp`='%6%', `update`='%7%', `cash`='%8%', `MASSIVE_CHAT`='%9%'") 
						% sDatabase.EscapeString(HANDLE)
						% sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("0") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("0") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("0")   % sDatabase.EscapeString("0")  );

				INFO_LOG("<FX support enabled...>");

					sDatabase.Execute(format("INSERT INTO `mybuddylist` SET `handle`='%1%', `buddyList`='%2%', `limit`='%3%'") 
						% sDatabase.EscapeString(HANDLE)
						% sDatabase.EscapeString("NONE") % sDatabase.EscapeString("50") );

				INFO_LOG("<Buddy List enabled...>");

					sDatabase.Execute(format("INSERT INTO `mycrew` SET `handle`='%1%', `IN_CREW`='%2%', `COMMAND`='%3%', `SLOTS`='%4%', `MEMBERS`='%5%', `NAME_CREW`='%6%', `CAPTAIN`='%7%', `STATE`='%8%', `ID_CREW`='%9%'") 
						% sDatabase.EscapeString(HANDLE)
						% sDatabase.EscapeString("0") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("50") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("0") );

				INFO_LOG("<Crew support enabled...>");

					sDatabase.Execute(format("INSERT INTO `mymarketplace` SET `handle`='%1%', `market`='%2%'") 
						% sDatabase.EscapeString(HANDLE)
						% sDatabase.EscapeString("NONE") );

				INFO_LOG("<MarketPlace support enabled...>");

					sDatabase.Execute(format("INSERT INTO `myemailaccount` SET `handle`='%1%', `emails`='%2%'") 
						% sDatabase.EscapeString(HANDLE)
						% sDatabase.EscapeString("NONE") );

				INFO_LOG("<Email Account support enabled...>");

					//RESTORE STATE
					bool store_data_RESTORESTATE = sDatabase.Execute(format("UPDATE `myinstallation` SET `isCreatingCharacter` = '0' WHERE `userName` = '%1%'")% m_username );

					//UPDATE CHARIDs
					bool store_data_UPDATECHARIDS = sDatabase.Execute(format("UPDATE `mytotalnpcnumber` SET `npcNumber` = '%1%' WHERE `command` = 'PLAYERS'")% NEW_CHAR_ID );

				INFO_LOG("<Updating database. - Done.>");
				}
			
		}
	case MS_LoadCharacterRequest:
		{
			//first 8 bytes are uint64 charid
			if (packetData.remaining() < sizeof(charId))
				break;

			packetData >> charId;
			//scope for db ptr
			{
				scoped_ptr<QueryResult> result(sDatabase.Query(format("SELECT `charId`, `userId`, `handle`, `firstName`, `lastName`, `background` FROM `characters` WHERE `userId` = '%1%' AND `charId` = '%2%' LIMIT 1") % m_userId % charId) );
				if (result == NULL)
				{
					//ERROR_LOG(format("MS_LoadCharacterRequest: Character doesn't exist or username %1% doesn't own it") % m_username );
					SetCloseAndDelete(true);
					return;
				}

				Field *field = result->Fetch();

				m_charName = field[2].GetString();
				m_firstName = field[3].GetString();
				m_lastName = field[4].GetString();

				if (field[5].GetString() != NULL)
					m_background = field[5].GetString();
				else
					m_background = string();
			}

			//Don't allow multiple users with the same character id
			if (sConfig.GetBoolDefault("MarginServer.AllowMultipleSessionsPerCharacter", false)==false)
			{
				vector<class GameClient*> allUsers = sGame.GetClientsWithCharacterId(charId);
				if(allUsers.size() > 0) //someone else already using account
				{
					//ERROR_LOG(format("MS_LoadCharacterRequest: Closing connection for %1% (one already exists)") % m_charName );
					this->SetCloseAndDelete(true);
					return;
				}
			}

			//then 32 zeroes
			vector<byte> justZeroes(32);
			if (packetData.remaining() < justZeroes.size())
				break;
			packetData.read(&justZeroes[0],justZeroes.size());
			if (std::accumulate(justZeroes.begin(),justZeroes.end(),0) != 0)
			{
				//WARNING_LOG(format("MS_LoadCharacterRequest: Zeroes were %1%") % Bin2Hex(&justZeroes[0],justZeroes.size()) );
			}

			uint32 strangeCounter=0;
			byte shouldBeStrangeThing[16];
			while (packetData.remaining() >= sizeof(shouldBeStrangeThing))
			{
				packetData.read(shouldBeStrangeThing,sizeof(shouldBeStrangeThing));
				if (memcmp(shouldBeStrangeThing,weirdSequenceOfBytes,sizeof(shouldBeStrangeThing)) != 0)
				{
					//roll back the 16 bytes we read
					packetData.rpos(packetData.rpos()-sizeof(shouldBeStrangeThing));
					break;
				}
				else
				{
					strangeCounter++;
				}
			}
			if (strangeCounter != 9)
			{
				//WARNING_LOG(format("MS_LoadCharacterRequest: Strange counter was not 9 but %1%") % strangeCounter);
			}

			//abs position in packet of weird string size uint16
			uint16 weirdStringPos;
			if (packetData.remaining() < sizeof(weirdStringPos))
				break;
			packetData >> weirdStringPos;
			if (weirdStringPos >= packetData.size())
				break;
			packetData.rpos(weirdStringPos);
			uint16 weirdStringLen;
			if (packetData.remaining() < sizeof(weirdStringLen))
				break;
			packetData >> weirdStringLen;
			if (packetData.remaining() < weirdStringLen)
				break;
			vector<byte> stringStorage(weirdStringLen);
			packetData.read(&stringStorage[0],stringStorage.size());
			if (stringStorage.size() > 1)
			{
				soeChatString = string((const char*)&stringStorage[0],stringStorage.size()-1);
				//WARNING_LOG(format("MS_LoadCharacterRequest: weird string is %1%") % soeChatString );
			}
			else
			{
				soeChatString = string();
			}

			worldCharId = charId & 0xFFFFFFFF;
			NewCharacterReply();
			//10 00 00 00 00 11 a0 07 00 00 00 01 00 01 00 00 
			//               ?? ?? ?? ?? <- world character id (unique per world, the uint64 charId is globally unique across worlds)
			byte firstOne[] = {0x00,0x00};
			SendCharacterReply(0,false,1,ByteBuffer(firstOne,sizeof(firstOne)));

			byte firstNameLastNameBackground[] =
			{
				0x10, 0x00, 0xAE, 0x04, 0x0A, 0x00, 0x00, 0x00, 0x6A, 0x00, 0x00, 0x00, 0x67, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2F, 0x01, 0x00, 0x00, 
				/* NAME */ 
				0x5f, 0x4e, 0x41, 0x4d, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				/* SURNAME */ 
				0x5f, 0x53, 0x55, 0x52, 0x4e, 0x41, 0x4d, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				/* BACKGROUND */ 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
				
				/*0x24, 0xCB, 0xBA, 0xE0, 
				0x00, 0x08, 0xAF, 0x2F, 
				0x01, 0x7A, 0x02, 0x00, 
				0x00, 0x6D, 0xE3, 0x72, 
				0x4A, 
				0x81, 0xFF, 0x81, 0xFF, 0x81, 0xFF, 
				0x67, 0x00, 0x67, 0x00, 0x67, 0x00, 
				0x00, 0x00, 
				0x03, 0x01, 0x03, 0x01, 0x31, 0x00, 
				0x00, 
				0xB4, 0x02, 0x32, 0x00, 0x00, 
				0xB4, 0x03, 0x38, 0x00, 0x00, 
				0xB4, 0x04, 0x4E, 0x00, 0x00, 
				0x00, 0x02, 0x00, 
				0x51, 0x00, 0x00, 0x00, 0x16, 0x00, 
				0x52, 0x00, 0x00, 0x00, 0x19, 0x00, 
				0x54, 0x00, 0x00, 0x00, 0x13, 0x00,*/

				0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 
				0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 
				0x03, 0x01, 0x03, 0x01, 0x31, 0x00, 
				0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 
				0x00, 0x02, 0x00, 
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			} ;

			memset(&firstNameLastNameBackground[0x28],0,32);
			strncpy((char*)&firstNameLastNameBackground[0x28],m_firstName.c_str(),31);
			memset(&firstNameLastNameBackground[0x48],0,32);
			strncpy((char*)&firstNameLastNameBackground[0x48],m_lastName.c_str(),31);
			memset(&firstNameLastNameBackground[0x68],0,1024);
			strncpy((char*)&firstNameLastNameBackground[0x68],m_background.c_str(),1023);

			SendCharacterReply(0,false,2,ByteBuffer(firstNameLastNameBackground,sizeof(firstNameLastNameBackground)));

			unsigned char packet1[] =
			{
				0x10, 0x00, 0x00, 0x00,
			} ;

			SendCharacterReply(0,false,3,ByteBuffer(packet1,sizeof(packet1)));

			unsigned char packet2[] =
			{
				0x10, 0x00, 0x00, 0x00,
			} ;

			SendCharacterReply(0,false,4,ByteBuffer(packet2,sizeof(packet2)));

			//load HARDLINES
			unsigned char packet3[] =
			{
				//Hardlines
				
				//0x10, 0x00, 0xee, 0x02,   // -> length packet (important) - length of the packet of ALL
				0x10, 0x00, 0x4a, 0x01,		// -> length packet (important) - only RICHLAND and WESTVIEW

				//01 -> Richland and Westview
				0x01, 0x30, 0x00, 0x00, 0x00, 
				0x01, 0x31, 0x00, 0x00, 0x00, 
				0x01, 0x37, 0x00, 0x00, 0x00, 
				0x01, 0x41, 0x00, 0x00,	0x00, 
				0x01, 0x45, 0x00, 0x00, 0x00, 
				0x01, 0x48, 0x00, 0x00, 0x00, 
				0x01, 0x49, 0x00, 0x00, 0x00, 
				0x01, 0x4a, 0x00, 0x00, 0x00, 
				0x01, 0x4b, 0x00, 0x00, 0x00, 
				0x01, 0x4c, 0x00, 0x00, 0x00, 
				0x01, 0x4e, 0x00, 0x00, 0x00, 
				0x01, 0x4f, 0x00, 0x00, 0x00, 
				0x01, 0x50, 0x00, 0x00, 0x00, 
				0x01, 0x51, 0x00, 0x00, 0x00, 
				0x01, 0x52, 0x00, 0x00, 0x00, 
				0x01, 0x53, 0x00, 0x00, 0x00, //Gracy fixed
				0x01, 0x63, 0x00, 0x00, 0x00, 
				0x01, 0x64, 0x00, 0x00, 0x00, 
				0x01, 0x65, 0x00, 0x00, 0x00, 
				0x01, 0x66, 0x00, 0x00, 0x00, 
				0x01, 0x67, 0x00, 0x00, 0x00, 
				0x01, 0x68, 0x00, 0x00, 0x00, 
				0x01, 0x69, 0x00, 0x00, 0x00, 
				0x01, 0x6a, 0x00, 0x00, 0x00,
				0x01, 0x6b, 0x00, 0x00, 0x00, 
				0x01, 0x6d, 0x00, 0x00, 0x00, 
				0x01, 0x6f, 0x00, 0x00, 0x00, 
				0x01, 0x70, 0x00, 0x00, 0x00, 
				0x01, 0x71, 0x00, 0x00, 0x00, 
				0x01, 0x72, 0x00, 0x00, 0x00, 
				0x01, 0x73,	0x00, 0x00, 0x00, 
				0x01, 0x74, 0x00, 0x00, 0x00, 
				0x01, 0x77, 0x00, 0x00, 0x00, 
				0x01, 0x78, 0x00, 0x00, 0x00, 
				0x01, 0x79, 0x00, 0x00, 0x00, 
				0x01, 0x7a, 0x00, 0x00, 0x00, 
				0x01, 0x7b, 0x00, 0x00,	0x00, 
				0x01, 0x7c, 0x00, 0x00, 0x00, 
				0x01, 0x7d, 0x00, 0x00, 0x00, 
				0x01, 0x7e, 0x00, 0x00, 0x00, 
				0x01, 0x7f, 0x00, 0x00, 0x00, 
				0x01, 0x80, 0x00, 0x00, 0x00, 
				0x01, 0x81, 0x00, 0x00, 0x00, 
				0x01, 0x82, 0x00, 0x00, 0x00, 
				0x01, 0x83, 0x00, 0x00, 0x00, 
				0x01, 0x84, 0x00, 0x00, 0x00, 
				0x01, 0x85, 0x00, 0x00, 0x00, 
				0x01, 0x86, 0x00, 0x00, 0x00, 
				0x01, 0x87, 0x00, 0x00, 0x00, 
				0x01, 0x88, 0x00, 0x00, 0x00, 
				0x01, 0x8a, 0x00, 0x00, 0x00, 
				0x01, 0x8b, 0x00, 0x00, 0x00,
				0x01, 0x8c, 0x00, 0x00, 0x00, 
				0x01, 0x8d, 0x00, 0x00, 0x00, 
				0x01, 0x8e, 0x00, 0x00, 0x00, 
				0x01, 0x90, 0x00, 0x00, 0x00,
				0x01, 0x91, 0x00, 0x00, 0x00, 
				0x01, 0x92, 0x00, 0x00, 0x00, 
				0x01, 0x93, 0x00, 0x00, 0x00, 
				0x01, 0x94, 0x00, 0x00, 0x00, 
				0x01, 0x95, 0x00, 0x00, 0x00, 
				0x01, 0x96, 0x00, 0x00, 0x00, 
				0x01, 0x97,	0x00, 0x00, 0x00, 
				0x01, 0x98, 0x00, 0x00, 0x00, 
				0x01, 0x99, 0x00, 0x00, 0x00, 
				0x01, 0x9a, 0x00, 0x00, 0x00, //clone at Uriah

				//02 -> Downtown
				/*0x02, 0x07, 0x00, 0x00, 0x00, 
				0x02, 0x1f, 0x00, 0x00, 0x00, 
				0x02, 0x23, 0x00, 0x00,	0x00, 
				0x02, 0x24, 0x00, 0x00, 0x00, 
				0x02, 0x25, 0x00, 0x00, 0x00, 
				0x02, 0x26, 0x00, 0x00, 0x00, 
				0x02, 0x27, 0x00, 0x00, 0x00, 
				0x02, 0x29, 0x00, 0x00, 0x00, 
				0x02, 0x2b, 0x00, 0x00, 0x00,
				0x02, 0x2f, 0x00, 0x00, 0x00, 
				0x02, 0x30, 0x00, 0x00, 0x00, 
				0x02, 0x31, 0x00, 0x00, 0x00, 
				0x02, 0x32, 0x00, 0x00, 0x00, 
				0x02, 0x34, 0x00, 0x00, 0x00, 
				0x02, 0x35, 0x00, 0x00, 0x00, 
				0x02, 0x36, 0x00, 0x00, 0x00, 
				0x02, 0x37, 0x00, 0x00, 0x00, 
				0x02, 0x38, 0x00, 0x00, 0x00, 
				0x02, 0x3b, 0x00, 0x00, 0x00, 
				0x02, 0x3f, 0x00, 0x00, 0x00, 
				0x02, 0x40, 0x00, 0x00, 0x00, 
				0x02, 0x41, 0x00, 0x00, 0x00,
				0x02, 0x43, 0x00, 0x00, 0x00, 
				0x02, 0x45, 0x00, 0x00, 0x00, 
				0x02, 0x48, 0x00, 0x00, 0x00, 
				0x02, 0x49, 0x00, 0x00, 0x00, 
				0x02, 0x4a, 0x00, 0x00, 0x00, 
				0x02, 0x4b, 0x00, 0x00, 0x00, 
				0x02, 0x4d,	0x00, 0x00, 0x00, 
				0x02, 0x4e, 0x00, 0x00, 0x00, 
				0x02, 0x4f, 0x00, 0x00, 0x00, 
				0x02, 0x50, 0x00, 0x00, 0x00, 
				0x02, 0x51, 0x00, 0x00, 0x00, 
				0x02, 0x52, 0x00, 0x00, 0x00, 
				0x02, 0x54, 0x00, 0x00,	0x00, 
				0x02, 0x55, 0x00, 0x00, 0x00, 
				0x02, 0x56, 0x00, 0x00, 0x00, 
				0x02, 0x57, 0x00, 0x00, 0x00, 
				0x02, 0x58, 0x00, 0x00, 0x00, 
				0x02, 0x59, 0x00, 0x00, 0x00, 
				0x02, 0x5a, 0x00, 0x00, 0x00, 
				0x02, 0x5b, 0x00, 0x00, 0x00, 
				0x02, 0x5c, 0x00, 0x00, 0x00, 
				0x02, 0x5d, 0x00, 0x00, 0x00, 
				0x02, 0x5e, 0x00, 0x00, 0x00, 
				0x02, 0x5f, 0x00, 0x00, 0x00, 
				0x02, 0x60, 0x00, 0x00, 0x00, 
				0x02, 0x61, 0x00, 0x00, 0x00, 
				0x02, 0x62, 0x00, 0x00, 0x00, 
				0x02, 0x63, 0x00, 0x00, 0x00, 
				0x02, 0x64, 0x00, 0x00, 0x00, 
				0x02, 0x65, 0x00, 0x00, 0x00, 
				0x02, 0x66, 0x00, 0x00, 0x00, 
				0x02, 0x67, 0x00, 0x00, 0x00,
				0x02, 0x69, 0x00, 0x00, 0x00, 
				0x02, 0x6a, 0x00, 0x00, 0x00, 
				0x02, 0x6b, 0x00, 0x00, 0x00, 
				0x02, 0x6c, 0x00, 0x00, 0x00, 
				0x02, 0x6d, 0x00, 0x00, 0x00, 
				0x02, 0x6e, 0x00, 0x00, 0x00, 
				0x02, 0x6f,	0x00, 0x00, 0x00, 
				0x02, 0x70, 0x00, 0x00, 0x00, 
				0x02, 0x71, 0x00, 0x00, 0x00, */

				//03 -> International
				/*0x03, 0x02, 0x00, 0x00, 0x00, 
				0x03, 0x03, 0x00, 0x00, 0x00, 
				0x03, 0x04, 0x00, 0x00, 0x00, 
				0x03, 0x05, 0x00, 0x00, 0x00, 
				0x03, 0x06, 0x00, 0x00, 0x00, 
				0x03, 0x07, 0x00, 0x00, 0x00, 
				0x03, 0x08, 0x00, 0x00, 0x00, 
				0x03, 0x09, 0x00, 0x00, 0x00, 
				0x03, 0x0a, 0x00, 0x00, 0x00, 
				0x03, 0x0b, 0x00, 0x00, 0x00, 
				0x03, 0x0c, 0x00, 0x00, 0x00, 
				0x03, 0x0d, 0x00, 0x00, 0x00, 
				0x03, 0x0e, 0x00, 0x00, 0x00, 
				0x03, 0x0f, 0x00, 0x00, 0x00, 
				0x03, 0x10, 0x00, 0x00, 0x00, 
				0x03, 0x11, 0x00, 0x00, 0x00, 
				0x03, 0x12, 0x00, 0x00, 0x00, 
				0x03, 0x13, 0x00, 0x00, 0x00, 
				0x03, 0x14, 0x00, 0x00, 0x00, 
				0x03, 0x15, 0x00, 0x00, 0x00, 
				0x03, 0x16, 0x00, 0x00, 0x00, 
				0x03, 0x17, 0x00, 0x00, 0x00, 
				0x03, 0x18, 0x00, 0x00, 0x00,*/
			} ;

			SendCharacterReply(0,false,8,ByteBuffer(packet3,sizeof(packet3)));

			unsigned char packet4[] =
			{
				0x10, 0x00, 0x00, 0x00,
			} ;

			SendCharacterReply(0,false,9,ByteBuffer(packet4,sizeof(packet4)));

			unsigned char packet5[] =
			{
				0x10, 0x00, 0x00, 0x00,
			} ;

			SendCharacterReply(0,false,0x0A,ByteBuffer(packet5,sizeof(packet5)));

			//load MISSION CONTACTS
			unsigned char packet6[] =
			{
				0x10, 0x00, 0x00, 0x00,

				/*0x10, 0x00, 0x06, 0x00, // -> length packet (important)
				0x01, 0x00, 0x00, 0x00, 0x00, 0x00, //ecc*/

			} ;

			SendCharacterReply(0,false,0x0B,ByteBuffer(packet6,sizeof(packet6)));

			//load INVENTORY
			ByteBuffer inventory;
			inventory << uint16(swap16(0x1000)) << uint16(936);
			scoped_ptr<QueryResult> look_inventory(sDatabase.Query(format("SELECT `inventory` FROM `myinventory` WHERE `handle` = '%1%' LIMIT 1") % m_charName) );
			if (look_inventory == NULL)	{
				SetCloseAndDelete(true);
				return;
			}else{
				Field *field_look_inventory = look_inventory->Fetch();

				std::vector<std::string> def;
				string inventoryCommaSeparatedList = field_look_inventory[0].GetString();
				inventoryCommaSeparatedList += ",00000000";
				boost::split(def, inventoryCommaSeparatedList, boost::is_any_of(","));

				for(int i = 0; i<96; i++){
					def[i] = def[i].substr(0, 8);
					inventory << uint8(i);
					inventory.append( make_shared<HexGenericMsg>(def[i]+"00001401")->toBuf() ); //<00001401>(WHITE) <0000001C>(MAX ITEM HEALTH)
				}
				
			}
			scoped_ptr<QueryResult> look_appearence(sDatabase.Query( format("SELECT `hat`, `glasses`, `shirt`, `gloves`, `coat`, `pants`, `shoes`, `weapon`, `tool` FROM `myappearence` WHERE `handle` = '%1%' LIMIT 1") % m_charName) );
			if (look_inventory == NULL)	{
				SetCloseAndDelete(true);
				return;
			}else{
				Field *field_look_appearence = look_appearence->Fetch();

				for(int i = 0; i<9; i++){
					string itemId = (format("%1%")%field_look_appearence[i].GetString()).str();
					if(i < 6){
						inventory << uint8(i+97);
					}else{
						inventory << uint8(i+98);
					}
					inventory.append( make_shared<HexGenericMsg>(itemId+"00001401")->toBuf() ); //<00001401>(WHITE) <0000001C>(MAX ITEM HEALTH)
				}
			}

			SendCharacterReply(0,false,0x05,inventory);

			unsigned char packet8[] =
			{
				// LA Skill

				0x10, 0x00, 0x0e, 0x01, // -> length packet (important)
				0x00, 0x00, 0x32, 0x08, 0x00, 0x80, //AwakenedAbility 

				0x01, 0x00, 0x00, 0x64, 0x03, 0x80, //EvadeCombatAbility

				0x02, 0x00, 0x32, 0xe0, 0x02, 0x80, //HyperSprintAbility
				0x03, 0x00, 0x32, 0x2c, 0x00, 0x80, //HyperJumpAbility
				0x04, 0x00, 0x32, 0x30, 0x00, 0x80, //HyperSpeedAbility

				0x05, 0x00, 0x00, 0x18, 0x03, 0x80, //CheapShotAbility
				0x06, 0x00, 0x00, 0x14, 0x03, 0x80, //HeadButtAbility

				0x07, 0x00, 0x00, 0x1c, 0x0f, 0x80, //MobiusCodeAbility

				0x08, 0x00, 0x32, 0xc4, 0x01, 0x80, //CombatInsightAbility
				0x09, 0x00, 0x00, 0x08, 0x02, 0x80, //HinderingShotAbility
				0x0a, 0x00, 0x00, 0xe4, 0x02, 0x80, //IntenseAttackAbility
				0x0b, 0x00, 0x00, 0x3c, 0x02, 0x80, //OperativeAbility

				0x0c, 0x00, 0x32, 0xd0, 0x01, 0x80, //CombatToughnessAbility
				0x0d, 0x00, 0x00, 0xf0, 0x01, 0x80, //FirearmsAbility
				0x0e, 0x00, 0x00, 0x44, 0x02, 0x80, //OverhandSmashAbility
				0x0f, 0x00, 0x00, 0xa4, 0x01, 0x80, //BodyShotAbility
				0x10, 0x00, 0x00, 0x68, 0x02, 0x80, //SoldierAbility

				0x11, 0x00, 0x32, 0x20, 0x02, 0x80, //MartialArtsExpertiseAbility
				0x12, 0x00, 0x00, 0x2c, 0x02, 0x80, //IronGuardAbility
				0x13, 0x00, 0x00, 0x28, 0x02, 0x80, //GuardBreakerAbility
				0x14, 0x00, 0x00, 0x24, 0x02, 0x80, //MartialArtsInitiateAbility

				0x15, 0x00, 0x00, 0x1c, 0x02, 0x80, //MartialArtsAbility

				0x16, 0x00, 0x32, 0x14, 0x02, 0x80, //KungFuAbility

				0x17, 0x00, 0x32, 0x08, 0x04, 0x80, //KungfuMasteryAbility
				0x18, 0x00, 0x32, 0x10, 0x04, 0x80, //MisdirectPunchAbility
				0x19, 0x00, 0x32, 0x0c, 0x04, 0x80, //DimMakStrikeAbility
				0x1a, 0x00, 0x00, 0x14, 0x04, 0x80, //MachinegunFistComboAbility
				0x1b, 0x00, 0x00, 0x18, 0x04, 0x80, //PistonKicksAbility

				0x1c, 0x00, 0x00, 0x04, 0x04, 0x80, //KungFuMasterAbility

				0x1d, 0x00, 0x32, 0xbc, 0x06, 0x80, //KungFuPerfectionAbility
				0x1e, 0x00, 0x32, 0xb4, 0x06, 0x80, //ExtremeFallingKickAbility
				0x1f, 0x00, 0x32, 0xb0, 0x06, 0x80, //SuicidalButterflyAbility
				0x20, 0x00, 0x00, 0xb8, 0x06, 0x80, //WoodenDummyDrillAbility
				0x21, 0x00, 0x00, 0xac, 0x06, 0x80, //TripleFrontKickAbility

				0x22, 0x00, 0x32, 0x10, 0x02, 0x80, //KarateAbility

				0x23, 0x00, 0x32, 0x20, 0x04, 0x80, //KarateExpertiseAbility
				0x24, 0x00, 0x32, 0x28, 0x04, 0x80, //SidekickComboAbility
				0x25, 0x00, 0x32, 0x24, 0x04, 0x80, //KiChargedPunchAbility
				0x26, 0x00, 0x00, 0x1c, 0x04, 0x80, //KarateMasterAbility

				0x27, 0x00, 0x32, 0x4c, 0x08, 0x80, //KarateFocusAbility
				0x28, 0x00, 0x32, 0x50, 0x08, 0x80, //MachinegunKickAbility

				0x29, 0x00, 0x32, 0xb4, 0x00, 0x80, //ExecuteProgramAbility
				0x2a, 0x00, 0x32, 0xb8, 0x0f, 0x80, //HackerCombatStyleAbility
				0x2b, 0x00, 0x00, 0xE4, 0x00, 0x80, //LogicBarrageAbility
				0x2c, 0x00, 0x32, 0xb0, 0x00, 0x80, //DownloadMissionMapAbility
			} ;

			SendCharacterReply(0,false,0x06,ByteBuffer(packet8,sizeof(packet8)));

			unsigned char packet9[] =
			{
				// Optional (Upgrade Panel - Skill)
				0x10, 0x00, 0x00, 0x00,
			} ;

			SendCharacterReply(0,false,0x07,ByteBuffer(packet9,sizeof(packet9)));

			unsigned char packet10[] =
			{
				0x10, 0x00, 0x00, 0x00,
			} ;

			SendCharacterReply(0,false,0x0e,ByteBuffer(packet10,sizeof(packet10)));

			unsigned char packet11[] =
			{
				0x10, 0x00, 0x00, 0x00,
			} ;

			SendCharacterReply(swap16(0x1027),true,0x0f,ByteBuffer(packet11,sizeof(packet11)));
			break;
		}

	}
}

void MarginSocket::SendCharacterReply( uint16 shortAfterId,bool lastPacket,uint8 opcode,ByteBuffer theData )
{
	numCharacterReplies++;

	TwofishEncryptedPacket derp;
	derp << uint8(MS_LoadCharacterReply)
		 << uint32(0)
		 << uint32(worldCharId)
		 << uint16(shortAfterId)
		 << uint8(numCharacterReplies)
		 << uint8(lastPacket)
		 << uint8(opcode);
	derp.append(&theData.contents()[theData.rpos()],theData.remaining());

	SendCrypted(derp);
}

bool MarginSocket::UdpReady(GameClient *theClient)
{
	if (!readyForUdp)
		return false;

	TwofishEncryptedPacket response;
	response << uint8(MS_EstablishUDPSessionReply)
		     << uint32(0);

	SendCrypted(response);
	return true;
}
