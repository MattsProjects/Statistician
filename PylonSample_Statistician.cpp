// PylonSample_.cpp
/*
Note: Before getting started, Basler recommends reading the Programmer's Guide topic
in the pylon C++ API documentation that gets installed with pylon.
If you are upgrading to a higher major version of pylon, Basler also
strongly recommends reading the Migration topic in the pylon C++ API documentation.

*/

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

#include "Statistician.h"

// Namespace for using pylon objects.
using namespace Pylon;

// Namespace for using cout.
using namespace std;

// Number of images to be grabbed.
static const uint32_t c_countOfImagesToGrab = 1000;

int main(int argc, char* argv[])
{
	// The exit code of the sample application.
	int exitCode = 0;

	// Automagically call PylonInitialize and PylonTerminate to ensure the pylon runtime system
	// is initialized during the lifetime of this object.
	Pylon::PylonAutoInitTerm autoInitTerm;

	Statistician::CStatistician myStatistician;

	try
	{
		CDeviceInfo info;
		info.SetSerialNumber("22663088");

		// Create an instant camera object with the camera device found first.
		CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice(info));

		// Print the model name of the camera.
		cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;

		// open the camera so we can configure the physical device itself.
		camera.Open();

		myStatistician.Initialize(camera);

		// set original pixel format from camera
		GenApi::CEnumerationPtr(camera.GetNodeMap().GetNode("PixelFormat"))->FromString("Mono8");

		// This smart pointer will receive the grab result data.
		CGrabResultPtr ptrGrabResult;

		myStatistician.Start();

		// Start the grabbing of c_countOfImagesToGrab images.
		camera.StartGrabbing(c_countOfImagesToGrab);

		while (camera.IsGrabbing())
		{
			// Wait for an image and then retrieve it. A timeout of 5000 ms is used.
			// Camera.StopGrabbing() is called automatically by RetrieveResult() when c_countOfImagesToGrab have been grabbed.
			camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);

			// Image grabbed successfully?
			if (ptrGrabResult->GrabSucceeded())
			{

				uint8_t *pBuffer = (uint8_t *)ptrGrabResult->GetBuffer();

				cout << "Value of first pixel   : " << (uint32_t)pBuffer[0] << endl;

			}
			else
			{
				cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
			}
		}

		myStatistician.Stop();

		// *************************************************************************************************
		// Show Statistics
		//mtx.lock();
		cout  << " Final Statistics:"  << endl;

		if (camera.GetDeviceInfo().GetDeviceClass() == BaslerUsbDeviceClass)
		{
			cout  << "  Grab Engine:"  << endl;
			cout << "   Total Processed Buffers                : " << myStatistician.totalBuffers  << endl;
			cout << "   Total Failed Buffers                   : " << myStatistician.failedBuffers  << endl;
			cout << "   Last BlockID (starts at 0)             : " << myStatistician.lastBlockID << endl;
			cout << "   Total Missed Frames                    : " << myStatistician.missedFrameCount  << endl;
			cout << "   Last Failed Buffer Status              : " << myStatistician.lastFailedBufferStatus << endl;
			cout << "   Last Failed Buffer Status Text         : " << myStatistician.lastFailedBufferStatusText.substr(0, myStatistician.lastFailedBufferStatusText.size() - 1) << endl; // remove the newline built into this error message string.
			for (std::set<Pylon::String_t>::iterator it = myStatistician.lastFailedBufferStatusTextList.begin(); it != myStatistician.lastFailedBufferStatusTextList.end(); ++it)
				cout << "   Other Failed Buffer Status Text logged : " << *it; // newline is built into this error message string.

			cout  << "  Transport Layer:"  << endl;
			cout << "   Read Pipe Reset Count                  : " << myStatistician.readPipeResetCount << endl;
			cout << "   Write Pipe Reset Count                 : " << myStatistician.writePipeResetCount << endl;
			cout << "   Read Operations Failed Count           : " << myStatistician.readOperationsFailedCount << endl;
			cout << "   Write Operations Failed Count          : " << myStatistician.writeOperationsFailedCount << endl;
			cout << "   Last Error Status                      : " << myStatistician.tlLastErrorStatus << endl;
			cout << "   Last Error Status Text                 : " << myStatistician.tlLastErrorStatusText.substr(0, myStatistician.tlLastErrorStatusText.size() - 1) << endl; // remove the newline built into this error message string.
			for (std::set<Pylon::String_t>::iterator it = myStatistician.tlLastErrorStatusTextList.begin(); it != myStatistician.tlLastErrorStatusTextList.end(); ++it)
				cout << "   Other Error Status Text logged         : " << *it; // newline is built into this error message string.
		}

		if (camera.GetDeviceInfo().GetDeviceClass() == BaslerGigEDeviceClass)
		{
			cout  << "  Camera:"  << endl;
			cout << "   Last Error                             : " << myStatistician.lastError.substr(0, myStatistician.tlLastErrorStatusText.size() - 1) << endl;
			for (std::set<Pylon::String_t>::iterator it = myStatistician.lastErrorList.begin(); it != myStatistician.lastErrorList.end(); ++it)
				cout << "   Other Errors logged                    : " << *it; // newline is built into this error message string.
			cout  << "  Grab Engine:"  << endl;
			cout << "   Total Frames Received                  : " << myStatistician.totalBuffers  << endl;
			cout << "   Total Failed Buffers                   : " << myStatistician.failedBuffers  << endl;
			cout << "   Total buffer underruns                 : " << myStatistician.bufferUnderruns  << endl;

			cout  << "  Transport Layer:"  << endl;
			cout << "   Total Packets Received                 : " << myStatistician.totalPackets  << endl;
			cout << "   Total Packet Resend Requests           : " << myStatistician.resendRequests  << endl;
			cout << "   Total Resent Packets                   : " << myStatistician.resendPackets  << endl;
			// TODO Issue #22
			// I get failed packets = 0 even when results fail?
			cout << "   Total Failed (unrecoverable) Packets   : " << myStatistician.failedPackets  << endl;
		}
		// *************************************************************************************************
	}
	catch (GenICam::GenericException &e)
	{
		// Error handling.
		cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;

		myStatistician.Stop();
	}

	// Comment the following two lines to disable waiting on exit.
	cerr << endl << "Press Enter to exit." << endl;
	while (cin.get() != '\n');

	return exitCode;
}
