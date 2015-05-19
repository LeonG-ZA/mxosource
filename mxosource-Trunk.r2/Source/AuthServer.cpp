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
#include "AuthServer.h"
#include "AuthSocket.h"
#include "Log.h"
#include "Config.h"
#include "Database/DatabaseEnv.h"
#include "Util.h"

#include <boost/algorithm/string.hpp>
using boost::iequals;
using boost::erase_all;

initialiseSingleton( AuthServer );

void AuthServer::GenerateRSAKeys( unsigned int keyLen,CryptoPP::RSA::PublicKey &publicOutput, CryptoPP::RSA::PrivateKey &privateOutput )
{
	CryptoPP::InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize( randPool, keyLen );
	publicOutput = CryptoPP::RSA::PublicKey(params);
	privateOutput = CryptoPP::RSA::PrivateKey(params);
}

void AuthServer::GenerateSignKeys( string &privKeyOut, string &pubKeyOut )
{
	for (;;)
	{
		CryptoPP::RSA::PublicKey publicKey;
		CryptoPP::RSA::PrivateKey privateKey;
		GenerateRSAKeys(2048,publicKey,privateKey);

		//just dump private key to file so we can load it
		CryptoPP::StringSink privOutput(privKeyOut);
		privateKey.DEREncodePrivateKey(privOutput);
		privOutput.MessageEnd();

		//for public key, we will just dump exponent as modulus is always 17
		CryptoPP::StringSink pubOutput(pubKeyOut);
		publicKey.GetModulus().Encode(pubOutput,publicKey.GetModulus().MinEncodedSize());
		pubOutput.MessageEnd();

		//make sure its exactly 256 bytes, if not, regenerate
		if (pubKeyOut.size() == (2048/8))
		{
			break;
		}
		else
		{
			//clear the output strings and try again
			privKeyOut = string();
			pubKeyOut = string();
			continue;
		}
	}
}

void AuthServer::LoadSignKeys()
{
	ifstream f_signPriv;
	f_signPriv.open("signPriv.dat",ios::binary);
	if (f_signPriv.is_open())
	{
		f_signPriv.seekg(0,ios::end);
		ifstream::pos_type keySize = f_signPriv.tellg();
		vector<byte> storage;
		storage.resize(keySize);
		f_signPriv.seekg(0,ios::beg);
		f_signPriv.read((char*)&storage[0],storage.size());
		f_signPriv.close();

		string signPrivStr = string((const char*)&storage[0],storage.size());

		CryptoPP::InvertibleRSAFunction params;
		CryptoPP::StringSource pkeySource(signPrivStr,true);
		params.BERDecodePrivateKey(pkeySource,false,pkeySource.MaxRetrievable());

		signer2048bit.AccessKey() = CryptoPP::RSA::PrivateKey(params);
		verifier2048bit.AccessKey() = CryptoPP::RSA::PublicKey(params);
	}
	else
	{
		//generate new signing key
		string outPrivate, outPublic;
		GenerateSignKeys(outPrivate,outPublic);
		ofstream fileStream;
		if (outPrivate.size() > 0)
		{
			//initialize RSA engine
			CryptoPP::InvertibleRSAFunction params;
			CryptoPP::StringSource pkeySource(outPrivate,true);
			params.BERDecodePrivateKey(pkeySource,false,pkeySource.MaxRetrievable());
			signer2048bit.AccessKey() = CryptoPP::RSA::PrivateKey(params);
			verifier2048bit.AccessKey() = CryptoPP::RSA::PublicKey(params);	

			//write to file
			fileStream.open("signPriv.dat",ios::binary | ios::trunc);
			fileStream.write(outPrivate.data(),outPrivate.size());
			fileStream.close();
		}
		if (outPublic.size() > 0)
		{
			//write to file, we will copy this to mxo exe
			fileStream.open("signPub.dat",ios::binary | ios::trunc);
			fileStream.write(outPublic.data(),outPublic.size());
			fileStream.close();
		}
	}
}

ByteBuffer AuthServer::MessageFromPublicKey( CryptoPP::RSA::PublicKey &inputKey )
{
	//message to be signed/verified is just binary dump of modulus (128 bytes) and then binary dump of exponent (1 byte)
	vector<byte> modulusBinary;
	modulusBinary.resize(inputKey.GetModulus().MinEncodedSize());
	vector<byte> exponentBinary;
	exponentBinary.resize(inputKey.GetPublicExponent().MinEncodedSize());
	inputKey.GetModulus().Encode(&modulusBinary[0],modulusBinary.size());
	inputKey.GetPublicExponent().Encode(&exponentBinary[0],exponentBinary.size());

	ByteBuffer theMessage;
	theMessage.append(modulusBinary);
	theMessage.append(exponentBinary);

	return theMessage;
}

void AuthServer::GenerateCryptoKeys( string &privKeyOut, string &pubKeyOut )
{
	CryptoPP::RSA::PublicKey publicKey;
	CryptoPP::RSA::PrivateKey privateKey;
	GenerateRSAKeys(1024,publicKey,privateKey);

	//just dump private key to file so we can load it
	CryptoPP::StringSink privOutput(privKeyOut);
	privateKey.DEREncodePrivateKey(privOutput);
	privOutput.MessageEnd();

	//for public key, we have to follow mxo standard
	CryptoPP::StringSink pubOutput(pubKeyOut);
	pubOutput.PutWord32(4,CryptoPP::BIG_ENDIAN_ORDER);
	publicKey.GetModulus().DEREncode(pubOutput);
	publicKey.GetPublicExponent().DEREncode(pubOutput);
	//separates pubkey from signature
	pubOutput.Put(0);

	//generate signature
	ByteBuffer signMe = MessageFromPublicKey(publicKey);
	vector<byte> signature;
	signature.resize(signer2048bit.MaxSignatureLength());
	size_t actualSignatureSize = signer2048bit.SignMessage(randPool,(byte*)signMe.contents(),signMe.size(),&signature[0]);
	signature.resize(actualSignatureSize);

	//cache for later retrieval
	pubKeySignature = signature;

	//append signature to file
	for (size_t i=0;i<signature.size();i++)
	{
		pubOutput.Put(signature[i]);
	}
	pubOutput.MessageEnd();
}

void AuthServer::LoadCryptoKeys()
{
	bool invalidKeys = false;

	ifstream f_privateKey;
	f_privateKey.open("privkey.dat",ios::binary);
	if (f_privateKey.is_open())
	{
		f_privateKey.seekg(0,ios::end);
		ifstream::pos_type privKeySize = f_privateKey.tellg();
		vector<byte> privKeyStorage;
		privKeyStorage.resize(privKeySize);
		f_privateKey.seekg(0,ios::beg);
		f_privateKey.read((char*)&privKeyStorage[0],privKeyStorage.size());
		f_privateKey.close();

		string pkeyStr = string((const char*)&privKeyStorage[0],privKeyStorage.size());

		CryptoPP::InvertibleRSAFunction params;
		CryptoPP::StringSource pkeySource(pkeyStr,true);
		params.BERDecodePrivateKey(pkeySource,false,pkeySource.MaxRetrievable());

		CryptoPP::RSA::PrivateKey privateKey( params );
		CryptoPP::RSA::PublicKey publicKey( params );

		ifstream f_pubKey;
		f_pubKey.open("pubkey.dat",ios::binary);
		if (f_pubKey.is_open())
		{
			f_pubKey.seekg(0,ios::end);
			ifstream::pos_type pubKeySize = f_pubKey.tellg();
			vector<byte> pubKeyStorage;
			pubKeyStorage.resize(pubKeySize);
			f_pubKey.seekg(0,ios::beg);
			f_pubKey.read((char*)&pubKeyStorage[0],pubKeyStorage.size());
			f_pubKey.close();	

			ByteBuffer pubKeyBuf(pubKeyStorage);

			uint32 rsaMethod;
			pubKeyBuf >> rsaMethod;
			rsaMethod = swap32(rsaMethod);

			if (rsaMethod != 4)
			{
				invalidKeys = true;
			}
			else
			{
				vector <byte> derEncodedPubKey;
				derEncodedPubKey.resize( pubKeyBuf.size() - pubKeyBuf.rpos() - sizeof(uint8) - verifier2048bit.MaxSignatureLength() );
				pubKeyBuf.read(&derEncodedPubKey[0],derEncodedPubKey.size());
				uint8 zeroSeparator;
				pubKeyBuf >> zeroSeparator;

				if (zeroSeparator != 0)
				{
					invalidKeys = true;
				}
				else
				{
					vector<byte> signature;
					signature.resize(verifier2048bit.MaxSignatureLength());
					pubKeyBuf.read(&signature[0],signature.size());

					string pubKeyString = string((const char*)&derEncodedPubKey[0],derEncodedPubKey.size());
					CryptoPP::StringSource pubKeySource(pubKeyString,true);

					CryptoPP::Integer Modulus;
					Modulus.BERDecode(pubKeySource);
					CryptoPP::Integer Exponent;
					Exponent.BERDecode(pubKeySource);

					if (Modulus != publicKey.GetModulus() || Exponent != publicKey.GetPublicExponent())
					{
						invalidKeys = true;
					}
					else
					{
						ByteBuffer verifyMe = MessageFromPublicKey(publicKey);

						bool messageCorrect = verifier2048bit.VerifyMessage(
							(byte*)verifyMe.contents(),
							verifyMe.size(),
							&signature[0],
							signature.size());

						if (messageCorrect == true)
						{
							//cache this signature
							pubKeySignature = signature;
							invalidKeys = false;
						}
						else
						{
							invalidKeys = true;
						}
					}
				}

			}
		}
		else
		{
			invalidKeys = true;
		}

		if (invalidKeys == false)
		{
			rsaDecryptor.AccessKey() = privateKey;
			rsaEncryptor.AccessKey() = publicKey;
			signer1024bit.AccessKey() = privateKey;
			verifier1024bit.AccessKey() = publicKey;
			pubKeyModulus = publicKey.GetModulus();
		}
	}
	else
	{
		invalidKeys = true;
	}

	if (invalidKeys == true)
	{
		//generate new rsa key
		string outPrivate, outPublic;
		GenerateCryptoKeys(outPrivate,outPublic);
		ofstream fileStream;
		if (outPrivate.size() > 0)
		{
			//initialize RSA engine
			CryptoPP::InvertibleRSAFunction params;
			CryptoPP::StringSource pkeySource(outPrivate,true);
			params.BERDecodePrivateKey(pkeySource,false,pkeySource.MaxRetrievable());
			CryptoPP::RSA::PrivateKey privateKey( params );
			CryptoPP::RSA::PublicKey publicKey( params );

			rsaDecryptor.AccessKey() = privateKey;
			rsaEncryptor.AccessKey() = publicKey;
			signer1024bit.AccessKey() = privateKey;
			verifier1024bit.AccessKey() = publicKey;
			pubKeyModulus = publicKey.GetModulus();

			//write to file
			fileStream.open("privkey.dat",ios::binary | ios::trunc);
			fileStream.write(outPrivate.data(),outPrivate.size());
			fileStream.close();
		}
		if (outPublic.size() > 0)
		{
			//write to file, mxo client will use this
			fileStream.open("pubkey.dat",ios::binary | ios::trunc);
			fileStream.write(outPublic.data(),outPublic.size());
			fileStream.close();
		}
	}
}

string AuthServer::Encrypt(string input)
{
	string output;
	CryptoPP::StringSource(input,true, new CryptoPP::PK_EncryptorFilter(randPool, rsaEncryptor, new CryptoPP::StringSink(output)));
	return output;
}

string AuthServer::Decrypt(string input)
{
	string output;
	CryptoPP::StringSource(input,true, new CryptoPP::PK_DecryptorFilter(randPool, rsaDecryptor, new CryptoPP::StringSink(output)));
	return output;
}

ByteBuffer AuthServer::SignWith1024Bit( byte *message,size_t messageLen )
{
	//generate signature
	ByteBuffer signMe(message,messageLen);
	vector<byte> signature;
	signature.resize(signer1024bit.MaxSignatureLength());
	size_t actualSignatureSize = signer1024bit.SignMessage(randPool,(byte*)signMe.contents(),signMe.size(),&signature[0]);
	signature.resize(actualSignatureSize);

	return ByteBuffer(signature);
}

bool AuthServer::VerifyWith1024Bit( byte *message,size_t messageLen,byte *signature,size_t signatureLen )
{
	return verifier1024bit.VerifyMessage(message,messageLen,signature,signatureLen);
}

ByteBuffer AuthServer::GetPubKeyData()
{
	ByteBuffer result;

	string pubKeyModulusStr;
	CryptoPP::StringSink pubKeyModulusSink(pubKeyModulusStr);
	pubKeyModulus.Encode(pubKeyModulusSink,pubKeyModulus.MinEncodedSize());

	result << uint16(pubKeyModulusStr.size());
	result.append(pubKeyModulusStr);

	result << uint16(pubKeySignature.size());
	result.append(pubKeySignature);

	return result;
}

AuthServer::AuthServer()
{
	listenSocketInst = NULL;
}

AuthServer::~AuthServer()
{
	if (listenSocketInst != NULL)
	{
		delete listenSocketInst;
	}
}

void AuthServer::Start()
{
	LoadSignKeys();
	LoadCryptoKeys();

	string Interface = sConfig.GetStringDefault("AuthServer.IP","0.0.0.0");
	int Port = sConfig.GetIntDefault("AuthServer.Port",11000);
	INFO_LOG(format("Starting Auth server on port %1%") % Port);	

	if (listenSocketInst != NULL)
	{
		delete listenSocketInst;
	}
	listenSocketInst = new AuthListenSocket(authSocketHandler);

	bool bindFailed=false;
	try
	{
		if (listenSocketInst->Bind(Interface,Port)!=0)
			bindFailed=true;
	}
	catch (Exception)
	{
		bindFailed=true;
	}
	if (bindFailed)
	{
		ERROR_LOG(format("Error binding AuthServer to port %1%") % Port);
		return;
	}
	authSocketHandler.Add(listenSocketInst);
}

void AuthServer::Stop()
{
	INFO_LOG("Auth Server shutdown");
	if (listenSocketInst != NULL)
	{
		delete listenSocketInst;
		listenSocketInst = NULL;
	}
}


void AuthServer::Loop(void)
{
	authSocketHandler.Select(0, 100000);                      // 100 ms
}

string AuthServer::MakeSHA1HashHex( const string& input )
{
	CryptoPP::SHA1 hash;
	string output;
	CryptoPP::StringSource(input,true,new CryptoPP::HashFilter(hash, new CryptoPP::HexEncoder(new CryptoPP::StringSink(output),false) )); 
	return output;
}

string AuthServer::HashPassword( const string& salt, const string& password )
{
	string thingToHash = MakeSHA1HashHex(salt) + MakeSHA1HashHex(password);
	return MakeSHA1HashHex(thingToHash);
}

string AuthServer::GenerateSalt(uint32 length)
{
	string theSalt;
	for (int i=0;i<8;i++)
	{
		char charToAdd = random('!','~');
		theSalt += string(&charToAdd,1);
	}
	return theSalt;
}

uint32 AuthServer::getAccountIdForUsername( const string &username )
{
	scoped_ptr<QueryResult> query(sDatabase.Query(format("SELECT `userId` FROM `users` WHERE `username` = '%1%'") % sDatabase.EscapeString(username)));
	if (query == NULL || query->GetRowCount() == 0)
		return 0;

	Field *field = query->Fetch();
	return field[0].GetUInt32();
}

bool AuthServer::CreateAccount( const string& username,const string& password )
{	
	if (getAccountIdForUsername(username) == 0)
	{
		string salt = GenerateSalt(8);
		string passwordHash = HashPassword(salt,password);

		sDatabase.Execute(format("INSERT INTO `users` SET `username`='%1%', `passwordSalt`='%2%', `passwordHash`='%3%', `timeCreated`=UNIX_TIMESTAMP()")
			% sDatabase.EscapeString(username)
			% sDatabase.EscapeString(salt)
			% sDatabase.EscapeString(passwordHash) );

		//myinstallation
		scoped_ptr<QueryResult> valueHACCOUNTS(sDatabase.Query("SELECT `npcNumber` FROM `mytotalnpcnumber` WHERE `command` = 'ACCOUNTS' LIMIT 1") );
		Field *field_valueHACCOUNTS = valueHACCOUNTS->Fetch();
		int ACCOUNTS = 0+ atoi( ( format(field_valueHACCOUNTS[0].GetString()).str() ).c_str() );
		int NEW_ACCOUNTS = ACCOUNTS+1; //new character => +1 IDs
		bool store_data_RESTORESTATE = sDatabase.Execute(format("UPDATE `mytotalnpcnumber` SET `npcNumber` = '%1%' WHERE `command` = 'ACCOUNTS'")% NEW_ACCOUNTS );

		return sDatabase.Execute(format("INSERT INTO `myinstallation` SET `userId`='%1%', `userName`='%2%', `isCreatingCharacter`='%3%', `handle`='%4%'")
			% NEW_ACCOUNTS
			% username
			% sDatabase.EscapeString("0")
			% sDatabase.EscapeString("NONE") );
	}
	return false;
}

bool AuthServer::ChangePassword( const string& username,const string& newPass )
{
	uint32 accountId = getAccountIdForUsername(username);
	if (accountId==0)
		return false;

	string salt = GenerateSalt(8);
	string passwordHash = HashPassword(salt,newPass);

	return sDatabase.Execute(format("UPDATE `users` SET `passwordSalt`='%1%', `passwordHash`='%2%' WHERE `userId`='%3%' LIMIT 1")
		% sDatabase.EscapeString(salt)
		% sDatabase.EscapeString(passwordHash)
		% accountId );
}

uint16 AuthServer::getWorldIdForName( const string &worldName )
{
	scoped_ptr<QueryResult> query(sDatabase.Query(format("SELECT `worldId` FROM `worlds` WHERE `name` = '%1%'") % sDatabase.EscapeString(worldName)));
	if (query == NULL || query->GetRowCount() == 0)
		return 0;

	Field *field = query->Fetch();
	return field[0].GetUInt16();
}

bool AuthServer::CreateWorld( const string& worldName )
{
	if (getWorldIdForName(worldName) == 0)
		return sDatabase.Execute(format("INSERT INTO `worlds` SET `name`='%1%'") % sDatabase.EscapeString(worldName) );

	return false;
}

uint64 AuthServer::getCharIdForHandle( const string &handle )
{
	scoped_ptr<QueryResult> query(sDatabase.Query(format("SELECT `charId` FROM `characters` WHERE `handle` = '%1%'") % sDatabase.EscapeString(handle)));
	if (query == NULL || query->GetRowCount() == 0)
		return 0;

	Field *field = query->Fetch();
	return field[0].GetUInt64();	
}

bool AuthServer::CreateCharacter( const string& worldName, const string& userName, const string& charHandle, const string& firstName, const string& lastName, const string& rank )
{
	uint32 worldId = getWorldIdForName(worldName);
	uint32 userId = getAccountIdForUsername(userName);
	if (worldId == 0 || userId == 0)
		return false;

	if (getCharIdForHandle(charHandle) != 0)
		return false;

	/**/
	string new_charHandle = (format("%1%")%charHandle).str()+"_NONE";

	std::vector<std::string> DEF;
	boost::split(DEF, new_charHandle, boost::is_any_of("_"));
	if(DEF.size() > 0){
		if(DEF[1] == "NONE"){
			new_charHandle = DEF[0];
		}else{
			new_charHandle = DEF[0]+" "+DEF[1];
		}
	}

	scoped_ptr<QueryResult> valueHMC(sDatabase.Query("SELECT `npcNumber` FROM `mytotalnpcnumber` WHERE `command` = 'PLAYERS' LIMIT 1") );
	Field *field_valueHMC = valueHMC->Fetch();
	int HMC = 0+ atoi( ( format(field_valueHMC[0].GetString()).str() ).c_str() );
	int NEW_CHAR_ID = HMC+1; //new character => +1 IDs

	if(rank == "lesig" || rank == "mod" || rank == "admin"){
		sDatabase.Execute(format("INSERT INTO `characters` SET `userId`='%1%', `worldId`='%2%', `handle`='%3%', `firstName`='%4%', `lastName`='%5%', `level`='%6%', `exp`='%7%', `charId`='%8%'") 
			% userId % worldId
			% sDatabase.EscapeString(new_charHandle)
			% sDatabase.EscapeString(firstName)
			% sDatabase.EscapeString(lastName) 
			% sDatabase.EscapeString("70") 
			% sDatabase.EscapeString("314625000") 
			% NEW_CHAR_ID
			);
	}else if(rank == "player"){
		sDatabase.Execute(format("INSERT INTO `characters` SET `userId`='%1%', `worldId`='%2%', `handle`='%3%', `firstName`='%4%', `lastName`='%5%', `charId`='%6%'") 
			% userId % worldId
			% sDatabase.EscapeString(new_charHandle)
			% sDatabase.EscapeString(firstName)
			% sDatabase.EscapeString(lastName) 
			% NEW_CHAR_ID
			);
	}

	INFO_LOG("<Updating database. - This process can take some several minutes.>");

	sDatabase.Execute(format("INSERT INTO `pvpmode` SET `handle`='%1%', `pvp`='%2%', `hp`='%3%', `is`='%4%', `lvl`='%5%', `animForMe`='%6%', `animForOpponent`='%7%', `damageForOpponent`='%8%', `HandleOpponent`='%9%', `buffer`='%10%', `X_POS`='%11%', `Y_POS`='%12%', `Z_POS`='%13%', `STATE`='%14%', `STATEB`='%15%', `pve`='%16%', `IdOpponentPVE`='%17%'") 
		% new_charHandle
		% sDatabase.EscapeString("0") % sDatabase.EscapeString("10") % sDatabase.EscapeString("10") % sDatabase.EscapeString("2") % sDatabase.EscapeString("000000") % sDatabase.EscapeString("000000") % sDatabase.EscapeString("0") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("0")% sDatabase.EscapeString("0")% sDatabase.EscapeString("0")% sDatabase.EscapeString("0") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("0") % sDatabase.EscapeString("NONE")  );

	INFO_LOG("<PVP mode enabled....>");

	sDatabase.Execute(format("INSERT INTO `myinventory` SET `handle`='%1%', `inventory`='%2%'") 
		% new_charHandle
		% sDatabase.EscapeString("13020000,77a70000,00180380,00440280,00a40180,00140380,00140480,00240480,000c0480,00500880,00b40680,00ac0680,00180480,00b00080,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000") );

	INFO_LOG("<Inventory enabled...>");

	sDatabase.Execute(format("INSERT INTO `rsivalues` SET `charId`='%1%', `sex`='%2%', `body`='%3%', `hat`='%4%', `face`='%5%', `shirt`='%6%', `coat`='%7%', `pants`='%8%', `shoes`='%9%', `gloves`='%10%', `glasses`='%11%', `hair`='%12%', `facialdetail`='%13%', `shirtcolor`='%14%', `pantscolor`='%15%', `coatcolor`='%16%', `shoecolor`='%17%', `glassescolor`='%18%', `haircolor`='%19%', `skintone`='%20%', `tattoo`='%21%', `facialdetailcolor`='%22%', `leggings`='%23%'") 
		% NEW_CHAR_ID
		% sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") );

	INFO_LOG("<RSI enabled...>");

	sDatabase.Execute(format("INSERT INTO `myappearence` SET `handle`='%1%', `hat`='%2%', `glasses`='%3%', `shirt`='%4%', `gloves`='%5%', `coat`='%6%', `pants`='%7%', `shoes`='%8%', `weapon`='%9%', `tool`='%10%'") 
		% new_charHandle
		% sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") );

	INFO_LOG("<Appearence enabled...>");

	sDatabase.Execute(format("INSERT INTO `myattributes` SET `handle`='%1%', `belief`='%2%', `percepition`='%3%', `reason`='%4%', `focus`='%5%', `vitality`='%6%', `total`='%7%'") 
		% new_charHandle
		% sDatabase.EscapeString("8") % sDatabase.EscapeString("11") % sDatabase.EscapeString("5") % sDatabase.EscapeString("8") % sDatabase.EscapeString("8") % sDatabase.EscapeString("0") );

	INFO_LOG("<Attributes editing enabled...>");

	sDatabase.Execute(format("INSERT INTO `myloot` SET `handle`='%1%', `loot`='%2%'") 
		% new_charHandle
		% sDatabase.EscapeString("NONE") );

	INFO_LOG("<Loots support enabled...>");

	sDatabase.Execute(format("INSERT INTO `myquests` SET `handle`='%1%', `MISSION_STATE`='%2%', `MISSION_ID`='%3%', `TOTAL_OBJECTIVES`='%4%', `MISSION_OBJECTIVE`='%5%', `MISSION_ORGANIZATION`='%6%', `MISSION_DATA`='%7%', `NPC_FOR_ME`='%8%'") 
		% new_charHandle
		% sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("NONE") );

	INFO_LOG("<Mission support enabled...>");

	sDatabase.Execute(format("INSERT INTO `myteam` SET `handle`='%1%', `IN_TEAM`='%2%', `COMMAND`='%3%', `SLOTS`='%4%', `MEMBERS`='%5%', `CAPTAIN`='%6%', `STATE`='%7%'") 
		% new_charHandle
		% sDatabase.EscapeString("0") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("50") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("NONE") );

	INFO_LOG("<Team support enabled...>");

	sDatabase.Execute(format("INSERT INTO `myreputation` SET `handle`='%1%', `ZION`='%2%', `MACHINIST`='%3%', `MEROVINGIAN`='%4%', `EPN`='%5%', `CYPH`='%6%'") 
		% new_charHandle
		% sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") % sDatabase.EscapeString("0") );

	INFO_LOG("<Reputation System support enabled...>");

	if(rank == "player"){
		sDatabase.Execute(format("INSERT INTO `myplayers` SET `handle`='%1%', `fx`='%2%', `fx2`='%3%', `PAL`='%4%', `RSI`='%5%', `hp`='%6%', `update`='%7%', `cash`='%8%', `MASSIVE_CHAT`='%9%'") 
			% new_charHandle
			% sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("0") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("0") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("0")  % sDatabase.EscapeString("0") );
	}else if(rank == "lesig"){
		sDatabase.Execute(format("INSERT INTO `myplayers` SET `handle`='%1%', `fx`='%2%', `fx2`='%3%', `PAL`='%4%', `RSI`='%5%', `hp`='%6%', `update`='%7%', `cash`='%8%', `MASSIVE_CHAT`='%9%'") 
			% new_charHandle
			% sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("1") % sDatabase.EscapeString(new_charHandle) % sDatabase.EscapeString("0") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("0")  % sDatabase.EscapeString("0") );
	}else if(rank == "mod"){
		sDatabase.Execute(format("INSERT INTO `myplayers` SET `handle`='%1%', `fx`='%2%', `fx2`='%3%', `PAL`='%4%', `RSI`='%5%', `hp`='%6%', `update`='%7%', `cash`='%8%', `MASSIVE_CHAT`='%9%'") 
			% new_charHandle
			% sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("2") % sDatabase.EscapeString(new_charHandle) % sDatabase.EscapeString("0") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("0")  % sDatabase.EscapeString("0") );
	}else if(rank == "lesig"){
		sDatabase.Execute(format("INSERT INTO `myplayers` SET `handle`='%1%', `fx`='%2%', `fx2`='%3%', `PAL`='%4%', `RSI`='%5%', `hp`='%6%', `update`='%7%', `cash`='%8%', `MASSIVE_CHAT`='%9%'") 
			% new_charHandle
			% sDatabase.EscapeString("00000000") % sDatabase.EscapeString("00000000") % sDatabase.EscapeString("3") % sDatabase.EscapeString(new_charHandle) % sDatabase.EscapeString("0") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("0")  % sDatabase.EscapeString("0") );
	}
	INFO_LOG("<FX support enabled...>");

	sDatabase.Execute(format("INSERT INTO `mybuddylist` SET `handle`='%1%', `buddyList`='%2%', `limit`='%3%'") 
		% new_charHandle
		% sDatabase.EscapeString("NONE") % sDatabase.EscapeString("50") );

	INFO_LOG("<Buddy List enabled...>");

	if(rank == "lesig" || rank == "mod" || rank == "admin"){
		scoped_ptr<QueryResult> valueCREW(sDatabase.Query("SELECT `MEMBERS` FROM `mycrew` WHERE `handle` = 'System' LIMIT 1") );
		Field *field_valueCREW = valueCREW->Fetch();
		string CREW = field_valueCREW[0].GetString();
		if(CREW == "NONE"){
			CREW = new_charHandle;
		}else{
			CREW+=","+new_charHandle;
		}

		sDatabase.Execute(format("INSERT INTO `mycrew` SET `handle`='%1%', `IN_CREW`='%2%', `COMMAND`='%3%', `SLOTS`='%4%', `MEMBERS`='%5%', `NAME_CREW`='%6%', `CAPTAIN`='%7%', `STATE`='%8%', `ID_CREW`='%9%'") 
			% new_charHandle
			% sDatabase.EscapeString("1") % sDatabase.EscapeString("RECRUIT") % sDatabase.EscapeString("50") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("System") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("0") );

		bool store_data_updateCREW_LET = sDatabase.Execute(format("UPDATE `mycrew` SET `MEMBERS` = '%1%' WHERE `handle` = 'System'")% CREW );
	}else if(rank == "player"){
		sDatabase.Execute(format("INSERT INTO `mycrew` SET `handle`='%1%', `IN_CREW`='%2%', `COMMAND`='%3%', `SLOTS`='%4%', `MEMBERS`='%5%', `NAME_CREW`='%6%', `CAPTAIN`='%7%', `STATE`='%8%', `ID_CREW`='%9%'") 
			% new_charHandle
			% sDatabase.EscapeString("0") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("50") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("NONE") % sDatabase.EscapeString("0") );
	}
	INFO_LOG("<Crew support enabled...>");

	sDatabase.Execute(format("INSERT INTO `mymarketplace` SET `handle`='%1%', `market`='%2%'") 
		% new_charHandle
		% sDatabase.EscapeString("NONE") );

	INFO_LOG("<MarketPlace support enabled...>");

	sDatabase.Execute(format("INSERT INTO `myemailaccount` SET `handle`='%1%', `emails`='%2%'") 
		% new_charHandle
		% sDatabase.EscapeString("NONE") );

	INFO_LOG("<Email Account support enabled...>");

	bool store_data_RESTORESTATE = sDatabase.Execute(format("UPDATE `mytotalnpcnumber` SET `npcNumber` = '%1%' WHERE `command` = 'PLAYERS'")% NEW_CHAR_ID );

	INFO_LOG("<Updating database. - Done.>");

	return true;
}

void AuthServer::SetItemCash( const string& itemId, const string& price )
{

	INFO_LOG("<Updating database. - This process can take some several minutes.>");

	sDatabase.Execute(format("UPDATE `%1%` SET `%2%` = '%3%' WHERE `%4%` = '%5%'")% "myitems" % "cash" % price % "itemId" % itemId );

	scoped_ptr<QueryResult> value(sDatabase.Query(format("SELECT `%2%` FROM `%1%` WHERE `%3%` = '%4%' LIMIT 1") % "myitems" % "itemName" % "itemId" % itemId ) );
	if (value == NULL){
		INFO_LOG("Impossible to update this item. It doesn't exist.");
	}else{
		Field *field_value = value->Fetch();
		string def = ""+format(field_value[0].GetString()).str();
		INFO_LOG("Updating item: "+def);
	}

	INFO_LOG("<Updating database. - Done.>");
}

void AuthServer::SetProcedureAdmin( const string& value )
{
	
	INFO_LOG("<Updating database. - This process can take some several minutes.>");

	sDatabase.Execute(format("UPDATE `mytotalnpcnumber` SET `npcNumber` = '%1%' WHERE `command` = 'PROCEDURE_ADMIN' ")% value );

	INFO_LOG("<Updating database. - Done.>");
}

void AuthServer::UpdateMissionSystem()
{
	
	INFO_LOG("<Updating database. - This process can take some several minutes.>");

	scoped_ptr<QueryResult> valueM(sDatabase.Query("SELECT `npcNumber` FROM `mytotalnpcnumber` WHERE `command` = 'MISSIONS' LIMIT 1") );
	Field *field_valueM = valueM->Fetch();
	int MISSIONS = 0+ atoi( ( format(field_valueM[0].GetString()).str() ).c_str() );
	int NEW_COUNTER_MISSIONS = MISSIONS+1; //new mission => +1 IDs
	bool store_data_MISSIONS = sDatabase.Execute(format("UPDATE `mytotalnpcnumber` SET `npcNumber` = '%1%' WHERE `command` = 'MISSIONS'")% NEW_COUNTER_MISSIONS );

	INFO_LOG("<Updating database. - Done.>");
}

void AuthServer::UpdateNPCSystem()
{
	
	INFO_LOG("<Updating database. - This process can take some several minutes.>");

	scoped_ptr<QueryResult> valueNPC(sDatabase.Query("SELECT `npcNumber` FROM `mytotalnpcnumber` WHERE `command` = 'SPAWN' LIMIT 1") );
	Field *field_valueNPC = valueNPC->Fetch();
	int NPCS = 0+ atoi( ( format(field_valueNPC[0].GetString()).str() ).c_str() );
	int NEW_COUNTER_NCPS = NPCS+1; //new npc => +1 IDs
	bool store_data_NPCS = sDatabase.Execute(format("UPDATE `mytotalnpcnumber` SET `npcNumber` = '%1%' WHERE `command` = 'SPAWN'")% NEW_COUNTER_NCPS );

	INFO_LOG("<Updating database. - Done.>");
}

void AuthServer::UpdateClothSystem()
{
	
	INFO_LOG("<Updating database. - This process can take some several minutes.>");

	scoped_ptr<QueryResult> valueC(sDatabase.Query("SELECT `npcNumber` FROM `mytotalnpcnumber` WHERE `command` = 'CLOTHES' LIMIT 1") );
	Field *field_valueC = valueC->Fetch();
	int CLOTHES = 0+ atoi( ( format(field_valueC[0].GetString()).str() ).c_str() );
	int NEW_COUNTER_CLOTHES = CLOTHES+1; //new clothes => +1 IDs
	bool store_data_NPCS = sDatabase.Execute(format("UPDATE `mytotalnpcnumber` SET `npcNumber` = '%1%' WHERE `command` = 'CLOTHES'")% NEW_COUNTER_CLOTHES );

	INFO_LOG("<Updating database. - Done.>");
}

void AuthServer::SetLimitAreaChat( const string& value )
{
	INFO_LOG("<Updating database. - This process can take some several minutes.>");

	bool store_data_NPCS = sDatabase.Execute(format("UPDATE `mytotalnpcnumber` SET `npcNumber` = '%1%' WHERE `command` = 'LIMIT_AREACHAT'")% value );

	INFO_LOG("<Updating database. - Done.>");
}

void AuthServer::REBOOTMESSAGE(){

	scoped_ptr<QueryResult> valueHMC(sDatabase.Query("SELECT `npcNumber` FROM `mytotalnpcnumber` WHERE `command` = 'PLAYERS' LIMIT 1") );
	Field *field_valueHMC = valueHMC->Fetch();
	int players = 0+ atoi( ( format(field_valueHMC[0].GetString()).str() ).c_str() );

	for(int i = 0; i=players; i++){

		string id = (format("%1%")%i).str();

		scoped_ptr<QueryResult> valueC(sDatabase.Query( format("SELECT `handle` FROM `characters` WHERE `charId` = '%1%' LIMIT 1") % id ) );
		Field *field_valueC = valueC->Fetch();
		string handle = field_valueC[0].GetString();

		bool store_reboot = sDatabase.Execute(format("UPDATE `myplayers` SET `update` = 'REBOOT_SERVER' WHERE `handle` = '%1%'") % handle );

	}

}
