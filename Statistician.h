// Statistician.h
// Collects statistics during grabbing
//
// Copyright (c) 2016-2019 Matthew Breit - matt.breit@baslerweb.com or matt.breit@gmail.com
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http ://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef STATISTICIAN_H
#define STATISTICIAN_H

#include <pylon/PylonIncludes.h>
#include <thread>
#include <atomic>
#include <set>

namespace Statistician
{
	class CStatistician
	{
	private:
		Pylon::CInstantCamera *m_camera;
		Pylon::String_t m_deviceClass;
		GenApi::CPortPtr m_devicePort;
		std::thread m_tstats;
		std::atomic<bool> m_stop;
		std::stringstream m_strStatistics;
		bool m_isInitialized;
		bool CheckStatistics();		

	public:
		//CStatistician(Pylon::CInstantCamera &camera);
		CStatistician();
		~CStatistician();
		void Initialize(Pylon::CInstantCamera &camera);
		bool Start();
		bool Stop();
		std::string PrintStatistics();

		// possible available statistics
		int64_t totalBuffers;
		int64_t failedBuffers;
		int64_t bufferUnderruns;
		int64_t totalPackets;
		int64_t resendRequests;
		int64_t resendPackets;
		int64_t failedPackets;
		int64_t lastFailedBufferStatus;
		Pylon::String_t lastFailedBufferStatusText;
		std::set<Pylon::String_t> lastFailedBufferStatusTextList;
		int64_t missedFrameCount;
		int64_t resyncCount;
		int64_t lastBlockID;
		Pylon::String_t lastError;
		std::set<Pylon::String_t> lastErrorList;
		int64_t currentTemperature;
		Pylon::String_t temperatureState;
		bool criticalTemp;
		bool overTemp;
		int64_t readPipeResetCount;
		int64_t writePipeResetCount;
		int64_t readOperationsFailedCount;
		int64_t writeOperationsFailedCount;
		int64_t tlLastErrorStatus;
		Pylon::String_t tlLastErrorStatusText;
		std::set<Pylon::String_t> tlLastErrorStatusTextList;
		uint64_t EPU;
		uint64_t PE;
		uint64_t UR;
		uint64_t EPR;
		uint64_t LE;
		uint64_t LR;
	};
}

// *********************************************************************************************************
// DEFINITIONS
inline Statistician::CStatistician::CStatistician()
{
}

inline Statistician::CStatistician::~CStatistician()
{
	this->Stop();
}

inline void Statistician::CStatistician::Initialize(Pylon::CInstantCamera &camera)
{
	try
	{
		m_stop.store(true);

		criticalTemp = false;
		overTemp = false;
		totalBuffers = -1;
		failedBuffers = -1;
		bufferUnderruns = -1;
		totalPackets = -1;
		resendRequests = -1;
		resendPackets = -1;
		failedPackets = -1;
		lastFailedBufferStatus = -1;
		readPipeResetCount = -1;
		writePipeResetCount = -1;
		readOperationsFailedCount = -1;
		writeOperationsFailedCount = -1;
		tlLastErrorStatus = -1;
		missedFrameCount = -1;
		resyncCount = -1;
		lastBlockID = -1;
		currentTemperature = -1;
		lastFailedBufferStatusText = "-1";
		lastError = "-1";
		temperatureState = "-1";
		tlLastErrorStatusText = "-1";
		EPU = 0;
		PE = 0;
		UR = 0;
		EPR = 0;
		LE = 0;
		LR = 0;

		m_camera = &camera;
		m_deviceClass = m_camera->GetDeviceInfo().GetDeviceClass();
		m_devicePort = camera.GetNodeMap().GetNode("Device");
		m_isInitialized = true;
	}
	catch (GenICam::GenericException &e)
	{
		std::cout << __FUNCTION__ << e.GetDescription() << std::endl;
		m_isInitialized = false;
	}
	catch (std::exception &e)
	{
		std::cout << __FUNCTION__ << e.what() << std::endl;
		m_isInitialized = false;
	}
}
inline bool Statistician::CStatistician::CheckStatistics()
{
	try
	{
		if (m_camera->IsOpen())
		{
			GenApi::INodeMap& m_control = m_camera->GetNodeMap();
			GenApi::INodeMap& m_grabber = m_camera->GetStreamGrabberNodeMap();
			GenApi::INodeMap& m_transport = m_camera->GetTLNodeMap();

			GenApi::CIntegerPtr ptrInteger;
			GenApi::CStringPtr ptrString;
			GenApi::CEnumerationPtr ptrEnumeration;
			GenApi::CBooleanPtr ptrBool;

			while (m_stop.load() == false)
			{
				ptrInteger = m_grabber.GetNode("Statistic_Last_Failed_Buffer_Status");
				if (GenApi::IsReadable(ptrInteger))
					lastFailedBufferStatus = ptrInteger->GetValue();

				ptrInteger = m_grabber.GetNode("Statistic_Total_Buffer_Count");
				if (GenApi::IsReadable(ptrInteger))
					totalBuffers = ptrInteger->GetValue();

				ptrInteger = m_grabber.GetNode("Statistic_Failed_Buffer_Count");
				if (GenApi::IsReadable(ptrInteger))
					failedBuffers = ptrInteger->GetValue();

				ptrInteger = m_control.GetNode("TemperatureAbs");
				if (GenApi::IsReadable(ptrInteger))
					currentTemperature = ptrInteger->GetValue();

				ptrInteger = m_transport.GetNode("Statistic_Last_Error_Status");
				if (GenApi::IsReadable(ptrInteger))
					tlLastErrorStatus = ptrInteger->GetValue();

				GenApi::CStringPtr ptrTLLastErrorStatusText;
				ptrTLLastErrorStatusText = m_transport.GetNode("Statistic_Last_Error_Status_Text");
				if (GenApi::IsReadable(ptrTLLastErrorStatusText))
				{
					tlLastErrorStatusText = ptrTLLastErrorStatusText->GetValue();
					if (tlLastErrorStatusText == "")
						tlLastErrorStatusText = "(no error)\n";
					else
						tlLastErrorStatusTextList.insert(tlLastErrorStatusText);
				}

				ptrString = m_grabber.GetNode("Statistic_Last_Failed_Buffer_Status_Text");
				if (GenApi::IsReadable(ptrString))
				{
					lastFailedBufferStatusText = ptrString->GetValue();
					if (lastFailedBufferStatusText == "")
						lastFailedBufferStatusText = "(no error)\n";
					else
						lastFailedBufferStatusTextList.insert(lastFailedBufferStatusText);
				}

				ptrEnumeration = m_grabber.GetNode("LastError");
				if (GenApi::IsReadable(ptrEnumeration))
				{
					lastError = ptrEnumeration->ToString();
					lastErrorList.insert(lastError);
				}

				ptrEnumeration = m_control.GetNode("TemperatureState");
				if (GenApi::IsReadable(ptrEnumeration))
					temperatureState = ptrEnumeration->ToString();

				ptrBool = m_control.GetNode("CriticalTemperature");
				if (GenApi::IsReadable(ptrBool))
					criticalTemp = ptrBool->GetValue();

				ptrBool = m_control.GetNode("OverTemperature");
				if (GenApi::IsReadable(ptrBool))
					overTemp = ptrBool->GetValue();

				if (m_deviceClass == Pylon::BaslerGigEDeviceClass)
				{
					ptrInteger = m_grabber.GetNode("Statistic_Buffer_Underrun_Count");
					if (GenApi::IsReadable(ptrInteger))
						bufferUnderruns = ptrInteger->GetValue();

					ptrInteger = m_grabber.GetNode("Statistic_Total_Packet_Count");
					if (GenApi::IsReadable(ptrInteger))
						totalPackets = ptrInteger->GetValue();

					ptrInteger = m_grabber.GetNode("Statistic_Resend_Request_Count");
					if (GenApi::IsReadable(ptrInteger))
						resendRequests = ptrInteger->GetValue();

					ptrInteger = m_grabber.GetNode("Statistic_Resend_Packet_Count");
					if (GenApi::IsReadable(ptrInteger))
						resendPackets = ptrInteger->GetValue();

					ptrInteger = m_grabber.GetNode("Statistic_Failed_Packet_Count");
					if (GenApi::IsReadable(ptrInteger))
						failedPackets = ptrInteger->GetValue();
				}

				if (m_deviceClass == Pylon::BaslerUsbDeviceClass)
				{
					ptrInteger = m_grabber.GetNode("Statistic_Missed_Frame_Count");
					if (GenApi::IsReadable(ptrInteger))
						missedFrameCount = ptrInteger->GetValue();

					ptrInteger = m_grabber.GetNode("Statistic_Resynchronization_Count");
					if (GenApi::IsReadable(ptrInteger))
						resyncCount = ptrInteger->GetValue();

					ptrInteger = m_grabber.GetNode("Statistic_Last_Block_Id");
					if (GenApi::IsReadable(ptrInteger))
						lastBlockID = ptrInteger->GetValue();

					ptrInteger = m_transport.GetNode("Statistic_Read_Pipe_Reset_Count");
					if (GenApi::IsReadable(ptrInteger))
						readPipeResetCount = ptrInteger->GetValue();

					ptrInteger = m_transport.GetNode("Statistic_Write_Pipe_Reset_Count");
					if (GenApi::IsReadable(ptrInteger))
						writePipeResetCount = ptrInteger->GetValue();

					ptrInteger = m_transport.GetNode("Statistic_Read_Operations_Failed_Count");
					if (GenApi::IsReadable(ptrInteger))
						readOperationsFailedCount = ptrInteger->GetValue();

					ptrInteger = m_transport.GetNode("Statistic_Write_Operations_Failed_Count");
					if (GenApi::IsReadable(ptrInteger))
						writeOperationsFailedCount = ptrInteger->GetValue();

					m_devicePort->Read(reinterpret_cast<char*>(&EPU), 0xAFFB0010, 4);
					m_devicePort->Read(reinterpret_cast<char*>(&PE), 0xAFFB0004, 4);
					m_devicePort->Read(reinterpret_cast<char*>(&UR), 0xAFFB000C, 4);
					m_devicePort->Read(reinterpret_cast<char*>(&EPR), 0xAFFB0020, 4);
					m_devicePort->Read(reinterpret_cast<char*>(&LE), 0xAFFB0008, 4);
					m_devicePort->Read(reinterpret_cast<char*>(&LR), 0xAFFB0014, 4);
				}
			}
			return true;
		}
		else
		{
			return false;
		}
	}
	catch (GenICam::GenericException /*&e*/)
	{
		// really the only exception would be due to camera removal, and we'll catch that in main().
		//	Utility_ParseError(e.GetDescription(), "CheckStatistics()");
		//std::cout << __FUNCTION__ << e.GetDescription() << std::endl; // just to satisfy compilier warning about unreferenced local variable.
		return false;
	}
	catch (std::exception /*&e*/)
	{
		// really the only exception would be due to camera removal, and we'll catch that in main().
		//	Utility_ParseError(e.what(), "CheckStatistics()");
		//std::cout << __FUNCTION__ << e.what() << std::endl; // just to satisfy compilier warning about unreferenced local variable.
		return false;
	}
}

inline bool Statistician::CStatistician::Start()
{
	if (m_isInitialized == true && m_stop.load() == true)
	{
		m_stop.store(false);
		m_tstats = std::thread(&Statistician::CStatistician::CheckStatistics, this);
		return true;
	}
	else
		return false;
}

inline bool Statistician::CStatistician::Stop()
{
	m_stop.store(true);
	if (m_tstats.joinable())
		m_tstats.join();

	return true;
}

inline std::string Statistician::CStatistician::PrintStatistics()
{
	try
	{
		std::stringstream().swap(m_strStatistics);
		m_strStatistics << " Statistics:" << std::endl;

		if (m_deviceClass == Pylon::BaslerUsbDeviceClass)
		{
			m_strStatistics << "  Grab Engine:" << std::endl;
			m_strStatistics << "   Total Processed Buffers                : " << totalBuffers << std::endl;
			m_strStatistics << "   Total Failed Buffers                   : " << failedBuffers << std::endl;
			m_strStatistics << "   Last BlockID (starts at 0)             : " << lastBlockID << std::endl;
			m_strStatistics << "   Total Missed Frames                    : " << missedFrameCount << std::endl;
			m_strStatistics << "   Last Failed Buffer Status              : " << lastFailedBufferStatus << std::endl;
			m_strStatistics << "   Last Failed Buffer Status Text         : " << lastFailedBufferStatusText.substr(0, lastFailedBufferStatusText.size() - 1) << std::endl; // remove the newline built into this error message string.
			for (std::set<Pylon::String_t>::iterator it = lastFailedBufferStatusTextList.begin(); it != lastFailedBufferStatusTextList.end(); ++it)
				m_strStatistics << "   Other Failed Buffer Status Text logged : " << *it; // newline is built into this error message string.

			m_strStatistics << "  Transport Layer:" << std::endl;
			m_strStatistics << "   Read Pipe Reset Count                  : " << readPipeResetCount << std::endl;
			m_strStatistics << "   Write Pipe Reset Count                 : " << writePipeResetCount << std::endl;
			m_strStatistics << "   Read Operations Failed Count           : " << readOperationsFailedCount << std::endl;
			m_strStatistics << "   Write Operations Failed Count          : " << writeOperationsFailedCount << std::endl;
			m_strStatistics << "   Last Error Status                      : " << tlLastErrorStatus << std::endl;
			m_strStatistics << "   Last Error Status Text                 : " << tlLastErrorStatusText.substr(0, tlLastErrorStatusText.size() - 1) << std::endl; // remove the newline built into this error message string.
			for (std::set<Pylon::String_t>::iterator it = tlLastErrorStatusTextList.begin(); it != tlLastErrorStatusTextList.end(); ++it)
				m_strStatistics << "   Other Error Status Text logged     : " << *it; // newline is built into this error message string.
			m_strStatistics << "   EPU                                    : " << EPU << std::endl;
			m_strStatistics << "   PE                                     : " << PE << std::endl;
			m_strStatistics << "   UR                                     : " << UR << std::endl;
			m_strStatistics << "   EPR                                    : " << EPR << std::endl;
			m_strStatistics << "   LE                                     : " << LE << std::endl;
			m_strStatistics << "   LR                                     : " << LR << std::endl;
		}

		if (m_deviceClass == Pylon::BaslerGigEDeviceClass)
		{
			m_strStatistics << "  Camera:" << std::endl;
			m_strStatistics << "   Last Error                             : " << lastError.substr(0, tlLastErrorStatusText.size() - 1) << std::endl;
			for (std::set<Pylon::String_t>::iterator it = lastErrorList.begin(); it != lastErrorList.end(); ++it)
				m_strStatistics << "   Other Errors logged                    : " << *it; // newline is built into this error message string.
			m_strStatistics << "  Grab Engine:" << std::endl;
			m_strStatistics << "   Total Frames Received                  : " << totalBuffers << std::endl;
			m_strStatistics << "   Total Failed Buffers                   : " << failedBuffers << std::endl;
			m_strStatistics << "   Total buffer underruns                 : " << bufferUnderruns << std::endl;

			m_strStatistics << "  Transport Layer:" << std::endl;
			m_strStatistics << "   Total Packets Received                 : " << totalPackets << std::endl;
			m_strStatistics << "   Total Packet Resend Requests           : " << resendRequests << std::endl;
			m_strStatistics << "   Total Resent Packets                   : " << resendPackets << std::endl;
			// TODO Issue #22
			// I get failed packets = 0 even when results fail?
			m_strStatistics << "   Total Failed (unrecoverable) Packets   : " << failedPackets << std::endl;
		}

		return m_strStatistics.str();
	}
	catch (GenICam::GenericException& e)
	{
		std::cout << __FUNCTION__ << e.GetDescription() << std::endl; // just to satisfy compilier warning about unreferenced local variable.
		return m_strStatistics.str();
	}
	catch (std::exception& e)
	{
		std::cout << __FUNCTION__ << e.what() << std::endl; // just to satisfy compilier warning about unreferenced local variable.
		return m_strStatistics.str();
	}
}

// *********************************************************************************************************
#endif