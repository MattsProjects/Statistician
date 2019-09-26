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
		std::thread m_tstats;
		std::atomic<bool> m_stop;
		bool CheckStatistics();

	public:
		CStatistician(Pylon::CInstantCamera &camera);
		~CStatistician();
		bool Start();
		bool Stop();

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
	};
}

// *********************************************************************************************************
// DEFINITIONS

Statistician::CStatistician::CStatistician(Pylon::CInstantCamera &camera)
{
	m_camera = &camera;
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
}

Statistician::CStatistician::~CStatistician()
{
	this->Stop();
}

bool Statistician::CStatistician::CheckStatistics()
{
	try
	{
		if (m_camera->IsOpen())
		{
			GenApi::INodeMap &control = m_camera->GetNodeMap();
			GenApi::INodeMap &grabber = m_camera->GetStreamGrabberNodeMap();
			GenApi::INodeMap &transport = m_camera->GetTLNodeMap();
			Pylon::String_t deviceClass = m_camera->GetDeviceInfo().GetDeviceClass();

			GenApi::CIntegerPtr ptrInteger;
			GenApi::CStringPtr ptrString;
			GenApi::CEnumerationPtr ptrEnumeration;
			GenApi::CBooleanPtr ptrBool;

			while (m_stop.load() == false)
			{
				ptrInteger = grabber.GetNode("Statistic_Last_Failed_Buffer_Status");
				if (GenApi::IsReadable(ptrInteger))
					lastFailedBufferStatus = ptrInteger->GetValue();

				ptrInteger = grabber.GetNode("Statistic_Total_Buffer_Count");
				if (GenApi::IsReadable(ptrInteger))
					totalBuffers = ptrInteger->GetValue();

				ptrInteger = grabber.GetNode("Statistic_Failed_Buffer_Count");
				if (GenApi::IsReadable(ptrInteger))
					failedBuffers = ptrInteger->GetValue();

				ptrInteger = grabber.GetNode("TemperatureAbs");
				if (GenApi::IsReadable(ptrInteger))
					currentTemperature = ptrInteger->GetValue();

				ptrInteger = transport.GetNode("Statistic_Last_Error_Status");
				if (GenApi::IsReadable(ptrInteger))
					tlLastErrorStatus = ptrInteger->GetValue();

				GenApi::CStringPtr ptrTLLastErrorStatusText;
				ptrTLLastErrorStatusText = transport.GetNode("Statistic_Last_Error_Status_Text");
				if (GenApi::IsReadable(ptrTLLastErrorStatusText))
				{
					tlLastErrorStatusText = ptrTLLastErrorStatusText->GetValue();
					if (tlLastErrorStatusText == "")
						tlLastErrorStatusText = "(no error)\n";
					else
						tlLastErrorStatusTextList.insert(tlLastErrorStatusText);
				}

				ptrString = grabber.GetNode("Statistic_Last_Failed_Buffer_Status_Text");
				if (GenApi::IsReadable(ptrString))
				{
					lastFailedBufferStatusText = ptrString->GetValue();
					if (lastFailedBufferStatusText == "")
						lastFailedBufferStatusText = "(no error)\n";
					else
						lastFailedBufferStatusTextList.insert(lastFailedBufferStatusText);
				}

				ptrEnumeration = grabber.GetNode("LastError");
				if (GenApi::IsReadable(ptrEnumeration))
				{
					lastError = ptrEnumeration->ToString();
					lastErrorList.insert(lastError);
				}

				ptrEnumeration = grabber.GetNode("TemperatureState");
				if (GenApi::IsReadable(ptrEnumeration))
					temperatureState = ptrEnumeration->ToString();

				ptrBool = grabber.GetNode("CriticalTemperature");
				if (GenApi::IsReadable(ptrBool))
					criticalTemp = ptrBool->GetValue();

				ptrBool = grabber.GetNode("OverTemperature");
				if (GenApi::IsReadable(ptrBool))
					overTemp = ptrBool->GetValue();

				if (deviceClass == Pylon::BaslerGigEDeviceClass)
				{
					ptrInteger = grabber.GetNode("Statistic_Buffer_Underrun_Count");
					if (GenApi::IsReadable(ptrInteger))
						bufferUnderruns = ptrInteger->GetValue();

					ptrInteger = grabber.GetNode("Statistic_Total_Packet_Count");
					if (GenApi::IsReadable(ptrInteger))
						totalPackets = ptrInteger->GetValue();

					ptrInteger = grabber.GetNode("Statistic_Resend_Request_Count");
					if (GenApi::IsReadable(ptrInteger))
						resendRequests = ptrInteger->GetValue();

					ptrInteger = grabber.GetNode("Statistic_Resend_Packet_Count");
					if (GenApi::IsReadable(ptrInteger))
						resendPackets = ptrInteger->GetValue();

					ptrInteger = grabber.GetNode("Statistic_Failed_Packet_Count");
					if (GenApi::IsReadable(ptrInteger))
						failedPackets = ptrInteger->GetValue();
				}

				if (deviceClass == Pylon::BaslerUsbDeviceClass)
				{
					ptrInteger = grabber.GetNode("Statistic_Missed_Frame_Count");
					if (GenApi::IsReadable(ptrInteger))
						missedFrameCount = ptrInteger->GetValue();

					ptrInteger = grabber.GetNode("Statistic_Resynchronization_Count");
					if (GenApi::IsReadable(ptrInteger))
						resyncCount = ptrInteger->GetValue();

					ptrInteger = grabber.GetNode("Statistic_Last_Block_Id");
					if (GenApi::IsReadable(ptrInteger))
						lastBlockID = ptrInteger->GetValue();

					ptrInteger = transport.GetNode("Statistic_Read_Pipe_Reset_Count");
					if (GenApi::IsReadable(ptrInteger))
						readPipeResetCount = ptrInteger->GetValue();

					ptrInteger = transport.GetNode("Statistic_Write_Pipe_Reset_Count");
					if (GenApi::IsReadable(ptrInteger))
						writePipeResetCount = ptrInteger->GetValue();

					ptrInteger = transport.GetNode("Statistic_Read_Operations_Failed_Count");
					if (GenApi::IsReadable(ptrInteger))
						readOperationsFailedCount = ptrInteger->GetValue();

					ptrInteger = transport.GetNode("Statistic_Write_Operations_Failed_Count");
					if (GenApi::IsReadable(ptrInteger))
						writeOperationsFailedCount = ptrInteger->GetValue();
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

bool Statistician::CStatistician::Start()
{
	m_stop.store(false);
	m_tstats = std::thread(&Statistician::CStatistician::CheckStatistics, this);
	return true;
}

bool Statistician::CStatistician::Stop()
{
	m_stop.store(true);
	if (m_tstats.joinable())
		m_tstats.join();

	return true;
}

// *********************************************************************************************************
#endif