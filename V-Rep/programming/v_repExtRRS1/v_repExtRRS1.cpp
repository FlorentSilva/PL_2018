// This file is part of the RRS-1 interface for V-REP
// 
// Copyright 2006-2015 Coppelia Robotics GmbH. All rights reserved. 
// marc@coppeliarobotics.com
// www.coppeliarobotics.com
// 
// This interface is licensed under the terms of GNU GPL:
// 
// -------------------------------------------------------------------
// This interface is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// THIS INTERFACE IS DISTRIBUTED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
// WARRANTY. THE USER WILL USE IT AT HIS/HER OWN RISK. THE ORIGINAL
// AUTHORS AND COPPELIA ROBOTICS GMBH WILL NOT BE LIABLE FOR DATA LOSS,
// DAMAGES, LOSS OF PROFITS OR ANY OTHER KIND OF LOSS WHILE USING OR
// MISUSING THIS SOFTWARE.
// 
// See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with the REMOTE API.  If not, see <http://www.gnu.org/licenses/>.
// -------------------------------------------------------------------
//
// This file was automatically created for V-REP release V3.2.3 rev4 on December 21st 2015

#include "v_repExtRRS1.h"
#include "v_repLib.h"
#include "luaFunctionData.h"
#include "socketOutConnection.h"
#include "inputOutputBlock.h"
#include <iostream>
#include <boost/lexical_cast.hpp>

#ifdef _WIN32
	#ifdef QT_COMPIL
		#include <direct.h>
	#else
		#include <Shellapi.h>
		#include <shlwapi.h>
		#pragma comment(lib, "Shlwapi.lib")
		#pragma comment(lib, "Shell32.lib")
	#endif
#endif
#ifdef QT_COMPIL
	#include <QString>
	#include <QProcess>
#endif
#if defined (__linux) || defined (__APPLE__)
	#include <unistd.h>
	#define _stricmp(x,y) strcasecmp(x,y)
#endif

#define PLUGIN_VERSION 1
#define CONCAT(x,y,z) x y z
#define strConCat(x,y,z)	CONCAT(x,y,z)

LIBRARY vrepLib;

struct sRcsServer
{
	CSocketOutConnection* connection;
	int scriptId;
	int rcsServerHandle;
	bool isCurrentServer;
};

std::vector<sRcsServer> allRcsServers;
int nextRcsServerHandle=0;
std::string currentDirAndPath;

int getServerIndexFromServerHandle(int serverHandle)
{
	for (unsigned int i=0;i<allRcsServers.size();i++)
	{
		if (allRcsServers[i].rcsServerHandle==serverHandle)
			return(i);
	}
	return(-1);
}

int getServerIndexFromScriptId(int scriptId)
{
	for (unsigned int i=0;i<allRcsServers.size();i++)
	{
		if ( (allRcsServers[i].scriptId==scriptId)&&allRcsServers[i].isCurrentServer )
			return(i);
	}
	return(-1);
}

std::string getPartialString(const std::string& str,int charCnt)
{
    std::string retString;
    retString.assign(str.begin(),str.begin()+charCnt);
    return(retString);
}

// --------------------------------------------------------------------------------------
// Auxiliary function
// --------------------------------------------------------------------------------------
#define LUA_START_RCS_SERVER_COMMAND "simExtRRS1_startRcsServer"

const int inArgs_START_RCS_SERVER[]={
	3,
	sim_lua_arg_string,0,
	sim_lua_arg_string,0,
    sim_lua_arg_int,0,
};

void LUA_START_RCS_SERVER_CALLBACK(SLuaCallBack* p)
{
	p->outputArgCount=0;
	CLuaFunctionData D;
	int handle=-1; // means error
	if (D.readDataFromLua(p,inArgs_START_RCS_SERVER,inArgs_START_RCS_SERVER[0],LUA_START_RCS_SERVER_COMMAND))
	{
		std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
		std::string arguments;
		arguments+=inData->at(0).stringData[0];
		arguments+=" ";
		arguments+=inData->at(1).stringData[0];
		arguments+=" ";
		arguments+=boost::lexical_cast<std::string>(inData->at(2).intData[0]);

		#ifdef QT_COMPIL
			QStringList strList;
			strList << QString(inData->at(0).stringData[0].c_str());
			strList << QString(inData->at(1).stringData[0].c_str());
			strList << QString(std::string(boost::lexical_cast<std::string>(inData->at(2).intData[0])).c_str());
			QProcess::startDetached("./rcsServer",strList,QString(currentDirAndPath.c_str()),NULL);
			#ifdef _WIN32
				Sleep(1000);
			#else
				sleep(1);
			#endif
		#else
			#ifdef _WIN32
				ShellExecuteA(NULL,"open","rcsServer",arguments.c_str(),NULL,SW_SHOWDEFAULT);
				Sleep(1000);
			#else
				if(fork()==0)
				{
					execl("rcsServer","rcsServer",inData->at(0).stringData[0].c_str(),inData->at(1).stringData[0].c_str(),boost::lexical_cast<std::string>(inData->at(2).intData[0]).c_str(), (char*) 0);
					exit(0);
				}
				sleep(1);
			#endif
		#endif

		CSocketOutConnection* connection=new CSocketOutConnection("127.0.0.1",inData->at(2).intData[0]);
		if (connection->connectToServer()==1)
		{ // we could connect!
			// Now check if the rcsSever could correctly load the specified rcs library and bind the rcs function:
			if (connection->sendData("0",1))
			{ // send was successful
				// Now wait for the status reply:
				int dataSize;
				char* data=connection->receiveReplyData(dataSize);
				if (dataSize>0)
				{ // data reception was ok!
					if (data[0]==2)
					{ // ok, rcs server was launched and loaded the RCS library, and the RCS service function was successful!
						handle=nextRcsServerHandle++;
						sRcsServer server;
						server.connection=connection;
						server.scriptId=p->scriptID;
						server.rcsServerHandle=handle;
						server.isCurrentServer=true;
						allRcsServers.push_back(server);
					}
					else
					{ // there was a problem
						if (data[0]==0)
							simSetLastError(LUA_START_RCS_SERVER_COMMAND,"The RCS server failed to load the RCS library.");
						if (data[0]==1)
							simSetLastError(LUA_START_RCS_SERVER_COMMAND,"The RCS server failed to bind the RCS service function.");
						delete connection;
					}
					delete[] data;
				}
				else
				{
					delete connection;
					simSetLastError(LUA_START_RCS_SERVER_COMMAND,"Failed to receive data from the RCS server.");
				}
			}
			else
			{
				delete connection;
				simSetLastError(LUA_START_RCS_SERVER_COMMAND,"Failed to send data to the RCS server.");
			}
		}
		else
		{
			delete connection;
			simSetLastError(LUA_START_RCS_SERVER_COMMAND,"Failed to start or connect to RCS server. (i.e. 'rcsServer.exe')");
		}
			
	}
	D.pushOutData(CLuaFunctionDataItem(handle));
	D.writeDataToLua(p);
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// Auxiliary function
// --------------------------------------------------------------------------------------
#define LUA_SELECT_RCS_SERVER_COMMAND "simExtRRS1_selectRcsServer"

const int inArgs_SELECT_RCS_SERVER[]={
	1,
    sim_lua_arg_int,0,
};

void LUA_SELECT_RCS_SERVER_CALLBACK(SLuaCallBack* p)
{
	p->outputArgCount=0;
	CLuaFunctionData D;
	bool success=false;
	if (D.readDataFromLua(p,inArgs_SELECT_RCS_SERVER,inArgs_SELECT_RCS_SERVER[0],LUA_SELECT_RCS_SERVER_COMMAND))
	{
		std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
		int serverHandle=inData->at(0).intData[0];
		int scriptId=p->scriptID;
		int index=getServerIndexFromServerHandle(serverHandle);
		if (index!=-1)
		{
			if (allRcsServers[index].scriptId==scriptId)
			{
				for (unsigned int i=0;i<allRcsServers.size();i++)
				{
					if (allRcsServers[i].scriptId==scriptId)
						allRcsServers[i].isCurrentServer=false;
				}
				allRcsServers[index].isCurrentServer=true;
				success=true;
			}
			else
				simSetLastError(LUA_SELECT_RCS_SERVER_COMMAND,"Cannot access RCS server started in a different script.");
		}
		else
			simSetLastError(LUA_SELECT_RCS_SERVER_COMMAND,"Invalid RCS server handle.");
	}
	D.pushOutData(CLuaFunctionDataItem(success));
	D.writeDataToLua(p);
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// Auxiliary function
// --------------------------------------------------------------------------------------
#define LUA_STOP_RCS_SERVER_COMMAND "simExtRRS1_stopRcsServer"

const int inArgs_STOP_RCS_SERVER[]={
	1,
    sim_lua_arg_int,0,
};

void LUA_STOP_RCS_SERVER_CALLBACK(SLuaCallBack* p)
{
	p->outputArgCount=0;
	CLuaFunctionData D;
	bool success=false;
	if (D.readDataFromLua(p,inArgs_STOP_RCS_SERVER,inArgs_STOP_RCS_SERVER[0],LUA_STOP_RCS_SERVER_COMMAND))
	{
		std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
		int serverHandle=inData->at(0).intData[0];
		int scriptId=p->scriptID;
		int index=getServerIndexFromServerHandle(serverHandle);
		if (index!=-1)
		{
			if (allRcsServers[index].scriptId==scriptId)
			{
				delete allRcsServers[index].connection;
				allRcsServers.erase(allRcsServers.begin()+index);
				success=true;
			}
			else
				simSetLastError(LUA_STOP_RCS_SERVER_COMMAND,"Cannot access RCS server started in a different script.");
		}
		else
			simSetLastError(LUA_STOP_RCS_SERVER_COMMAND,"Invalid RCS server handle.");
	}
	D.pushOutData(CLuaFunctionDataItem(success));
	D.writeDataToLua(p);
}
// --------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------
// INITIALIZE
// --------------------------------------------------------------------------------------
#define LUA_INITIALIZE_COMMAND "simExtRRS1_INITIALIZE"
const int inArgs_INITIALIZE[]={
	6,
	sim_lua_arg_int,0,
	sim_lua_arg_string,0,
	sim_lua_arg_string,0,
	sim_lua_arg_string,0,
	sim_lua_arg_int,0,
    sim_lua_arg_int,0,
};

void LUA_INITIALIZE_CALLBACK(SLuaCallBack* p)
{
	p->outputArgCount=0;
	CLuaFunctionData D;
	if (D.readDataFromLua(p,inArgs_INITIALIZE,inArgs_INITIALIZE[0],LUA_INITIALIZE_COMMAND))
	{
		std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
		int scriptId=p->scriptID;
		int index=getServerIndexFromScriptId(scriptId);
		if (index!=-1)
		{
			CInputOutputBlock inputBlock(101,0);
			inputBlock.pushBackInteger(inData->at(0).intData[0]);
			inputBlock.pushBackString(inData->at(1).stringData[0]);
			inputBlock.pushBackString(inData->at(2).stringData[0]);
			inputBlock.pushBackString(inData->at(3).stringData[0]);
			inputBlock.pushBackInteger(inData->at(4).intData[0]);
			inputBlock.pushBackInteger(inData->at(5).intData[0]);

			int inputBlockSize;
			void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
			if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
			{
				int outputBlockSize;
				char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
				if (data!=0)
				{
					CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
					delete[] data;
					if (outputBlockSize>=4)
					{
						int s=outputBlock.readInteger();
						if (s>=8)
						{
							int status=outputBlock.readInteger();
							D.pushOutData(CLuaFunctionDataItem(status));
						}
						if (s>=16)
						{
                            std::string rcsHandle=outputBlock.readBitstring2();
                            D.pushOutData(CLuaFunctionDataItem(&rcsHandle[0],BITSTRING2_SIZE));
						}
						if (s>=20)
						{
							int version1=outputBlock.readInteger();
							D.pushOutData(CLuaFunctionDataItem(version1));
						}
						if (s>=24)
						{
							int version2=outputBlock.readInteger();
							D.pushOutData(CLuaFunctionDataItem(version2));
						}
						if (s>=28)
						{
							int numberOfMsg=outputBlock.readInteger();
							D.pushOutData(CLuaFunctionDataItem(numberOfMsg));
						}

						D.writeDataToLua(p);
					}
					else
						simSetLastError(LUA_INITIALIZE_COMMAND,"Received a bad reply from the server.");
				}
				else
					simSetLastError(LUA_INITIALIZE_COMMAND,"Failed receiving a reply from the server.");
			}
			else
				simSetLastError(LUA_INITIALIZE_COMMAND,"Failed sending data to the server.");
		}
		else
			simSetLastError(LUA_INITIALIZE_COMMAND,"There is no RCS server currently selected for this script.");

	}
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// RESET
// --------------------------------------------------------------------------------------
#define LUA_RESET_COMMAND "simExtRRS1_RESET"
const int inArgs_RESET[]={
	2,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
	sim_lua_arg_int,0,
};

void LUA_RESET_CALLBACK(SLuaCallBack* p)
{
	p->outputArgCount=0;
	CLuaFunctionData D;
	if (D.readDataFromLua(p,inArgs_RESET,inArgs_RESET[0],LUA_RESET_COMMAND))
	{
		std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
		int scriptId=p->scriptID;
		int index=getServerIndexFromScriptId(scriptId);
		if (index!=-1)
		{
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(102,rcsHandle.c_str());
			inputBlock.pushBackInteger(inData->at(1).intData[0]);

			int inputBlockSize;
			void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
			if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
			{
				int outputBlockSize;
				char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
				if (data!=0)
				{
					CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
					delete[] data;
					if (outputBlockSize>=4)
					{
						int s=outputBlock.readInteger();
						if (s>=8)
						{
							int status=outputBlock.readInteger();
							D.pushOutData(CLuaFunctionDataItem(status));
						}
						if (s>=12)
						{
							int numberOfMsg=outputBlock.readInteger();
							D.pushOutData(CLuaFunctionDataItem(numberOfMsg));
						}

						D.writeDataToLua(p);
					}
					else
						simSetLastError(LUA_RESET_COMMAND,"Received a bad reply from the server.");
				}
				else
					simSetLastError(LUA_RESET_COMMAND,"Failed receiving a reply from the server.");
			}
			else
				simSetLastError(LUA_RESET_COMMAND,"Failed sending data to the server.");
		}
		else
			simSetLastError(LUA_RESET_COMMAND,"There is no RCS server currently selected for this script.");

	}
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// TERMINATE
// --------------------------------------------------------------------------------------
#define LUA_TERMINATE_COMMAND "simExtRRS1_TERMINATE"

const int inArgs_TERMINATE[]={
	1,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
};

void LUA_TERMINATE_CALLBACK(SLuaCallBack* p)
{
	p->outputArgCount=0;
	CLuaFunctionData D;
	if (D.readDataFromLua(p,inArgs_TERMINATE,inArgs_TERMINATE[0],LUA_TERMINATE_COMMAND))
	{
		std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
		int scriptId=p->scriptID;
		int index=getServerIndexFromScriptId(scriptId);
		if (index!=-1)
		{
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(103,rcsHandle.c_str());

			int inputBlockSize;
			void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
			if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
			{
				int outputBlockSize;
				char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
				if (data!=0)
				{
					CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
					delete[] data;
					if (outputBlockSize>=4)
					{
						int s=outputBlock.readInteger();
						if (s>=8)
						{
							int status=outputBlock.readInteger();
							D.pushOutData(CLuaFunctionDataItem(status));
						}

						D.writeDataToLua(p);
					}
					else
						simSetLastError(LUA_TERMINATE_COMMAND,"Received a bad reply from the server.");
				}
				else
					simSetLastError(LUA_TERMINATE_COMMAND,"Failed receiving a reply from the server.");
			}
			else
				simSetLastError(LUA_TERMINATE_COMMAND,"Failed sending data to the server.");
		}
		else
			simSetLastError(LUA_TERMINATE_COMMAND,"There is no RCS server currently selected for this script.");

	}
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// GET_ROBOT_STAMP
// --------------------------------------------------------------------------------------
#define LUA_GET_ROBOT_STAMP_COMMAND "simExtRRS1_GET_ROBOT_STAMP"
const int inArgs_GET_ROBOT_STAMP[]={
    1,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
};

void LUA_GET_ROBOT_STAMP_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_GET_ROBOT_STAMP,inArgs_GET_ROBOT_STAMP[0],LUA_GET_ROBOT_STAMP_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(104,rcsHandle.c_str());

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }
                        if (s>=12)
                        {
                            std::string manipulator=outputBlock.readString();
                            D.pushOutData(CLuaFunctionDataItem(manipulator));
                            std::string controller=outputBlock.readString();
                            D.pushOutData(CLuaFunctionDataItem(controller));
                            std::string software=outputBlock.readString();
                            D.pushOutData(CLuaFunctionDataItem(software));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_GET_ROBOT_STAMP_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_GET_ROBOT_STAMP_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_GET_ROBOT_STAMP_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_GET_ROBOT_STAMP_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// GET_HOME_JOINT_POSITION
// --------------------------------------------------------------------------------------
#define LUA_GET_HOME_JOINT_POSITION_COMMAND "simExtRRS1_GET_HOME_JOINT_POSITION"
const int inArgs_GET_HOME_JOINT_POSITION[]={
    1,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
};

void LUA_GET_HOME_JOINT_POSITION_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_GET_HOME_JOINT_POSITION,inArgs_GET_HOME_JOINT_POSITION[0],LUA_GET_HOME_JOINT_POSITION_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(166,rcsHandle.c_str());

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }
                        if (s>=12)
                        {
                            std::string jointPos(outputBlock.readJointPos());
							D.pushOutData(CLuaFunctionDataItem(&jointPos[0],(int)jointPos.size()));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_GET_HOME_JOINT_POSITION_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_GET_HOME_JOINT_POSITION_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_GET_HOME_JOINT_POSITION_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_GET_HOME_JOINT_POSITION_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// GET_RCS_DATA
// --------------------------------------------------------------------------------------
#define LUA_GET_RCS_DATA_COMMAND "simExtRRS1_GET_RCS_DATA"
const int inArgs_GET_RCS_DATA[]={
    4,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_int,0,
    sim_lua_arg_int,0,
    sim_lua_arg_string,0,
};

void LUA_GET_RCS_DATA_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_GET_RCS_DATA,inArgs_GET_RCS_DATA[0],LUA_GET_RCS_DATA_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(105,rcsHandle.c_str());
            inputBlock.pushBackInteger(inData->at(1).intData[0]);
            inputBlock.pushBackInteger(inData->at(2).intData[0]);
            inputBlock.pushBackString(inData->at(3).stringData[0]);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }
                        if (s>=12)
                        {
                            std::string paramId=outputBlock.readString();
                            D.pushOutData(CLuaFunctionDataItem(paramId));

                            std::string paramContents=outputBlock.readString();
                            D.pushOutData(CLuaFunctionDataItem(paramContents));

                            int permissions=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(permissions));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_GET_RCS_DATA_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_GET_RCS_DATA_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_GET_RCS_DATA_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_GET_RCS_DATA_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// MODIFY_RCS_DATA
// --------------------------------------------------------------------------------------
#define LUA_MODIFY_RCS_DATA_COMMAND "simExtRRS1_MODIFY_RCS_DATA"
const int inArgs_MODIFY_RCS_DATA[]={
    4,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_int,0,
    sim_lua_arg_string,0,
    sim_lua_arg_string,0,
};

void LUA_MODIFY_RCS_DATA_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_MODIFY_RCS_DATA,inArgs_MODIFY_RCS_DATA[0],LUA_MODIFY_RCS_DATA_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(106,rcsHandle.c_str());
            inputBlock.pushBackInteger(inData->at(1).intData[0]);
            inputBlock.pushBackString(inData->at(2).stringData[0]);
            inputBlock.pushBackString(inData->at(3).stringData[0]);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_MODIFY_RCS_DATA_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_MODIFY_RCS_DATA_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_MODIFY_RCS_DATA_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_MODIFY_RCS_DATA_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SAVE_RCS_DATA
// --------------------------------------------------------------------------------------
#define LUA_SAVE_RCS_DATA_COMMAND "simExtRRS1_SAVE_RCS_DATA"
const int inArgs_SAVE_RCS_DATA[]={
    1,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
};

void LUA_SAVE_RCS_DATA_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_SAVE_RCS_DATA,inArgs_SAVE_RCS_DATA[0],LUA_SAVE_RCS_DATA_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(107,rcsHandle.c_str());

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SAVE_RCS_DATA_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SAVE_RCS_DATA_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SAVE_RCS_DATA_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SAVE_RCS_DATA_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// LOAD_RCS_DATA
// --------------------------------------------------------------------------------------
#define LUA_LOAD_RCS_DATA_COMMAND "simExtRRS1_LOAD_RCS_DATA"
const int inArgs_LOAD_RCS_DATA[]={
    1,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
};

void LUA_LOAD_RCS_DATA_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_LOAD_RCS_DATA,inArgs_LOAD_RCS_DATA[0],LUA_LOAD_RCS_DATA_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(108,rcsHandle.c_str());

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }
                        if (s>=12)
                        {
                            int msgNb=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(msgNb));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_LOAD_RCS_DATA_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_LOAD_RCS_DATA_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_LOAD_RCS_DATA_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_LOAD_RCS_DATA_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// GET_INVERSE_KINEMATIC
// --------------------------------------------------------------------------------------
#define LUA_GET_INVERSE_KINEMATIC_COMMAND "simExtRRS1_GET_INVERSE_KINEMATIC"
const int inArgs_GET_INVERSE_KINEMATIC[]={
    5,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_charbuff,CARTPOS_SIZE,
    sim_lua_arg_charbuff,JOINTPOS_SIZE,
    sim_lua_arg_string,0,
    sim_lua_arg_charbuff,BITSTRING_SIZE,
};

void LUA_GET_INVERSE_KINEMATIC_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_GET_INVERSE_KINEMATIC,inArgs_GET_INVERSE_KINEMATIC[0],LUA_GET_INVERSE_KINEMATIC_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(109,rcsHandle.c_str());

            std::string cartPos(getPartialString(inData->at(1).stringData[0],CARTPOS_SIZE));
            inputBlock.pushBackCartPos(cartPos);

            std::string jointPos(getPartialString(inData->at(2).stringData[0],JOINTPOS_SIZE));
            inputBlock.pushBackJointPos(jointPos);

            inputBlock.pushBackString(inData->at(3).stringData[0]);

            std::string outputFormat(getPartialString(inData->at(4).stringData[0],BITSTRING_SIZE));
			inputBlock.pushBackBitstring(outputFormat);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }
                        if (s>=12)
                        {
                            std::string jointPosOut(outputBlock.readJointPos());
                            D.pushOutData(CLuaFunctionDataItem(&jointPosOut[0],(int)jointPosOut.size()));

                            std::string jointLimits=outputBlock.readBitstring();
                            D.pushOutData(CLuaFunctionDataItem(&jointLimits[0],(int)jointLimits.size()));

                            int msgNb=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(msgNb));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_GET_INVERSE_KINEMATIC_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_GET_INVERSE_KINEMATIC_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_GET_INVERSE_KINEMATIC_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_GET_INVERSE_KINEMATIC_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// GET_FORWARD_KINEMATIC
// --------------------------------------------------------------------------------------
#define LUA_GET_FORWARD_KINEMATIC_COMMAND "simExtRRS1_GET_FORWARD_KINEMATIC"
const int inArgs_GET_FORWARD_KINEMATIC[]={
    2,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_charbuff,JOINTPOS_SIZE,
};

void LUA_GET_FORWARD_KINEMATIC_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_GET_FORWARD_KINEMATIC,inArgs_GET_FORWARD_KINEMATIC[0],LUA_GET_FORWARD_KINEMATIC_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(110,rcsHandle.c_str());

            std::string jointPos(getPartialString(inData->at(1).stringData[0],JOINTPOS_SIZE));
            inputBlock.pushBackJointPos(jointPos);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }
                        if (s>=12)
                        {
                            std::string cartPosOut(outputBlock.readCartPos());
                            D.pushOutData(CLuaFunctionDataItem(&cartPosOut[0],(int)cartPosOut.size()));

                            std::string jointPosOut(outputBlock.readJointPos());
                            D.pushOutData(CLuaFunctionDataItem(&jointPosOut[0],(int)jointPosOut.size()));

                            std::string configuration=outputBlock.readString();
                            D.pushOutData(CLuaFunctionDataItem(configuration));

                            std::string jointLimits=outputBlock.readBitstring();
                            D.pushOutData(CLuaFunctionDataItem(&jointLimits[0],(int)jointLimits.size()));

                            int msgNb=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(msgNb));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_GET_FORWARD_KINEMATIC_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_GET_FORWARD_KINEMATIC_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_GET_FORWARD_KINEMATIC_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_GET_FORWARD_KINEMATIC_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// MATRIX_TO_CONTROLLER_POSITION
// --------------------------------------------------------------------------------------
#define LUA_MATRIX_TO_CONTROLLER_POSITION_COMMAND "simExtRRS1_MATRIX_TO_CONTROLLER_POSITION"
const int inArgs_MATRIX_TO_CONTROLLER_POSITION[]={
    3,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_charbuff,CARTPOS_SIZE,
    sim_lua_arg_string,0,
};

void LUA_MATRIX_TO_CONTROLLER_POSITION_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_MATRIX_TO_CONTROLLER_POSITION,inArgs_MATRIX_TO_CONTROLLER_POSITION[0],LUA_MATRIX_TO_CONTROLLER_POSITION_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(111,rcsHandle.c_str());

            std::string cartPos(getPartialString(inData->at(1).stringData[0],CARTPOS_SIZE));
            inputBlock.pushBackCartPos(cartPos);

            inputBlock.pushBackString(inData->at(2).stringData[0]);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }
                        if (s>=12)
                        {
                            std::string contrPos=outputBlock.readString();
                            D.pushOutData(CLuaFunctionDataItem(contrPos));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_MATRIX_TO_CONTROLLER_POSITION_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_MATRIX_TO_CONTROLLER_POSITION_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_MATRIX_TO_CONTROLLER_POSITION_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_MATRIX_TO_CONTROLLER_POSITION_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// CONTROLLER_POSITION_TO_MATRIX
// --------------------------------------------------------------------------------------
#define LUA_CONTROLLER_POSITION_TO_MATRIX_COMMAND "simExtRRS1_CONTROLLER_POSITION_TO_MATRIX"
const int inArgs_CONTROLLER_POSITION_TO_MATRIX[]={
    2,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_string,0,
};

void LUA_CONTROLLER_POSITION_TO_MATRIX_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_CONTROLLER_POSITION_TO_MATRIX,inArgs_CONTROLLER_POSITION_TO_MATRIX[0],LUA_CONTROLLER_POSITION_TO_MATRIX_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(112,rcsHandle.c_str());

            inputBlock.pushBackString(inData->at(1).stringData[0]);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }
                        if (s>=12)
                        {
                            std::string cartPos=outputBlock.readCartPos();
                            D.pushOutData(CLuaFunctionDataItem(&cartPos[0],(int)cartPos.size()));

                            std::string configuration=outputBlock.readString();
                            D.pushOutData(CLuaFunctionDataItem(configuration));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_CONTROLLER_POSITION_TO_MATRIX_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_CONTROLLER_POSITION_TO_MATRIX_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_CONTROLLER_POSITION_TO_MATRIX_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_CONTROLLER_POSITION_TO_MATRIX_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// GET_CELL_FRAME
// --------------------------------------------------------------------------------------
#define LUA_GET_CELL_FRAME_COMMAND "simExtRRS1_GET_CELL_FRAME"
const int inArgs_GET_CELL_FRAME[]={
    4,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_int,0,
    sim_lua_arg_int,0,
    sim_lua_arg_string,0,
};

void LUA_GET_CELL_FRAME_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_GET_CELL_FRAME,inArgs_GET_CELL_FRAME[0],LUA_GET_CELL_FRAME_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(113,rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            inputBlock.pushBackInteger(inData->at(2).intData[0]);

            inputBlock.pushBackString(inData->at(3).stringData[0]);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }
                        if (s>=12)
                        {
                            std::string frameId=outputBlock.readString();
                            D.pushOutData(CLuaFunctionDataItem(frameId));

                            int frameType=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(frameType));

                            std::string relativeToId=outputBlock.readString();
                            D.pushOutData(CLuaFunctionDataItem(relativeToId));

                            std::string jointNumber=outputBlock.readBitstring();
                            D.pushOutData(CLuaFunctionDataItem(&jointNumber[0],(int)jointNumber.size()));

                            std::string frameData=outputBlock.readFrame();
                            D.pushOutData(CLuaFunctionDataItem(&frameData[0],(int)frameData.size()));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_GET_CELL_FRAME_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_GET_CELL_FRAME_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_GET_CELL_FRAME_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_GET_CELL_FRAME_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// MODIFY_CELL_FRAME
// --------------------------------------------------------------------------------------
#define LUA_MODIFY_CELL_FRAME_COMMAND "simExtRRS1_MODIFY_CELL_FRAME"
const int inArgs_MODIFY_CELL_FRAME[]={
    4,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_int,0,
    sim_lua_arg_string,0,
    sim_lua_arg_charbuff,FRAME_SIZE,
};

void LUA_MODIFY_CELL_FRAME_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_MODIFY_CELL_FRAME,inArgs_MODIFY_CELL_FRAME[0],LUA_MODIFY_CELL_FRAME_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(114,rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            inputBlock.pushBackString(inData->at(2).stringData[0]);

            std::string frameData(getPartialString(inData->at(3).stringData[0],FRAME_SIZE));
            inputBlock.pushBackFrame(frameData);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_MODIFY_CELL_FRAME_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_MODIFY_CELL_FRAME_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_MODIFY_CELL_FRAME_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_MODIFY_CELL_FRAME_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SELECT_WORK_FRAMES
// --------------------------------------------------------------------------------------
#define LUA_SELECT_WORK_FRAMES_COMMAND "simExtRRS1_SELECT_WORK_FRAMES"
const int inArgs_SELECT_WORK_FRAMES[]={
    3,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_string,0,
    sim_lua_arg_string,0,
};

void LUA_SELECT_WORK_FRAMES_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_SELECT_WORK_FRAMES,inArgs_SELECT_WORK_FRAMES[0],LUA_SELECT_WORK_FRAMES_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(115,rcsHandle.c_str());

            inputBlock.pushBackString(inData->at(1).stringData[0]);

            inputBlock.pushBackString(inData->at(2).stringData[0]);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SELECT_WORK_FRAMES_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SELECT_WORK_FRAMES_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SELECT_WORK_FRAMES_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SELECT_WORK_FRAMES_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_INITIAL_POSITION
// --------------------------------------------------------------------------------------
#define LUA_SET_INITIAL_POSITION_COMMAND "simExtRRS1_SET_INITIAL_POSITION"
const int inArgs_SET_INITIAL_POSITION[]={
    4,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_charbuff,CARTPOS_SIZE,
    sim_lua_arg_charbuff,JOINTPOS_SIZE,
    sim_lua_arg_string,0,
};

void LUA_SET_INITIAL_POSITION_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_SET_INITIAL_POSITION,inArgs_SET_INITIAL_POSITION[0],LUA_SET_INITIAL_POSITION_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(116,rcsHandle.c_str());

            std::string cartPosData(getPartialString(inData->at(1).stringData[0],CARTPOS_SIZE));
            inputBlock.pushBackCartPos(cartPosData);

            std::string jointPosData(getPartialString(inData->at(2).stringData[0],JOINTPOS_SIZE));
            inputBlock.pushBackJointPos(jointPosData);

            inputBlock.pushBackString(inData->at(3).stringData[0]);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }
                        if (s>=12)
                        {
                            std::string jointLimit=outputBlock.readBitstring();
                            D.pushOutData(CLuaFunctionDataItem(&jointLimit[0],(int)jointLimit.size()));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_INITIAL_POSITION_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_INITIAL_POSITION_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_INITIAL_POSITION_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_INITIAL_POSITION_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_NEXT_TARGET
// --------------------------------------------------------------------------------------
#define LUA_SET_NEXT_TARGET_COMMAND "simExtRRS1_SET_NEXT_TARGET"
const int inArgs_SET_NEXT_TARGET[]={
    7,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_int,0,
    sim_lua_arg_int,0,
    sim_lua_arg_charbuff,CARTPOS_SIZE,
    sim_lua_arg_charbuff,JOINTPOS_SIZE,
    sim_lua_arg_string,0,
    sim_lua_arg_double,0,
};

void LUA_SET_NEXT_TARGET_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_SET_NEXT_TARGET,inArgs_SET_NEXT_TARGET[0],LUA_SET_NEXT_TARGET_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(117,rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            inputBlock.pushBackInteger(inData->at(2).intData[0]);

            std::string cartPosData(getPartialString(inData->at(3).stringData[0],CARTPOS_SIZE));
            inputBlock.pushBackCartPos(cartPosData);

            std::string jointPosData(getPartialString(inData->at(4).stringData[0],JOINTPOS_SIZE));
            inputBlock.pushBackJointPos(jointPosData);

            inputBlock.pushBackString(inData->at(5).stringData[0]);

            inputBlock.pushBackReal(inData->at(6).doubleData[0]);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_NEXT_TARGET_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_NEXT_TARGET_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_NEXT_TARGET_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_NEXT_TARGET_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// GET_NEXT_STEP
// --------------------------------------------------------------------------------------
#define LUA_GET_NEXT_STEP_COMMAND "simExtRRS1_GET_NEXT_STEP"
const int inArgs_GET_NEXT_STEP[]={
    2,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_charbuff,BITSTRING_SIZE,
};

void LUA_GET_NEXT_STEP_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_GET_NEXT_STEP,inArgs_GET_NEXT_STEP[0],LUA_GET_NEXT_STEP_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
			std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(118,rcsHandle.c_str());

            std::string outputFormat(getPartialString(inData->at(1).stringData[0],BITSTRING_SIZE));
            inputBlock.pushBackBitstring(outputFormat);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
               int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                       int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }
                        if (s>=12)
                        {
                            std::string cartPos=outputBlock.readCartPos();
                            D.pushOutData(CLuaFunctionDataItem(&cartPos[0],(int)cartPos.size()));

                            std::string jointPos=outputBlock.readJointPos();
                            D.pushOutData(CLuaFunctionDataItem(&jointPos[0],(int)jointPos.size()));

                            std::string configuration=outputBlock.readString();
                            D.pushOutData(CLuaFunctionDataItem(configuration));

                            double elapsedTime=outputBlock.readReal();
                            D.pushOutData(CLuaFunctionDataItem(elapsedTime));

                            std::string jointLimit=outputBlock.readBitstring();
                            D.pushOutData(CLuaFunctionDataItem(&jointLimit[0],(int)jointLimit.size()));

                            int numberOfEvents=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(numberOfEvents));

                            int numberOfMessages=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(numberOfMessages));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_GET_NEXT_STEP_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_GET_NEXT_STEP_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_GET_NEXT_STEP_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_GET_NEXT_STEP_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_INTERPOLATION_TIME
// --------------------------------------------------------------------------------------
#define LUA_SET_INTERPOLATION_TIME_COMMAND "simExtRRS1_SET_INTERPOLATION_TIME"
const int inArgs_SET_INTERPOLATION_TIME[]={
    2,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_double,0,
};

void LUA_SET_INTERPOLATION_TIME_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_SET_INTERPOLATION_TIME,inArgs_SET_INTERPOLATION_TIME[0],LUA_SET_INTERPOLATION_TIME_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(119,rcsHandle.c_str());

            inputBlock.pushBackReal(inData->at(1).doubleData[0]);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_INTERPOLATION_TIME_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_INTERPOLATION_TIME_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_INTERPOLATION_TIME_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_INTERPOLATION_TIME_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SELECT_MOTION_TYPE
// --------------------------------------------------------------------------------------
#define LUA_SELECT_MOTION_TYPE_COMMAND "simExtRRS1_SELECT_MOTION_TYPE"
const int inArgs_SELECT_MOTION_TYPE[]={
    2,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_int,0,
};

void LUA_SELECT_MOTION_TYPE_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_SELECT_MOTION_TYPE,inArgs_SELECT_MOTION_TYPE[0],LUA_SELECT_MOTION_TYPE_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(120,rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SELECT_MOTION_TYPE_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SELECT_MOTION_TYPE_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SELECT_MOTION_TYPE_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SELECT_MOTION_TYPE_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SELECT_TARGET_TYPE
// --------------------------------------------------------------------------------------
#define LUA_SELECT_TARGET_TYPE_COMMAND "simExtRRS1_SELECT_TARGET_TYPE"
const int inArgs_SELECT_TARGET_TYPE[]={
    5,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_int,0,
    sim_lua_arg_charbuff,CARTPOS_SIZE,
    sim_lua_arg_charbuff,JOINTPOS_SIZE,
    sim_lua_arg_string,0,
};

void LUA_SELECT_TARGET_TYPE_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_SELECT_TARGET_TYPE,inArgs_SELECT_TARGET_TYPE[0],LUA_SELECT_TARGET_TYPE_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(121,rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            std::string cartPosData(getPartialString(inData->at(2).stringData[0],CARTPOS_SIZE));
            inputBlock.pushBackCartPos(cartPosData);

            std::string jointPosData(getPartialString(inData->at(3).stringData[0],JOINTPOS_SIZE));
            inputBlock.pushBackJointPos(jointPosData);

            inputBlock.pushBackString(inData->at(4).stringData[0]);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SELECT_TARGET_TYPE_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SELECT_TARGET_TYPE_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SELECT_TARGET_TYPE_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SELECT_TARGET_TYPE_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SELECT_TRAJECTORY_MODE
// --------------------------------------------------------------------------------------
#define LUA_SELECT_TRAJECTORY_MODE_COMMAND "simExtRRS1_SELECT_TRAJECTORY_MODE"
const int inArgs_SELECT_TRAJECTORY_MODE[]={
    2,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_int,0,
};

void LUA_SELECT_TRAJECTORY_MODE_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_SELECT_TRAJECTORY_MODE,inArgs_SELECT_TRAJECTORY_MODE[0],LUA_SELECT_TRAJECTORY_MODE_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(122,rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SELECT_TRAJECTORY_MODE_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SELECT_TRAJECTORY_MODE_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SELECT_TRAJECTORY_MODE_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SELECT_TRAJECTORY_MODE_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SELECT_ORIENTATION_INTERPOLATION_MODE
// --------------------------------------------------------------------------------------
#define LUA_SELECT_ORIENTATION_INTERPOLATION_MODE_COMMAND "simExtRRS1_SELECT_ORIENTATION_INTERPOLATION_MODE"
const int inArgs_SELECT_ORIENTATION_INTERPOLATION_MODE[]={
    3,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_int,0,
    sim_lua_arg_int,0,
};

void LUA_SELECT_ORIENTATION_INTERPOLATION_MODE_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_SELECT_ORIENTATION_INTERPOLATION_MODE,inArgs_SELECT_ORIENTATION_INTERPOLATION_MODE[0],LUA_SELECT_ORIENTATION_INTERPOLATION_MODE_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(123,rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            inputBlock.pushBackInteger(inData->at(2).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SELECT_ORIENTATION_INTERPOLATION_MODE_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SELECT_ORIENTATION_INTERPOLATION_MODE_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SELECT_ORIENTATION_INTERPOLATION_MODE_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SELECT_ORIENTATION_INTERPOLATION_MODE_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SELECT_DOMINANT_INTERPOLATION
// --------------------------------------------------------------------------------------
#define LUA_SELECT_DOMINANT_INTERPOLATION_COMMAND "simExtRRS1_SELECT_DOMINANT_INTERPOLATION"
const int inArgs_SELECT_DOMINANT_INTERPOLATION[]={
    3,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_int,0,
    sim_lua_arg_int,0,
};

void LUA_SELECT_DOMINANT_INTERPOLATION_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_SELECT_DOMINANT_INTERPOLATION,inArgs_SELECT_DOMINANT_INTERPOLATION[0],LUA_SELECT_DOMINANT_INTERPOLATION_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(124,rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            inputBlock.pushBackInteger(inData->at(2).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SELECT_DOMINANT_INTERPOLATION_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SELECT_DOMINANT_INTERPOLATION_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SELECT_DOMINANT_INTERPOLATION_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SELECT_DOMINANT_INTERPOLATION_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_ADVANCE_MOTION
// --------------------------------------------------------------------------------------
#define LUA_SET_ADVANCE_MOTION_COMMAND "simExtRRS1_SET_ADVANCE_MOTION"
const int inArgs_SET_ADVANCE_MOTION[]={
    2,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_int,0,
};

void LUA_SET_ADVANCE_MOTION_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_SET_ADVANCE_MOTION,inArgs_SET_ADVANCE_MOTION[0],LUA_SET_ADVANCE_MOTION_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(127,rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_ADVANCE_MOTION_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_ADVANCE_MOTION_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_ADVANCE_MOTION_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_ADVANCE_MOTION_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_MOTION_FILTER
// --------------------------------------------------------------------------------------
#define LUA_SET_MOTION_FILTER_COMMAND "simExtRRS1_SET_MOTION_FILTER"
const int inArgs_SET_MOTION_FILTER[]={
    2,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_int,0,
};

void LUA_SET_MOTION_FILTER_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_SET_MOTION_FILTER,inArgs_SET_MOTION_FILTER[0],LUA_SET_MOTION_FILTER_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(128,rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_MOTION_FILTER_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_MOTION_FILTER_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_MOTION_FILTER_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_MOTION_FILTER_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_OVERRIDE_POSITION
// --------------------------------------------------------------------------------------
#define LUA_SET_OVERRIDE_POSITION_COMMAND "simExtRRS1_SET_OVERRIDE_POSITION"
const int inArgs_SET_OVERRIDE_POSITION[]={
    2,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_charbuff,FRAME_SIZE,
};

void LUA_SET_OVERRIDE_POSITION_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_SET_OVERRIDE_POSITION,inArgs_SET_OVERRIDE_POSITION[0],LUA_SET_OVERRIDE_POSITION_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(129,rcsHandle.c_str());

            std::string posOffset(getPartialString(inData->at(1).stringData[0],FRAME_SIZE));
            inputBlock.pushBackFrame(posOffset);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_OVERRIDE_POSITION_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_OVERRIDE_POSITION_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_OVERRIDE_POSITION_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_OVERRIDE_POSITION_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// REVERSE_MOTION
// --------------------------------------------------------------------------------------
#define LUA_REVERSE_MOTION_COMMAND "simExtRRS1_REVERSE_MOTION"
const int inArgs_REVERSE_MOTION[]={
    2,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_double,0,
};

void LUA_REVERSE_MOTION_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_REVERSE_MOTION,inArgs_REVERSE_MOTION[0],LUA_REVERSE_MOTION_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(130,rcsHandle.c_str());

            inputBlock.pushBackReal(inData->at(1).doubleData[0]);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_REVERSE_MOTION_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_REVERSE_MOTION_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_REVERSE_MOTION_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_REVERSE_MOTION_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_PAYLOAD_PARAMETER
// --------------------------------------------------------------------------------------
#define LUA_SET_PAYLOAD_PARAMETER_COMMAND "simExtRRS1_SET_PAYLOAD_PARAMETER"
const int inArgs_SET_PAYLOAD_PARAMETER[]={
    5,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_int,0,
    sim_lua_arg_string,0,
    sim_lua_arg_int,0,
    sim_lua_arg_double,0,
};

void LUA_SET_PAYLOAD_PARAMETER_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_SET_PAYLOAD_PARAMETER,inArgs_SET_PAYLOAD_PARAMETER[0],LUA_SET_PAYLOAD_PARAMETER_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(160,rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            inputBlock.pushBackString(inData->at(2).stringData[0]);

            inputBlock.pushBackInteger(inData->at(3).intData[0]);

            inputBlock.pushBackReal(inData->at(4).doubleData[0]);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_PAYLOAD_PARAMETER_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_PAYLOAD_PARAMETER_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_PAYLOAD_PARAMETER_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_PAYLOAD_PARAMETER_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SELECT_TIME_COMPENSATION
// --------------------------------------------------------------------------------------
#define LUA_SELECT_TIME_COMPENSATION_COMMAND "simExtRRS1_SELECT_TIME_COMPENSATION"
const int inArgs_SELECT_TIME_COMPENSATION[]={
    2,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_charbuff,BITSTRING_SIZE,
};

void LUA_SELECT_TIME_COMPENSATION_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_SELECT_TIME_COMPENSATION,inArgs_SELECT_TIME_COMPENSATION[0],LUA_SELECT_TIME_COMPENSATION_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(165,rcsHandle.c_str());

            inputBlock.pushBackBitstring(inData->at(1).stringData[0]);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SELECT_TIME_COMPENSATION_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SELECT_TIME_COMPENSATION_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SELECT_TIME_COMPENSATION_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SELECT_TIME_COMPENSATION_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_CONFIGURATION_CONTROL
// --------------------------------------------------------------------------------------
#define LUA_SET_CONFIGURATION_CONTROL_COMMAND "simExtRRS1_SET_CONFIGURATION_CONTROL"
const int inArgs_SET_CONFIGURATION_CONTROL[]={
    3,
    sim_lua_arg_charbuff,BITSTRING2_SIZE,
    sim_lua_arg_string,0,
    sim_lua_arg_string,0,
};

void LUA_SET_CONFIGURATION_CONTROL_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount=0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p,inArgs_SET_CONFIGURATION_CONTROL,inArgs_SET_CONFIGURATION_CONTROL[0],LUA_SET_CONFIGURATION_CONTROL_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData=D.getInDataPtr();
        int scriptId=p->scriptID;
        int index=getServerIndexFromScriptId(scriptId);
        if (index!=-1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0],BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(161,rcsHandle.c_str());

            inputBlock.pushBackString(inData->at(1).stringData[0]);

            inputBlock.pushBackString(inData->at(2).stringData[0]);

            int inputBlockSize;
            void* inputBlockPointer=inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer,inputBlockSize))
            {
                int outputBlockSize;
                char* data=allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data!=0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data,outputBlockSize);
                    delete[] data;
                    if (outputBlockSize>=4)
                    {
                        int s=outputBlock.readInteger();
                        if (s>=8)
                        {
                            int status=outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_CONFIGURATION_CONTROL_COMMAND,"Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_CONFIGURATION_CONTROL_COMMAND,"Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_CONFIGURATION_CONTROL_COMMAND,"Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_CONFIGURATION_CONTROL_COMMAND,"There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_JOINT_SPEEDS
// --------------------------------------------------------------------------------------
#define LUA_SET_JOINT_SPEEDS_COMMAND "simExtRRS1_SET_JOINT_SPEEDS"
const int inArgs_SET_JOINT_SPEEDS[] = {
    4,
	sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_int, 0,
    sim_lua_arg_charbuff, BITSTRING_SIZE,
    sim_lua_arg_double|sim_lua_arg_table, 32,
};

void LUA_SET_JOINT_SPEEDS_CALLBACK(SLuaCallBack* p)
{
	p->outputArgCount = 0;
	CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SET_JOINT_SPEEDS, inArgs_SET_JOINT_SPEEDS[0], LUA_SET_JOINT_SPEEDS_COMMAND))
	{
		std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
		int scriptId = p->scriptID;
		int index = getServerIndexFromScriptId(scriptId);
		if (index != -1)
		{
			std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(131, rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            inputBlock.pushBackBitstring(inData->at(2).stringData[0]);

            for (unsigned int i=0;i<32;i++)
                inputBlock.pushBackReal(inData->at(3).doubleData[i]);

			int inputBlockSize;
			void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
			if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
			{
				int outputBlockSize;
				char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
				if (data != 0)
				{
					CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
					delete[] data;
					if (outputBlockSize >= 4)
					{
						int s = outputBlock.readInteger();
						if (s >= 8)
						{
							int status = outputBlock.readInteger();
							D.pushOutData(CLuaFunctionDataItem(status));
						}

						D.writeDataToLua(p);
					}
					else
                        simSetLastError(LUA_SET_JOINT_SPEEDS_COMMAND, "Received a bad reply from the server.");
				}
				else
                    simSetLastError(LUA_SET_JOINT_SPEEDS_COMMAND, "Failed receiving a reply from the server.");
			}
			else
                simSetLastError(LUA_SET_JOINT_SPEEDS_COMMAND, "Failed sending data to the server.");
		}
		else
            simSetLastError(LUA_SET_JOINT_SPEEDS_COMMAND, "There is no RCS server currently selected for this script.");

	}
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_CARTESIAN_POSITION_SPEED
// --------------------------------------------------------------------------------------
#define LUA_SET_CARTESIAN_POSITION_SPEED_COMMAND "simExtRRS1_SET_CARTESIAN_POSITION_SPEED"
const int inArgs_SET_CARTESIAN_POSITION_SPEED[] = {
    2,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_double, 0,
};

void LUA_SET_CARTESIAN_POSITION_SPEED_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SET_CARTESIAN_POSITION_SPEED, inArgs_SET_CARTESIAN_POSITION_SPEED[0], LUA_SET_CARTESIAN_POSITION_SPEED_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(133, rcsHandle.c_str());

            inputBlock.pushBackReal(inData->at(1).doubleData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_CARTESIAN_POSITION_SPEED_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_CARTESIAN_POSITION_SPEED_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_CARTESIAN_POSITION_SPEED_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_CARTESIAN_POSITION_SPEED_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_CARTESIAN_ORIENTATION_SPEED
// --------------------------------------------------------------------------------------
#define LUA_SET_CARTESIAN_ORIENTATION_SPEED_COMMAND "simExtRRS1_SET_CARTESIAN_ORIENTATION_SPEED"
const int inArgs_SET_CARTESIAN_ORIENTATION_SPEED[] = {
    3,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_int, 0,
    sim_lua_arg_double, 0,
};

void LUA_SET_CARTESIAN_ORIENTATION_SPEED_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SET_CARTESIAN_ORIENTATION_SPEED, inArgs_SET_CARTESIAN_ORIENTATION_SPEED[0], LUA_SET_CARTESIAN_ORIENTATION_SPEED_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(134, rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            inputBlock.pushBackReal(inData->at(2).doubleData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_CARTESIAN_ORIENTATION_SPEED_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_CARTESIAN_ORIENTATION_SPEED_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_CARTESIAN_ORIENTATION_SPEED_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_CARTESIAN_ORIENTATION_SPEED_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_JOINT_ACCELERATIONS
// --------------------------------------------------------------------------------------
#define LUA_SET_JOINT_ACCELERATIONS_COMMAND "simExtRRS1_SET_JOINT_ACCELERATIONS"
const int inArgs_SET_JOINT_ACCELERATIONS[] = {
    5,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_int, 0,
    sim_lua_arg_charbuff, BITSTRING_SIZE,
    sim_lua_arg_double|sim_lua_arg_table, 32,
    sim_lua_arg_int, 0,
};

void LUA_SET_JOINT_ACCELERATIONS_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SET_JOINT_ACCELERATIONS, inArgs_SET_JOINT_ACCELERATIONS[0], LUA_SET_JOINT_ACCELERATIONS_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(135, rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            inputBlock.pushBackBitstring(inData->at(2).stringData[0]);

            for (unsigned int i=0;i<32;i++)
                inputBlock.pushBackReal(inData->at(3).doubleData[i]);

            inputBlock.pushBackInteger(inData->at(4).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_JOINT_ACCELERATIONS_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_JOINT_ACCELERATIONS_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_JOINT_ACCELERATIONS_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_JOINT_ACCELERATIONS_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_CARTESIAN_POSITION_ACCELERATION
// --------------------------------------------------------------------------------------
#define LUA_SET_CARTESIAN_POSITION_ACCELERATION_COMMAND "simExtRRS1_SET_CARTESIAN_POSITION_ACCELERATION"
const int inArgs_SET_CARTESIAN_POSITION_ACCELERATION[] = {
    3,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_double, 0,
    sim_lua_arg_int, 0,
};

void LUA_SET_CARTESIAN_POSITION_ACCELERATION_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SET_CARTESIAN_POSITION_ACCELERATION, inArgs_SET_CARTESIAN_POSITION_ACCELERATION[0], LUA_SET_CARTESIAN_POSITION_ACCELERATION_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(137, rcsHandle.c_str());

            inputBlock.pushBackReal(inData->at(1).doubleData[0]);

            inputBlock.pushBackInteger(inData->at(2).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_CARTESIAN_POSITION_ACCELERATION_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_CARTESIAN_POSITION_ACCELERATION_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_CARTESIAN_POSITION_ACCELERATION_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_CARTESIAN_POSITION_ACCELERATION_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_CARTESIAN_ORIENTATION_ACCELERATION
// --------------------------------------------------------------------------------------
#define LUA_SET_CARTESIAN_ORIENTATION_ACCELERATION_COMMAND "simExtRRS1_SET_CARTESIAN_ORIENTATION_ACCELERATION"
const int inArgs_SET_CARTESIAN_ORIENTATION_ACCELERATION[] = {
    4,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_int, 0,
    sim_lua_arg_double, 0,
    sim_lua_arg_int, 0,
};

void LUA_SET_CARTESIAN_ORIENTATION_ACCELERATION_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SET_CARTESIAN_ORIENTATION_ACCELERATION, inArgs_SET_CARTESIAN_ORIENTATION_ACCELERATION[0], LUA_SET_CARTESIAN_ORIENTATION_ACCELERATION_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(138, rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            inputBlock.pushBackReal(inData->at(2).doubleData[0]);

            inputBlock.pushBackInteger(inData->at(3).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_CARTESIAN_ORIENTATION_ACCELERATION_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_CARTESIAN_ORIENTATION_ACCELERATION_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_CARTESIAN_ORIENTATION_ACCELERATION_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_CARTESIAN_ORIENTATION_ACCELERATION_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_JOINT_JERKS
// --------------------------------------------------------------------------------------
#define LUA_SET_JOINT_JERKS_COMMAND "simExtRRS1_SET_JOINT_JERKS"
const int inArgs_SET_JOINT_JERKS[] = {
    5,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_int, 0,
    sim_lua_arg_charbuff, BITSTRING_SIZE,
    sim_lua_arg_double|sim_lua_arg_table, 32,
    sim_lua_arg_int, 0,
};

void LUA_SET_JOINT_JERKS_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SET_JOINT_JERKS, inArgs_SET_JOINT_JERKS[0], LUA_SET_JOINT_JERKS_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(162, rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            inputBlock.pushBackBitstring(inData->at(2).stringData[0]);

            for (unsigned int i=0;i<32;i++)
                inputBlock.pushBackReal(inData->at(3).doubleData[i]);

            inputBlock.pushBackInteger(inData->at(4).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_JOINT_JERKS_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_JOINT_JERKS_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_JOINT_JERKS_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_JOINT_JERKS_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_MOTION_TIME
// --------------------------------------------------------------------------------------
#define LUA_SET_MOTION_TIME_COMMAND "simExtRRS1_SET_MOTION_TIME"
const int inArgs_SET_MOTION_TIME[] = {
    2,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_double, 0,
};

void LUA_SET_MOTION_TIME_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SET_MOTION_TIME, inArgs_SET_MOTION_TIME[0], LUA_SET_MOTION_TIME_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(156, rcsHandle.c_str());

            inputBlock.pushBackReal(inData->at(1).doubleData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_MOTION_TIME_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_MOTION_TIME_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_MOTION_TIME_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_MOTION_TIME_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_OVERRIDE_SPEED
// --------------------------------------------------------------------------------------
#define LUA_SET_OVERRIDE_SPEED_COMMAND "simExtRRS1_SET_OVERRIDE_SPEED"
const int inArgs_SET_OVERRIDE_SPEED[] = {
    3,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_double, 0,
    sim_lua_arg_int, 0,
};

void LUA_SET_OVERRIDE_SPEED_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SET_OVERRIDE_SPEED, inArgs_SET_OVERRIDE_SPEED[0], LUA_SET_OVERRIDE_SPEED_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(139, rcsHandle.c_str());

            inputBlock.pushBackReal(inData->at(1).doubleData[0]);

            inputBlock.pushBackInteger(inData->at(2).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_OVERRIDE_SPEED_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_OVERRIDE_SPEED_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_OVERRIDE_SPEED_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_OVERRIDE_SPEED_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_OVERRIDE_ACCELERATION
// --------------------------------------------------------------------------------------
#define LUA_SET_OVERRIDE_ACCELERATION_COMMAND "simExtRRS1_SET_OVERRIDE_ACCELERATION"
const int inArgs_SET_OVERRIDE_ACCELERATION[] = {
    4,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_double, 0,
    sim_lua_arg_int, 0,
    sim_lua_arg_int, 0,
};

void LUA_SET_OVERRIDE_ACCELERATION_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SET_OVERRIDE_ACCELERATION, inArgs_SET_OVERRIDE_ACCELERATION[0], LUA_SET_OVERRIDE_ACCELERATION_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(155, rcsHandle.c_str());

            inputBlock.pushBackReal(inData->at(1).doubleData[0]);

            inputBlock.pushBackInteger(inData->at(2).intData[0]);

            inputBlock.pushBackInteger(inData->at(3).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_OVERRIDE_ACCELERATION_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_OVERRIDE_ACCELERATION_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_OVERRIDE_ACCELERATION_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_OVERRIDE_ACCELERATION_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SELECT_FLYBY_MODE
// --------------------------------------------------------------------------------------
#define LUA_SELECT_FLYBY_MODE_COMMAND "simExtRRS1_SELECT_FLYBY_MODE"
const int inArgs_SELECT_FLYBY_MODE[] = {
    2,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_int, 0,
};

void LUA_SELECT_FLYBY_MODE_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SELECT_FLYBY_MODE, inArgs_SELECT_FLYBY_MODE[0], LUA_SELECT_FLYBY_MODE_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(140, rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SELECT_FLYBY_MODE_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SELECT_FLYBY_MODE_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SELECT_FLYBY_MODE_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SELECT_FLYBY_MODE_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_FLYBY_CRITERIA_PARAMETER
// --------------------------------------------------------------------------------------
#define LUA_SET_FLYBY_CRITERIA_PARAMETER_COMMAND "simExtRRS1_SET_FLYBY_CRITERIA_PARAMETER"
const int inArgs_SET_FLYBY_CRITERIA_PARAMETER[] = {
    4,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_int, 0,
    sim_lua_arg_int, 0,
    sim_lua_arg_double, 0,
};

void LUA_SET_FLYBY_CRITERIA_PARAMETER_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SET_FLYBY_CRITERIA_PARAMETER, inArgs_SET_FLYBY_CRITERIA_PARAMETER[0], LUA_SET_FLYBY_CRITERIA_PARAMETER_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(141, rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            inputBlock.pushBackInteger(inData->at(2).intData[0]);

            inputBlock.pushBackReal(inData->at(3).doubleData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_FLYBY_CRITERIA_PARAMETER_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_FLYBY_CRITERIA_PARAMETER_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_FLYBY_CRITERIA_PARAMETER_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_FLYBY_CRITERIA_PARAMETER_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SELECT_FLYBY_CRITERIA
// --------------------------------------------------------------------------------------
#define LUA_SELECT_FLYBY_CRITERIA_COMMAND "simExtRRS1_SELECT_FLYBY_CRITERIA"
const int inArgs_SELECT_FLYBY_CRITERIA[] = {
    2,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_int, 0,
};

void LUA_SELECT_FLYBY_CRITERIA_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SELECT_FLYBY_CRITERIA, inArgs_SELECT_FLYBY_CRITERIA[0], LUA_SELECT_FLYBY_CRITERIA_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(142, rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SELECT_FLYBY_CRITERIA_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SELECT_FLYBY_CRITERIA_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SELECT_FLYBY_CRITERIA_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SELECT_FLYBY_CRITERIA_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// CANCEL_FLYBY_CRITERIA
// --------------------------------------------------------------------------------------
#define LUA_CANCEL_FLYBY_CRITERIA_COMMAND "simExtRRS1_CANCEL_FLYBY_CRITERIA"
const int inArgs_CANCEL_FLYBY_CRITERIA[] = {
    2,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_int, 0,
};

void LUA_CANCEL_FLYBY_CRITERIA_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_CANCEL_FLYBY_CRITERIA, inArgs_CANCEL_FLYBY_CRITERIA[0], LUA_CANCEL_FLYBY_CRITERIA_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(143, rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_CANCEL_FLYBY_CRITERIA_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_CANCEL_FLYBY_CRITERIA_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_CANCEL_FLYBY_CRITERIA_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_CANCEL_FLYBY_CRITERIA_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SELECT_POINT_ACCURACY
// --------------------------------------------------------------------------------------
#define LUA_SELECT_POINT_ACCURACY_COMMAND "simExtRRS1_SELECT_POINT_ACCURACY"
const int inArgs_SELECT_POINT_ACCURACY[] = {
    2,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_int, 0,
};

void LUA_SELECT_POINT_ACCURACY_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SELECT_POINT_ACCURACY, inArgs_SELECT_POINT_ACCURACY[0], LUA_SELECT_POINT_ACCURACY_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(144, rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SELECT_POINT_ACCURACY_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SELECT_POINT_ACCURACY_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SELECT_POINT_ACCURACY_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SELECT_POINT_ACCURACY_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_POINT_ACCURACY_PARAMETER
// --------------------------------------------------------------------------------------
#define LUA_SET_POINT_ACCURACY_PARAMETER_COMMAND "simExtRRS1_SET_POINT_ACCURACY_PARAMETER"
const int inArgs_SET_POINT_ACCURACY_PARAMETER[] = {
    3,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_int, 0,
    sim_lua_arg_double, 0,
};

void LUA_SET_POINT_ACCURACY_PARAMETER_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SET_POINT_ACCURACY_PARAMETER, inArgs_SET_POINT_ACCURACY_PARAMETER[0], LUA_SET_POINT_ACCURACY_PARAMETER_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(145, rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            inputBlock.pushBackReal(inData->at(2).doubleData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_POINT_ACCURACY_PARAMETER_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_POINT_ACCURACY_PARAMETER_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_POINT_ACCURACY_PARAMETER_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_POINT_ACCURACY_PARAMETER_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_REST_PARAMETER
// --------------------------------------------------------------------------------------
#define LUA_SET_REST_PARAMETER_COMMAND "simExtRRS1_SET_REST_PARAMETER"
const int inArgs_SET_REST_PARAMETER[] = {
    3,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_int, 0,
    sim_lua_arg_double, 0,
};

void LUA_SET_REST_PARAMETER_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SET_REST_PARAMETER, inArgs_SET_REST_PARAMETER[0], LUA_SET_REST_PARAMETER_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(164, rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            inputBlock.pushBackReal(inData->at(2).doubleData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_REST_PARAMETER_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_REST_PARAMETER_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_REST_PARAMETER_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_REST_PARAMETER_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// GET_CURRENT_TARGETID
// --------------------------------------------------------------------------------------
#define LUA_GET_CURRENT_TARGETID_COMMAND "simExtRRS1_GET_CURRENT_TARGETID"
const int inArgs_GET_CURRENT_TARGETID[] = {
    1,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
};

void LUA_GET_CURRENT_TARGETID_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_GET_CURRENT_TARGETID, inArgs_GET_CURRENT_TARGETID[0], LUA_GET_CURRENT_TARGETID_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(163, rcsHandle.c_str());

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }
                        if (s >= 12)
                        {
                            int targetId = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(targetId));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_GET_CURRENT_TARGETID_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_GET_CURRENT_TARGETID_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_GET_CURRENT_TARGETID_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_GET_CURRENT_TARGETID_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SELECT_TRACKING
// --------------------------------------------------------------------------------------
#define LUA_SELECT_TRACKING_COMMAND "simExtRRS1_SELECT_TRACKING"
const int inArgs_SELECT_TRACKING[] = {
    2,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_charbuff, BITSTRING_SIZE,
};

void LUA_SELECT_TRACKING_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SELECT_TRACKING, inArgs_SELECT_TRACKING[0], LUA_SELECT_TRACKING_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(146, rcsHandle.c_str());

            inputBlock.pushBackBitstring(inData->at(1).stringData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SELECT_TRACKING_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SELECT_TRACKING_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SELECT_TRACKING_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SELECT_TRACKING_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_CONVEYOR_POSITION
// --------------------------------------------------------------------------------------
#define LUA_SET_CONVEYOR_POSITION_COMMAND "simExtRRS1_SET_CONVEYOR_POSITION"
const int inArgs_SET_CONVEYOR_POSITION[] = {
    4,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_charbuff, BITSTRING_SIZE,
    sim_lua_arg_charbuff, BITSTRING_SIZE,
    sim_lua_arg_double|sim_lua_arg_table, 32,
};

void LUA_SET_CONVEYOR_POSITION_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SET_CONVEYOR_POSITION, inArgs_SET_CONVEYOR_POSITION[0], LUA_SET_CONVEYOR_POSITION_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(147, rcsHandle.c_str());

            inputBlock.pushBackBitstring(inData->at(1).stringData[0]);

            inputBlock.pushBackBitstring(inData->at(2).stringData[0]);

            for (unsigned int i=0;i<32;i++)
                inputBlock.pushBackReal(inData->at(3).doubleData[i]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_CONVEYOR_POSITION_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_CONVEYOR_POSITION_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_CONVEYOR_POSITION_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_CONVEYOR_POSITION_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// DEFINE_EVENT
// --------------------------------------------------------------------------------------
#define LUA_DEFINE_EVENT_COMMAND "simExtRRS1_DEFINE_EVENT"
const int inArgs_DEFINE_EVENT[] = {
    6,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_int, 0,
    sim_lua_arg_int, 0,
    sim_lua_arg_double, 0,
    sim_lua_arg_int, 0,
    sim_lua_arg_double|sim_lua_arg_table, 16,
};

void LUA_DEFINE_EVENT_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_DEFINE_EVENT, inArgs_DEFINE_EVENT[0], LUA_DEFINE_EVENT_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(148, rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            inputBlock.pushBackInteger(inData->at(2).intData[0]);

            inputBlock.pushBackReal(inData->at(3).doubleData[0]);

            inputBlock.pushBackInteger(inData->at(4).intData[0]);

            for (unsigned int i=0;i<16;i++)
                inputBlock.pushBackReal(inData->at(5).doubleData[i]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_DEFINE_EVENT_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_DEFINE_EVENT_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_DEFINE_EVENT_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_DEFINE_EVENT_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// CANCEL_EVENT
// --------------------------------------------------------------------------------------
#define LUA_CANCEL_EVENT_COMMAND "simExtRRS1_CANCEL_EVENT"
const int inArgs_CANCEL_EVENT[] = {
    2,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_int, 0,
 };

void LUA_CANCEL_EVENT_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_CANCEL_EVENT, inArgs_CANCEL_EVENT[0], LUA_CANCEL_EVENT_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(149, rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_CANCEL_EVENT_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_CANCEL_EVENT_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_CANCEL_EVENT_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_CANCEL_EVENT_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// GET_EVENT
// --------------------------------------------------------------------------------------
#define LUA_GET_EVENT_COMMAND "simExtRRS1_GET_EVENT"
const int inArgs_GET_EVENT[] = {
    2,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_int, 0,
 };

void LUA_GET_EVENT_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_GET_EVENT, inArgs_GET_EVENT[0], LUA_GET_EVENT_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(150, rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }
                        if (s >= 12)
                        {
                            int eventId = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(eventId));

                            double timeTillEvent = outputBlock.readReal();
                            D.pushOutData(CLuaFunctionDataItem(timeTillEvent));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_GET_EVENT_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_GET_EVENT_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_GET_EVENT_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_GET_EVENT_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// STOP_MOTION
// --------------------------------------------------------------------------------------
#define LUA_STOP_MOTION_COMMAND "simExtRRS1_STOP_MOTION"
const int inArgs_STOP_MOTION[] = {
    1,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
 };

void LUA_STOP_MOTION_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_STOP_MOTION, inArgs_STOP_MOTION[0], LUA_STOP_MOTION_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(151, rcsHandle.c_str());

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_STOP_MOTION_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_STOP_MOTION_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_STOP_MOTION_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_STOP_MOTION_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// CONTINUE_MOTION
// --------------------------------------------------------------------------------------
#define LUA_CONTINUE_MOTION_COMMAND "simExtRRS1_CONTINUE_MOTION"
const int inArgs_CONTINUE_MOTION[] = {
    1,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
 };

void LUA_CONTINUE_MOTION_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_CONTINUE_MOTION, inArgs_CONTINUE_MOTION[0], LUA_CONTINUE_MOTION_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(152, rcsHandle.c_str());

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_CONTINUE_MOTION_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_CONTINUE_MOTION_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_CONTINUE_MOTION_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_CONTINUE_MOTION_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// CANCEL_MOTION
// --------------------------------------------------------------------------------------
#define LUA_CANCEL_MOTION_COMMAND "simExtRRS1_CANCEL_MOTION"
const int inArgs_CANCEL_MOTION[] = {
    1,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
 };

void LUA_CANCEL_MOTION_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_CANCEL_MOTION, inArgs_CANCEL_MOTION[0], LUA_CANCEL_MOTION_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(153, rcsHandle.c_str());

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_CANCEL_MOTION_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_CANCEL_MOTION_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_CANCEL_MOTION_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_CANCEL_MOTION_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// GET_MESSAGE
// --------------------------------------------------------------------------------------
#define LUA_GET_MESSAGE_COMMAND "simExtRRS1_GET_MESSAGE"
const int inArgs_GET_MESSAGE[] = {
    2,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_int, 0,
 };

void LUA_GET_MESSAGE_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_GET_MESSAGE, inArgs_GET_MESSAGE[0], LUA_GET_MESSAGE_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(154, rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }
                        if (s >= 12)
                        {
                            int severity = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(severity));

                            std::string text = outputBlock.readString();
                            D.pushOutData(CLuaFunctionDataItem(text));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_GET_MESSAGE_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_GET_MESSAGE_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_GET_MESSAGE_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_GET_MESSAGE_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SELECT_WEAVING_MODE
// --------------------------------------------------------------------------------------
#define LUA_SELECT_WEAVING_MODE_COMMAND "simExtRRS1_SELECT_WEAVING_MODE"
const int inArgs_SELECT_WEAVING_MODE[] = {
    2,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_int, 0,
 };

void LUA_SELECT_WEAVING_MODE_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SELECT_WEAVING_MODE, inArgs_SELECT_WEAVING_MODE[0], LUA_SELECT_WEAVING_MODE_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(157, rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SELECT_WEAVING_MODE_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SELECT_WEAVING_MODE_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SELECT_WEAVING_MODE_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SELECT_WEAVING_MODE_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SELECT_WEAVING_GROUP
// --------------------------------------------------------------------------------------
#define LUA_SELECT_WEAVING_GROUP_COMMAND "simExtRRS1_SELECT_WEAVING_GROUP"
const int inArgs_SELECT_WEAVING_GROUP[] = {
    3,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_int, 0,
    sim_lua_arg_int, 0,
 };

void LUA_SELECT_WEAVING_GROUP_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SELECT_WEAVING_GROUP, inArgs_SELECT_WEAVING_GROUP[0], LUA_SELECT_WEAVING_GROUP_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(158, rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            inputBlock.pushBackInteger(inData->at(2).intData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SELECT_WEAVING_GROUP_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SELECT_WEAVING_GROUP_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SELECT_WEAVING_GROUP_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SELECT_WEAVING_GROUP_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// SET_WEAVING_GROUP_PARAMETER
// --------------------------------------------------------------------------------------
#define LUA_SET_WEAVING_GROUP_PARAMETER_COMMAND "simExtRRS1_SET_WEAVING_GROUP_PARAMETER"
const int inArgs_SET_WEAVING_GROUP_PARAMETER[] = {
    4,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_int, 0,
    sim_lua_arg_int, 0,
    sim_lua_arg_double, 0,
 };

void LUA_SET_WEAVING_GROUP_PARAMETER_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_SET_WEAVING_GROUP_PARAMETER, inArgs_SET_WEAVING_GROUP_PARAMETER[0], LUA_SET_WEAVING_GROUP_PARAMETER_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(159, rcsHandle.c_str());

            inputBlock.pushBackInteger(inData->at(1).intData[0]);

            inputBlock.pushBackInteger(inData->at(2).intData[0]);

            inputBlock.pushBackReal(inData->at(3).doubleData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_SET_WEAVING_GROUP_PARAMETER_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_SET_WEAVING_GROUP_PARAMETER_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_SET_WEAVING_GROUP_PARAMETER_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_SET_WEAVING_GROUP_PARAMETER_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// DEBUG
// --------------------------------------------------------------------------------------
#define LUA_DEBUG_COMMAND "simExtRRS1_DEBUG"
const int inArgs_DEBUG[] = {
    4,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_charbuff, BITSTRING_SIZE,
    sim_lua_arg_int, 0,
    sim_lua_arg_string, 0,
 };

void LUA_DEBUG_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_DEBUG, inArgs_DEBUG[0], LUA_DEBUG_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(1000, rcsHandle.c_str());

            inputBlock.pushBackBitstring(inData->at(1).stringData[0]);

            inputBlock.pushBackInteger(inData->at(2).intData[0]);

            inputBlock.pushBackString(inData->at(3).stringData[0]);

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();
                        if (s >= 8)
                        {
                            int status = outputBlock.readInteger();
                            D.pushOutData(CLuaFunctionDataItem(status));
                        }

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_DEBUG_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_DEBUG_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_DEBUG_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_DEBUG_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------
// EXTENDED_SERVICE
// --------------------------------------------------------------------------------------
#define LUA_EXTENDED_SERVICE_COMMAND "simExtRRS1_EXTENDED_SERVICE"
const int inArgs_EXTENDED_SERVICE[] = {
    2,
    sim_lua_arg_charbuff, BITSTRING2_SIZE,
    sim_lua_arg_charbuff, 0, // can be any length
 };

void LUA_EXTENDED_SERVICE_CALLBACK(SLuaCallBack* p)
{
    p->outputArgCount = 0;
    CLuaFunctionData D;
    if (D.readDataFromLua(p, inArgs_EXTENDED_SERVICE, inArgs_EXTENDED_SERVICE[0], LUA_EXTENDED_SERVICE_COMMAND))
    {
        std::vector<CLuaFunctionDataItem>* inData = D.getInDataPtr();
        int scriptId = p->scriptID;
        int index = getServerIndexFromScriptId(scriptId);
        if (index != -1)
        {
            std::string rcsHandle(getPartialString(inData->at(0).stringData[0], BITSTRING2_SIZE));
            CInputOutputBlock inputBlock(1001, rcsHandle.c_str());

			inputBlock.pushBackBuffer(&inData->at(1).stringData[0][0],(int)inData->at(1).stringData[0].size());

            int inputBlockSize;
            void* inputBlockPointer = inputBlock.getDataPointer(&inputBlockSize);
            if (allRcsServers[index].connection->sendData((char*)inputBlockPointer, inputBlockSize))
            {
                int outputBlockSize;
                char* data = allRcsServers[index].connection->receiveReplyData(outputBlockSize);
                if (data != 0)
                {
                    CInputOutputBlock outputBlock((unsigned char*)data, outputBlockSize);
                    delete[] data;
                    if (outputBlockSize >= 4)
                    {
                        int s = outputBlock.readInteger();

						if (s>8)
						{
							std::string outData=outputBlock.readBuffer(s-8);
							D.pushOutData(CLuaFunctionDataItem(&outData[0],(int)outData.size()));
						}

                        D.writeDataToLua(p);
                    }
                    else
                        simSetLastError(LUA_EXTENDED_SERVICE_COMMAND, "Received a bad reply from the server.");
                }
                else
                    simSetLastError(LUA_EXTENDED_SERVICE_COMMAND, "Failed receiving a reply from the server.");
            }
            else
                simSetLastError(LUA_EXTENDED_SERVICE_COMMAND, "Failed sending data to the server.");
        }
        else
            simSetLastError(LUA_EXTENDED_SERVICE_COMMAND, "There is no RCS server currently selected for this script.");

    }
}
// --------------------------------------------------------------------------------------

VREP_DLLEXPORT unsigned char v_repStart(void* reservedPointer,int reservedInt)
{
	// 1. Figure out this plugin's directory:
	char curDirAndFile[1024];
#ifdef _WIN32
	#ifdef QT_COMPIL
		_getcwd(curDirAndFile, sizeof(curDirAndFile));
	#else
		GetModuleFileName(NULL,curDirAndFile,1023);
		PathRemoveFileSpec(curDirAndFile);
	#endif
#elif defined (__linux) || defined (__APPLE__)
	getcwd(curDirAndFile, sizeof(curDirAndFile));
#endif
	currentDirAndPath=curDirAndFile;

	// 2. Append the V-REP library's name:
	std::string temp(currentDirAndPath);
#ifdef _WIN32
	temp+="\\v_rep.dll";
#elif defined (__linux)
	temp+="/libv_rep.so";
#elif defined (__APPLE__)
	temp+="/libv_rep.dylib";
#endif 

	// 3. Load the V-REP library:
	vrepLib=loadVrepLibrary(temp.c_str());
	if (vrepLib==NULL)
	{
		std::cout << "Error, could not find or correctly load the V-REP library. Cannot start 'RRS1' plugin.\n";
		return(0); 
	}
	if (getVrepProcAddresses(vrepLib)==0)
	{
		std::cout << "Error, could not find all required functions in the V-REP library. Cannot start 'RRS1' plugin.\n";
		unloadVrepLibrary(vrepLib);
		return(0);
	}

	// Check the version of V-REP:
	int vrepVer;
	simGetIntegerParameter(sim_intparam_program_version,&vrepVer);
	if (vrepVer<30200)
	{
		std::cout << "Sorry, your V-REP copy is somewhat old. Cannot start 'RRS1' plugin.\n";
		unloadVrepLibrary(vrepLib);
		return(0);
	}



	std::vector<int> inArgs;

	// startRcsModule (auxiliary command)
	CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_START_RCS_SERVER,inArgs);
	simRegisterCustomLuaFunction(LUA_START_RCS_SERVER_COMMAND,strConCat("number rcsServerHandle=",LUA_START_RCS_SERVER_COMMAND,"(string rcsLibraryFilename,string rcsLibraryFunctionName,int portNumber)"),&inArgs[0],LUA_START_RCS_SERVER_CALLBACK);

	// selectRcsModule (auxiliary command)
	CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SELECT_RCS_SERVER,inArgs);
	simRegisterCustomLuaFunction(LUA_SELECT_RCS_SERVER_COMMAND,strConCat("boolean result=",LUA_SELECT_RCS_SERVER_COMMAND,"(number rcsServerHandle)"),&inArgs[0],LUA_SELECT_RCS_SERVER_CALLBACK);

	// stopRcsModule (auxiliary command)
	CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_STOP_RCS_SERVER,inArgs);
	simRegisterCustomLuaFunction(LUA_STOP_RCS_SERVER_COMMAND,strConCat("boolean result=",LUA_STOP_RCS_SERVER_COMMAND,"(number rcsServerHandle)"),&inArgs[0],LUA_STOP_RCS_SERVER_CALLBACK);

	// INITIALIZE
	CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_INITIALIZE,inArgs);
    simRegisterCustomLuaFunction(LUA_INITIALIZE_COMMAND,strConCat("int status,bitstring2 rcsHandle,int rcsRrsVersion,int rcsVersion,int numberOfMessages=",LUA_INITIALIZE_COMMAND,"(int robotNumber,string robotPathName,string modulePathName,string manipulatorType,int CarrrsVersion,int debug)"),&inArgs[0],LUA_INITIALIZE_CALLBACK);

	// RESET
	CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_RESET,inArgs);
    simRegisterCustomLuaFunction(LUA_RESET_COMMAND,strConCat("int status,int numberOfMessages=",LUA_RESET_COMMAND,"(bitstring2 rcsHandle,int resetLevel)"),&inArgs[0],LUA_RESET_CALLBACK);

	// TERMINATE
	CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_TERMINATE,inArgs);
    simRegisterCustomLuaFunction(LUA_TERMINATE_COMMAND,strConCat("int status=",LUA_TERMINATE_COMMAND,"(bitstring2 rcsHandle)"),&inArgs[0],LUA_TERMINATE_CALLBACK);

    // GET_ROBOT_STAMP
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_GET_ROBOT_STAMP,inArgs);
    simRegisterCustomLuaFunction(LUA_GET_ROBOT_STAMP_COMMAND,strConCat("int status,string manipulator,string controller,string software=",LUA_GET_ROBOT_STAMP_COMMAND,"(bitstring2 rcsHandle)"),&inArgs[0],LUA_GET_ROBOT_STAMP_CALLBACK);

    // GET_HOME_JOINT_POSITION
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_GET_HOME_JOINT_POSITION,inArgs);
    simRegisterCustomLuaFunction(LUA_GET_HOME_JOINT_POSITION_COMMAND,strConCat("int status,jointPosType homePosition=",LUA_GET_HOME_JOINT_POSITION_COMMAND,"(bitstring2 rcsHandle)"),&inArgs[0],LUA_GET_HOME_JOINT_POSITION_CALLBACK);

    // GET_RCS_DATA
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_GET_RCS_DATA,inArgs);
    simRegisterCustomLuaFunction(LUA_GET_RCS_DATA_COMMAND,strConCat("int status,string paramId,string paramContents,int permission=",LUA_GET_RCS_DATA_COMMAND,"(bitstring2 rcsHandle,int storage,int firstNext,string paramId)"),&inArgs[0],LUA_GET_RCS_DATA_CALLBACK);

    // MODIFY_RCS_DATA
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_MODIFY_RCS_DATA,inArgs);
    simRegisterCustomLuaFunction(LUA_MODIFY_RCS_DATA_COMMAND,strConCat("int status=",LUA_MODIFY_RCS_DATA_COMMAND,"(bitstring2 rcsHandle,int storage,string paramId,string paramContents)"),&inArgs[0],LUA_MODIFY_RCS_DATA_CALLBACK);

    // SAVE_RCS_DATA
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SAVE_RCS_DATA,inArgs);
    simRegisterCustomLuaFunction(LUA_SAVE_RCS_DATA_COMMAND,strConCat("int status=",LUA_SAVE_RCS_DATA_COMMAND,"(bitstring2 rcsHandle)"),&inArgs[0],LUA_SAVE_RCS_DATA_CALLBACK);

    // LOAD_RCS_DATA
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_LOAD_RCS_DATA,inArgs);
    simRegisterCustomLuaFunction(LUA_LOAD_RCS_DATA_COMMAND,strConCat("int status,int numberOfMessages=",LUA_LOAD_RCS_DATA_COMMAND,"(bitstring2 rcsHandle)"),&inArgs[0],LUA_LOAD_RCS_DATA_CALLBACK);

    // GET_INVERSE_KINEMATIC
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_GET_INVERSE_KINEMATIC,inArgs);
    simRegisterCustomLuaFunction(LUA_GET_INVERSE_KINEMATIC_COMMAND,strConCat("int status,jointPosType jointPos,bitString jointLimit,int numberOfMessages=",LUA_GET_INVERSE_KINEMATIC_COMMAND,"(bitstring2 rcsHandle,cartPosType cartPos,jointPosType jointPos,string configuration,bitstring outputFormat)"),&inArgs[0],LUA_GET_INVERSE_KINEMATIC_CALLBACK);

    // GET_FORWARD_KINEMATIC
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_GET_FORWARD_KINEMATIC,inArgs);
    simRegisterCustomLuaFunction(LUA_GET_FORWARD_KINEMATIC_COMMAND,strConCat("int status,cartPosType cartPos,jointPosType jointPos,string configuration,bitString jointLimit,int numberOfMessages=",LUA_GET_FORWARD_KINEMATIC_COMMAND,"(bitstring2 rcsHandle,jointPosType jointPos)"),&inArgs[0],LUA_GET_FORWARD_KINEMATIC_CALLBACK);

    // MATRIX_TO_CONTROLLER_POSITION
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_MATRIX_TO_CONTROLLER_POSITION,inArgs);
    simRegisterCustomLuaFunction(LUA_MATRIX_TO_CONTROLLER_POSITION_COMMAND,strConCat("int status,string contrPos=",LUA_MATRIX_TO_CONTROLLER_POSITION_COMMAND,"(bitstring2 rcsHandle,cartPosType cartPos,string configuration)"),&inArgs[0],LUA_MATRIX_TO_CONTROLLER_POSITION_CALLBACK);

    // CONTROLLER_POSITION_TO_MATRIX
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_CONTROLLER_POSITION_TO_MATRIX,inArgs);
    simRegisterCustomLuaFunction(LUA_CONTROLLER_POSITION_TO_MATRIX_COMMAND,strConCat("int status,cartPosType cartPos,string configuration=",LUA_CONTROLLER_POSITION_TO_MATRIX_COMMAND,"(bitstring2 rcsHandle,string contrPos)"),&inArgs[0],LUA_CONTROLLER_POSITION_TO_MATRIX_CALLBACK);

    // GET_CELL_FRAME
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_GET_CELL_FRAME,inArgs);
    simRegisterCustomLuaFunction(LUA_GET_CELL_FRAME_COMMAND,strConCat("int status,string frameId,int frameType,string relativeToId,bitstring jointNumber,frame frameData=",LUA_GET_CELL_FRAME_COMMAND,"(bitstring2 rcsHandle,int storage,int firstNext,string frameId)"),&inArgs[0],LUA_GET_CELL_FRAME_CALLBACK);

    // MODIFY_CELL_FRAME
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_MODIFY_CELL_FRAME,inArgs);
    simRegisterCustomLuaFunction(LUA_MODIFY_CELL_FRAME_COMMAND,strConCat("int status=",LUA_MODIFY_CELL_FRAME_COMMAND,"(bitstring2 rcsHandle,int storage,string frameId,frame frameData)"),&inArgs[0],LUA_MODIFY_CELL_FRAME_CALLBACK);

    // SELECT_WORK_FRAMES
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SELECT_WORK_FRAMES,inArgs);
    simRegisterCustomLuaFunction(LUA_SELECT_WORK_FRAMES_COMMAND,strConCat("int status=",LUA_SELECT_WORK_FRAMES_COMMAND,"(bitstring2 rcsHandle,string toolId,string objectId)"),&inArgs[0],LUA_SELECT_WORK_FRAMES_CALLBACK);

    // SET_INITIAL_POSITION
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_INITIAL_POSITION,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_INITIAL_POSITION_COMMAND,strConCat("int status,bitstring jointLimit=",LUA_SET_INITIAL_POSITION_COMMAND,"(bitstring2 rcsHandle,cartPosType cartPos,jointPosType jointPos,string configuration)"),&inArgs[0],LUA_SET_INITIAL_POSITION_CALLBACK);

    // SET_NEXT_TARGET
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_NEXT_TARGET,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_NEXT_TARGET_COMMAND,strConCat("int status=",LUA_SET_NEXT_TARGET_COMMAND,"(bitstring2 rcsHandle,int targetId,int targetParam,cartPosType cartPos,jointPosType jointPos,string configuration,real targetParamValue)"),&inArgs[0],LUA_SET_NEXT_TARGET_CALLBACK);

    // GET_NEXT_STEP
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_GET_NEXT_STEP,inArgs);
    simRegisterCustomLuaFunction(LUA_GET_NEXT_STEP_COMMAND,strConCat("int status,cartPosType cartPos,jointPosType jointPos,string configuration,real elapsedTime,bitstring jointLimit,int numberOfEvents,int numberOfMessages=",LUA_GET_NEXT_STEP_COMMAND,"(bitstring2 rcsHandle,bitstring outputFormat)"),&inArgs[0],LUA_GET_NEXT_STEP_CALLBACK);

    // SET_INTERPOLATION_TIME
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_INTERPOLATION_TIME,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_INTERPOLATION_TIME_COMMAND,strConCat("int status=",LUA_SET_INTERPOLATION_TIME_COMMAND,"(bitstring2 rcsHandle,real interpolationTime)"),&inArgs[0],LUA_SET_INTERPOLATION_TIME_CALLBACK);

    // SELECT_MOTION_TYPE
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SELECT_MOTION_TYPE,inArgs);
    simRegisterCustomLuaFunction(LUA_SELECT_MOTION_TYPE_COMMAND,strConCat("int status=",LUA_SELECT_MOTION_TYPE_COMMAND,"(bitstring2 rcsHandle,int motionType)"),&inArgs[0],LUA_SELECT_MOTION_TYPE_CALLBACK);

    // SELECT_TARGET_TYPE
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SELECT_TARGET_TYPE,inArgs);
    simRegisterCustomLuaFunction(LUA_SELECT_TARGET_TYPE_COMMAND,strConCat("int status=",LUA_SELECT_TARGET_TYPE_COMMAND,"(bitstring2 rcsHandle,int targetType,cartPosType cartPos,jointPosType jointPos,string configuration)"),&inArgs[0],LUA_SELECT_TARGET_TYPE_CALLBACK);

    // SELECT_TRAJECTORY_MODE
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SELECT_TRAJECTORY_MODE,inArgs);
    simRegisterCustomLuaFunction(LUA_SELECT_TRAJECTORY_MODE_COMMAND,strConCat("int status=",LUA_SELECT_TRAJECTORY_MODE_COMMAND,"(bitstring2 rcsHandle,int trajectoryOn)"),&inArgs[0],LUA_SELECT_TRAJECTORY_MODE_CALLBACK);

    // SELECT_ORIENTATION_INTERPOLATION_MODE
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SELECT_ORIENTATION_INTERPOLATION_MODE,inArgs);
    simRegisterCustomLuaFunction(LUA_SELECT_ORIENTATION_INTERPOLATION_MODE_COMMAND,strConCat("int status=",LUA_SELECT_ORIENTATION_INTERPOLATION_MODE_COMMAND,"(bitstring2 rcsHandle,int interpolationMode,int oriConst)"),&inArgs[0],LUA_SELECT_ORIENTATION_INTERPOLATION_MODE_CALLBACK);

    // SELECT_DOMINANT_INTERPOLATION
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SELECT_DOMINANT_INTERPOLATION,inArgs);
    simRegisterCustomLuaFunction(LUA_SELECT_DOMINANT_INTERPOLATION_COMMAND,strConCat("int status=",LUA_SELECT_DOMINANT_INTERPOLATION_COMMAND,"(bitstring2 rcsHandle,int dominantIntType,int dominantIntParam)"),&inArgs[0],LUA_SELECT_DOMINANT_INTERPOLATION_CALLBACK);

    // SET_ADVANCE_MOTION
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_ADVANCE_MOTION,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_ADVANCE_MOTION_COMMAND,strConCat("int status=",LUA_SET_ADVANCE_MOTION_COMMAND,"(bitstring2 rcsHandle,int numberOfMotion)"),&inArgs[0],LUA_SET_ADVANCE_MOTION_CALLBACK);

    // SET_MOTION_FILTER
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_MOTION_FILTER,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_MOTION_FILTER_COMMAND,strConCat("int status=",LUA_SET_MOTION_FILTER_COMMAND,"(bitstring2 rcsHandle,int filterFactor)"),&inArgs[0],LUA_SET_MOTION_FILTER_CALLBACK);

    // SET_OVERRIDE_POSITION
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_OVERRIDE_POSITION,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_OVERRIDE_POSITION_COMMAND,strConCat("int status=",LUA_SET_OVERRIDE_POSITION_COMMAND,"(bitstring2 rcsHandle,frame posOffset)"),&inArgs[0],LUA_SET_OVERRIDE_POSITION_CALLBACK);

    // REVERSE_MOTION
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_REVERSE_MOTION,inArgs);
    simRegisterCustomLuaFunction(LUA_REVERSE_MOTION_COMMAND,strConCat("int status=",LUA_REVERSE_MOTION_COMMAND,"(bitstring2 rcsHandle,real distance)"),&inArgs[0],LUA_REVERSE_MOTION_CALLBACK);

    // SET_PAYLOAD_PARAMETER
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_PAYLOAD_PARAMETER,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_PAYLOAD_PARAMETER_COMMAND,strConCat("int status=",LUA_SET_PAYLOAD_PARAMETER_COMMAND,"(bitstring2 rcsHandle,int storage,string frameId,int paramNumber,real paramValue)"),&inArgs[0],LUA_SET_PAYLOAD_PARAMETER_CALLBACK);

    // SELECT_TIME_COMPENSATION
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SELECT_TIME_COMPENSATION,inArgs);
    simRegisterCustomLuaFunction(LUA_SELECT_TIME_COMPENSATION_COMMAND,strConCat("int status=",LUA_SELECT_TIME_COMPENSATION_COMMAND,"(bitstring2 rcsHandle,bitstring compensation)"),&inArgs[0],LUA_SELECT_TIME_COMPENSATION_CALLBACK);

    // SET_CONFIGURATION_CONTROL
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_CONFIGURATION_CONTROL,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_CONFIGURATION_CONTROL_COMMAND,strConCat("int status=",LUA_SET_CONFIGURATION_CONTROL_COMMAND,"(bitstring2 rcsHandle,string paramId,string paramContents)"),&inArgs[0],LUA_SET_CONFIGURATION_CONTROL_CALLBACK);

    // SET_JOINT_SPEEDS
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_JOINT_SPEEDS,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_JOINT_SPEEDS_COMMAND,strConCat("int status=",LUA_SET_JOINT_SPEEDS_COMMAND,"(bitstring2 rcsHandle,int allJointFlags,bitstring jointFlags,real_32 speedPercent)"),&inArgs[0],LUA_SET_JOINT_SPEEDS_CALLBACK);

    // SET_CARTESIAN_POSITION_SPEED
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_CARTESIAN_POSITION_SPEED,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_CARTESIAN_POSITION_SPEED_COMMAND,strConCat("int status=",LUA_SET_CARTESIAN_POSITION_SPEED_COMMAND,"(bitstring2 rcsHandle,real speedValue)"),&inArgs[0],LUA_SET_CARTESIAN_POSITION_SPEED_CALLBACK);

    // SET_CARTESIAN_ORIENTATION_SPEED
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_CARTESIAN_ORIENTATION_SPEED,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_CARTESIAN_ORIENTATION_SPEED_COMMAND,strConCat("int status=",LUA_SET_CARTESIAN_ORIENTATION_SPEED_COMMAND,"(bitstring2 rcsHandle,int rotationNo,real speedValue)"),&inArgs[0],LUA_SET_CARTESIAN_ORIENTATION_SPEED_CALLBACK);

    // SET_JOINT_ACCELERATIONS
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_JOINT_ACCELERATIONS,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_JOINT_ACCELERATIONS_COMMAND,strConCat("int status=",LUA_SET_JOINT_ACCELERATIONS_COMMAND,"(bitstring2 rcsHandle,int allJointFlags,bitstring jointFlags,real_32 accelPercent,int accelType)"),&inArgs[0],LUA_SET_JOINT_ACCELERATIONS_CALLBACK);

    // SET_CARTESIAN_POSITION_ACCELERATION
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_CARTESIAN_POSITION_ACCELERATION,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_CARTESIAN_POSITION_ACCELERATION_COMMAND,strConCat("int status=",LUA_SET_CARTESIAN_POSITION_ACCELERATION_COMMAND,"(bitstring2 rcsHandle,real accelValue,int accelType)"),&inArgs[0],LUA_SET_CARTESIAN_POSITION_ACCELERATION_CALLBACK);

    // SET_CARTESIAN_ORIENTATION_ACCELERATION
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_CARTESIAN_ORIENTATION_ACCELERATION,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_CARTESIAN_ORIENTATION_ACCELERATION_COMMAND,strConCat("int status=",LUA_SET_CARTESIAN_ORIENTATION_ACCELERATION_COMMAND,"(bitstring2 rcsHandle,int rotationNo,real accelValue,int accelType)"),&inArgs[0],LUA_SET_CARTESIAN_ORIENTATION_ACCELERATION_CALLBACK);

    // SET_JOINT_JERKS
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_JOINT_JERKS,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_JOINT_JERKS_COMMAND,strConCat("int status=",LUA_SET_JOINT_JERKS_COMMAND,"(bitstring2 rcsHandle,int allJointFlags,bitstring jointFlags,real_32 jerkPercent,int jerkType)"),&inArgs[0],LUA_SET_JOINT_JERKS_CALLBACK);

    // SET_MOTION_TIME
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_MOTION_TIME,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_MOTION_TIME_COMMAND,strConCat("int status=",LUA_SET_MOTION_TIME_COMMAND,"(bitstring2 rcsHandle,real timeValue)"),&inArgs[0],LUA_SET_MOTION_TIME_CALLBACK);

    // SET_OVERRIDE_SPEED
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_OVERRIDE_SPEED,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_OVERRIDE_SPEED_COMMAND,strConCat("int status=",LUA_SET_OVERRIDE_SPEED_COMMAND,"(bitstring2 rcsHandle,real correctionValue,int correctionType)"),&inArgs[0],LUA_SET_OVERRIDE_SPEED_CALLBACK);

    // SET_OVERRIDE_ACCELERATION
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_OVERRIDE_ACCELERATION,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_OVERRIDE_ACCELERATION_COMMAND,strConCat("int status=",LUA_SET_OVERRIDE_ACCELERATION_COMMAND,"(bitstring2 rcsHandle,real correctionValue,int accelType,int correctionType)"),&inArgs[0],LUA_SET_OVERRIDE_ACCELERATION_CALLBACK);

    // SELECT_FLYBY_MODE
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SELECT_FLYBY_MODE,inArgs);
    simRegisterCustomLuaFunction(LUA_SELECT_FLYBY_MODE_COMMAND,strConCat("int status=",LUA_SELECT_FLYBY_MODE_COMMAND,"(bitstring2 rcsHandle,int flyByOn)"),&inArgs[0],LUA_SELECT_FLYBY_MODE_CALLBACK);

    // SET_FLYBY_CRITERIA_PARAMETER
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_FLYBY_CRITERIA_PARAMETER,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_FLYBY_CRITERIA_PARAMETER_COMMAND,strConCat("int status=",LUA_SET_FLYBY_CRITERIA_PARAMETER_COMMAND,"(bitstring2 rcsHandle,int paramNumber,int jointNr,real paramValue)"),&inArgs[0],LUA_SET_FLYBY_CRITERIA_PARAMETER_CALLBACK);

    // SELECT_FLYBY_CRITERIA
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SELECT_FLYBY_CRITERIA,inArgs);
    simRegisterCustomLuaFunction(LUA_SELECT_FLYBY_CRITERIA_COMMAND,strConCat("int status=",LUA_SELECT_FLYBY_CRITERIA_COMMAND,"(bitstring2 rcsHandle,int paramNumber)"),&inArgs[0],LUA_SELECT_FLYBY_CRITERIA_CALLBACK);

    // CANCEL_FLYBY_CRITERIA
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_CANCEL_FLYBY_CRITERIA,inArgs);
    simRegisterCustomLuaFunction(LUA_CANCEL_FLYBY_CRITERIA_COMMAND,strConCat("int status=",LUA_CANCEL_FLYBY_CRITERIA_COMMAND,"(bitstring2 rcsHandle,int paramNumber)"),&inArgs[0],LUA_CANCEL_FLYBY_CRITERIA_CALLBACK);

    // SELECT_POINT_ACCURACY
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SELECT_POINT_ACCURACY,inArgs);
    simRegisterCustomLuaFunction(LUA_SELECT_POINT_ACCURACY_COMMAND,strConCat("int status=",LUA_SELECT_POINT_ACCURACY_COMMAND,"(bitstring2 rcsHandle,int accuracyType)"),&inArgs[0],LUA_SELECT_POINT_ACCURACY_CALLBACK);

    // SET_POINT_ACCURACY_PARAMETER
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_POINT_ACCURACY_PARAMETER,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_POINT_ACCURACY_PARAMETER_COMMAND,strConCat("int status=",LUA_SET_POINT_ACCURACY_PARAMETER_COMMAND,"(bitstring2 rcsHandle,int accuracyType,real accuracyValue)"),&inArgs[0],LUA_SET_POINT_ACCURACY_PARAMETER_CALLBACK);

    // SET_REST_PARAMETER
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_REST_PARAMETER,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_REST_PARAMETER_COMMAND,strConCat("int status=",LUA_SET_REST_PARAMETER_COMMAND,"(bitstring2 rcsHandle,int paramNumber,real paramValue)"),&inArgs[0],LUA_SET_REST_PARAMETER_CALLBACK);

    // GET_CURRENT_TARGETID
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_GET_CURRENT_TARGETID,inArgs);
    simRegisterCustomLuaFunction(LUA_GET_CURRENT_TARGETID_COMMAND,strConCat("int status,int targetId=",LUA_GET_CURRENT_TARGETID_COMMAND,"(bitstring2 rcsHandle)"),&inArgs[0],LUA_GET_CURRENT_TARGETID_CALLBACK);

    // SELECT_TRACKING
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SELECT_TRACKING,inArgs);
    simRegisterCustomLuaFunction(LUA_SELECT_TRACKING_COMMAND,strConCat("int status=",LUA_SELECT_TRACKING_COMMAND,"(bitstring2 rcsHandle,bitstring conveyorFlags)"),&inArgs[0],LUA_SELECT_TRACKING_CALLBACK);

    // SET_CONVEYOR_POSITION
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_CONVEYOR_POSITION,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_CONVEYOR_POSITION_COMMAND,strConCat("int status=",LUA_SET_CONVEYOR_POSITION_COMMAND,"(bitstring2 rcsHandle,bitstring inputFormat,bitstring conveyorFlags,real_32 conveyorPos)"),&inArgs[0],LUA_SET_CONVEYOR_POSITION_CALLBACK);

    // DEFINE_EVENT
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_DEFINE_EVENT,inArgs);
    simRegisterCustomLuaFunction(LUA_DEFINE_EVENT_COMMAND,strConCat("int status=",LUA_DEFINE_EVENT_COMMAND,"(bitstring2 rcsHandle,int eventId,int targetId,real resolution,int typeOfEvent,real_16 eventSpec)"),&inArgs[0],LUA_DEFINE_EVENT_CALLBACK);

    // CANCEL_EVENT
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_CANCEL_EVENT,inArgs);
    simRegisterCustomLuaFunction(LUA_CANCEL_EVENT_COMMAND,strConCat("int status=",LUA_CANCEL_EVENT_COMMAND,"(bitstring2 rcsHandle,int eventId)"),&inArgs[0],LUA_CANCEL_EVENT_CALLBACK);

    // GET_EVENT
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_GET_EVENT,inArgs);
    simRegisterCustomLuaFunction(LUA_GET_EVENT_COMMAND,strConCat("int status,int eventId,real timeTillEvent=",LUA_GET_EVENT_COMMAND,"(bitstring2 rcsHandle,int eventNumber)"),&inArgs[0],LUA_GET_EVENT_CALLBACK);

    // STOP_MOTION
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_STOP_MOTION,inArgs);
    simRegisterCustomLuaFunction(LUA_STOP_MOTION_COMMAND,strConCat("int status=",LUA_STOP_MOTION_COMMAND,"(bitstring2 rcsHandle)"),&inArgs[0],LUA_STOP_MOTION_CALLBACK);

    // CONTINUE_MOTION
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_CONTINUE_MOTION,inArgs);
    simRegisterCustomLuaFunction(LUA_CONTINUE_MOTION_COMMAND,strConCat("int status=",LUA_CONTINUE_MOTION_COMMAND,"(bitstring2 rcsHandle)"),&inArgs[0],LUA_CONTINUE_MOTION_CALLBACK);

    // CANCEL_MOTION
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_CANCEL_MOTION,inArgs);
    simRegisterCustomLuaFunction(LUA_CANCEL_MOTION_COMMAND,strConCat("int status=",LUA_CANCEL_MOTION_COMMAND,"(bitstring2 rcsHandle)"),&inArgs[0],LUA_CANCEL_MOTION_CALLBACK);

    // GET_MESSAGE
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_GET_MESSAGE,inArgs);
    simRegisterCustomLuaFunction(LUA_GET_MESSAGE_COMMAND,strConCat("int status,int severity,string text=",LUA_GET_MESSAGE_COMMAND,"(bitstring2 rcsHandle,int messageNumber)"),&inArgs[0],LUA_GET_MESSAGE_CALLBACK);

    // SELECT_WEAVING_MODE
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SELECT_WEAVING_MODE,inArgs);
    simRegisterCustomLuaFunction(LUA_SELECT_WEAVING_MODE_COMMAND,strConCat("int status=",LUA_SELECT_WEAVING_MODE_COMMAND,"(bitstring2 rcsHandle,int weavingMode)"),&inArgs[0],LUA_SELECT_WEAVING_MODE_CALLBACK);

    // SELECT_WEAVING_GROUP
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SELECT_WEAVING_GROUP,inArgs);
    simRegisterCustomLuaFunction(LUA_SELECT_WEAVING_GROUP_COMMAND,strConCat("int status=",LUA_SELECT_WEAVING_GROUP_COMMAND,"(bitstring2 rcsHandle,int groupNo,int groupOn)"),&inArgs[0],LUA_SELECT_WEAVING_GROUP_CALLBACK);

    // SET_WEAVING_GROUP_PARAMETER
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_SET_WEAVING_GROUP_PARAMETER,inArgs);
    simRegisterCustomLuaFunction(LUA_SET_WEAVING_GROUP_PARAMETER_COMMAND,strConCat("int status=",LUA_SET_WEAVING_GROUP_PARAMETER_COMMAND,"(bitstring2 rcsHandle,int groupNo,int paramNo,real paramValue)"),&inArgs[0],LUA_SET_WEAVING_GROUP_PARAMETER_CALLBACK);

    // DEBUG
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_DEBUG,inArgs);
    simRegisterCustomLuaFunction(LUA_DEBUG_COMMAND,strConCat("int status=",LUA_DEBUG_COMMAND,"(bitstring2 rcsHandle,bitstring debugFlags,int opcodeSelect,string logFileName)"),&inArgs[0],LUA_DEBUG_CALLBACK);

    // EXTENDED_SERVICE
    CLuaFunctionData::getInputDataForFunctionRegistration(inArgs_EXTENDED_SERVICE,inArgs);
    simRegisterCustomLuaFunction(LUA_EXTENDED_SERVICE_COMMAND,strConCat("int status,string outData=",LUA_EXTENDED_SERVICE_COMMAND,"(bitstring2 rcsHandle,string inData)"),&inArgs[0],LUA_EXTENDED_SERVICE_CALLBACK);

	return(PLUGIN_VERSION);
}

VREP_DLLEXPORT void v_repEnd()
{
	unloadVrepLibrary(vrepLib);
}

VREP_DLLEXPORT void* v_repMessage(int message,int* auxiliaryData,void* customData,int* replyData)
{ // This is called quite often. Just watch out for messages/events you want to handle
	// Keep following 5 lines at the beginning and unchanged:
	int errorModeSaved;
	simGetIntegerParameter(sim_intparam_error_report_mode,&errorModeSaved);
	simSetIntegerParameter(sim_intparam_error_report_mode,sim_api_errormessage_ignore);
	void* retVal=NULL;

	if (message==sim_message_eventcallback_simulationended)
	{ // simulation ended. End all started RCS servers:
		for (unsigned int i=0;i<allRcsServers.size();i++)
			delete allRcsServers[i].connection;
		allRcsServers.clear();
	}


	// Keep following unchanged:
	simSetIntegerParameter(sim_intparam_error_report_mode,errorModeSaved);
	return(retVal);
}

