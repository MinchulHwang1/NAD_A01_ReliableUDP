/*
* FILE				: ReliableUDP.cpp
* PROJECT			: SENG 2040 - Network Application Development - Assignment 1
* PROGRAMMERS		: Cody Glanville (ID: 8864645) and Min Chul Hwang (ID: 8818858)
* FIRST VERSION		: January 31, 2023
* DESCRIPTION		:
*	This file was the initial file used to create the ReliableUDP program. However, we ran into issues with the code
*	and the methods that were utilized instead sent the entire file as a single packet. This would eventually cause problems
*	with large files and would make the program unable to also send images, etc. 
*/


///*
//Reliability and Flow Control Example
//From "Networking for Game Programmers" - http://www.gaffer.org/networking-for-game-programmers
//Author: Glenn Fiedler <gaffer@gaffer.org>
//*/
//
//// Assignment 1 added comment to see if changes are tracked
//
//
//#include <iostream>
//#include <fstream>
//#include <string>
//#include <vector>
//#include "Net.h"
//#include <filesystem>
//
//
//#pragma warning(disable : 4996)
//
////#define SHOW_ACKS
//
//using namespace std;
//using namespace net;
//
//const int ServerPort = 30000;
//const int ClientPort = 30001;
//const int ProtocolId = 0x11223344;
//const float DeltaTime = 1.0f / 30.0f;
//const float SendRate = 1.0f / 30.0f;
//const float TimeOut = 10.0f;
//const int PacketSize = 256;
//
//struct MetaData {
//	char name[128];
//	unsigned char checkSum;
//};
//
//class FlowControl
//{
//public:
//
//	FlowControl()
//	{
//		printf("flow control initialized\n");
//		Reset();
//	}
//
//	void Reset()
//	{
//		mode = Bad;
//		penalty_time = 4.0f;
//		good_conditions_time = 0.0f;
//		penalty_reduction_accumulator = 0.0f;
//	}
//
//	void Update(float deltaTime, float rtt)
//	{
//		const float RTT_Threshold = 250.0f;
//
//		if (mode == Good)
//		{
//			if (rtt > RTT_Threshold)
//			{
//				printf("*** dropping to bad mode ***\n");
//				mode = Bad;
//				if (good_conditions_time < 10.0f && penalty_time < 60.0f)
//				{
//					penalty_time *= 2.0f;
//					if (penalty_time > 60.0f)
//						penalty_time = 60.0f;
//					printf("penalty time increased to %.1f\n", penalty_time);
//				}
//				good_conditions_time = 0.0f;
//				penalty_reduction_accumulator = 0.0f;
//				return;
//			}
//
//			good_conditions_time += deltaTime;
//			penalty_reduction_accumulator += deltaTime;
//
//			if (penalty_reduction_accumulator > 10.0f && penalty_time > 1.0f)
//			{
//				penalty_time /= 2.0f;
//				if (penalty_time < 1.0f)
//					penalty_time = 1.0f;
//				printf("penalty time reduced to %.1f\n", penalty_time);
//				penalty_reduction_accumulator = 0.0f;
//			}
//		}
//
//		if (mode == Bad)
//		{
//			if (rtt <= RTT_Threshold)
//				good_conditions_time += deltaTime;
//			else
//				good_conditions_time = 0.0f;
//
//			if (good_conditions_time > penalty_time)
//			{
//				printf("*** upgrading to good mode ***\n");
//				good_conditions_time = 0.0f;
//				penalty_reduction_accumulator = 0.0f;
//				mode = Good;
//				return;
//			}
//		}
//	}
//
//	float GetSendRate()
//	{
//		return mode == Good ? 30.0f : 10.0f;
//	}
//
//private:
//
//	enum Mode
//	{
//		Good,
//		Bad
//	};
//
//	Mode mode;
//	float penalty_time;
//	float good_conditions_time;
//	float penalty_reduction_accumulator;
//};
//
//// ----------------------------------------------
//unsigned char calculateLRC(const std::string& filePath);
//void sendFileInPieces(const std::string& fileName, Connection& connection);
//void writePacketToFile(const unsigned char* packet, int packetSize, const std::string& fileName);
//
//int main(int argc, char* argv[])
//{
//	bool statusOfSendingData = false;			// the flag to check if the file contents are sent or not
//	// parse command line
//
//	enum Mode
//	{
//		Client,
//		Server
//	};
//
//	Mode mode = Server;
//	Address address;
//
//	int initialSetup = 0;
//
//
//	if (argc >= 2)
//	{
//		int a, b, c, d;
//
//#pragma warning(suppress : 4996)
//		if (sscanf(argv[1], "%d.%d.%d.%d", &a, &b, &c, &d))
//		{
//			mode = Client;
//			address = Address(a, b, c, d, ServerPort);
//		}
//	}
//
//	// NEW COMAMND-LINE ARGUMENTS AND FILE CHECK -------------------------------------------------------------------------------------------------
//
//	// This is where we would implement new logic to determine any command line arguments. This should be done early on in main
//	// And will most likely include a specific file path to a file that will be eventually broken fown and set
//
//	// Variables used that will hold the name of the client's file and the checksum to be used
//
//	char* sendingFile = NULL;
//	unsigned char clientLRC = 0;
//
//
//	if (argc == 3 && mode == Client)
//	{
//		sendingFile = argv[2];			// Store the file name into sending file
//
//		ifstream file;
//
//		// Check file is exists by trying to open it using an ifstream object
//		file.open(sendingFile);
//
//		// ERROR Output (If the file does not exist and open properly, the program will exit and prompt the user (Return 0 ends main)
//		if (!file)
//		{
//			printf("The file does not exist\n");
//			return 0;
//		}
//
//		file.close();
//
//		// Get the LRC Checksum (will be used for checking file integrity once the transfer is complete)
//
//		clientLRC = calculateLRC(sendingFile);
//
//		//https://www.scaler.com/topics/cpp-check-if-the-file-exists/ Method used for determining if the file exists
//
//		//std::cout << "LRC for file '" << sendingFile << "': " << static_cast<int>(lrc) << std::endl;
//	}
//	// ----------------------------------------------------------------------------------------------------------------------------------------------
//
//	// initialize
//
//	if (!InitializeSockets())
//	{
//		printf("failed to initialize sockets\n");
//		return 1;
//	}
//
//	ReliableConnection connection(ProtocolId, TimeOut);
//
//	const int port = mode == Server ? ServerPort : ClientPort;
//
//	if (!connection.Start(port))
//	{
//		printf("could not start connection on port %d\n", port);
//		return 1;
//	}
//
//	// RETRIEVING FILE FROM DISK -------------------------------------------------------------------------------------------------
//
//	/* Since the client will be sending the file to the server, the client can utilize the additional command - line arugment to include a
//	*  a filename that will be sent. It is at this point in the program we would take in the file from disk. The file will be used by the client
//	*  to break up into separate packets that will be sent to the server.
//	*/
//
//	// ----------------------------------------------------------------------------------------------------------------
//
//	if (mode == Client)
//		connection.Connect(address);
//	else
//		connection.Listen();
//
//
//	bool connected = false;
//	float sendAccumulator = 0.0f;
//	float statsAccumulator = 0.0f;
//
//	FlowControl flowControl;
//
//	char copyOfPackets[PacketSize];
//
//
//	// send and receive packets
//
//		// CLIENT SENDING METADATA (BEFORE FILE BROKEN INTO PIECES) ----------------------------------------------------------------------------------------
//
//		/*
//		*   This is where we would send the files metadata. If we are a client, we would get new variables determining the file size, etc.
//		*   After we get the file metadata, it would be broken into pieces before it would be sent. It is important to understand the file
//		*	sizes and sequence so we can understand data integrity at the end of the entire transaction as the server.
//		*/
//
//	if (mode == Client)
//	{
//		unsigned char metaDataPacket[PacketSize];
//		MetaData data;
//		strcpy(data.name, sendingFile);
//		data.checkSum = clientLRC;
//
//		memcpy(metaDataPacket, &data, sizeof(data));
//
//		connection.SendPacket(metaDataPacket, sizeof(metaDataPacket));
//
//	}
//
//	// --------------------------------------------------------------------------------------------------------------------------
//
//	while (true)
//	{
//		// update flow control
//
//		if (connection.IsConnected())
//			flowControl.Update(DeltaTime, connection.GetReliabilitySystem().GetRoundTripTime() * 1000.0f);
//
//		const float sendRate = flowControl.GetSendRate();
//
//		// detect changes in connection state
//
//		if (mode == Server && connected && !connection.IsConnected())
//		{
//			flowControl.Reset();
//			printf("reset flow control\n");
//			connected = false;
//		}
//
//		if (!connected && connection.IsConnected())
//		{
//			printf("client connected to server\n");
//			connected = true;
//		}
//
//		if (!connected && connection.ConnectFailed())
//		{
//			printf("connection failed\n");
//			break;
//		}
//
//
//		// SERVER RECIEVES METADATA -------------------------------------------------------------------------------------------------------------------------
//
//		/*
//		*   This will be where the server takes in the file metadata that was sent from the client. The server will need to know the metadata of the file
//		*	in order to determine the data integrity after the data transaction between the client - server interaction. This will ultimately be a smaller,
//		*	separate data transaction before the file contents are sent and received.
//		*/
//
//		// Declare two variables for the servers checksum and the name the server will check
//		MetaData serversMeta;
//
//		if (mode == Server && initialSetup == 0)
//		{
//			unsigned char recievedMetaData[PacketSize];
//			int metaDataBytesRead = connection.ReceivePacket(recievedMetaData, sizeof(recievedMetaData));
//
//			if (metaDataBytesRead > 0)
//			{
//				MetaData* recievedMetaDataPtr = reinterpret_cast<MetaData*>(recievedMetaData);
//
//				printf("Server File Name: %s\n", recievedMetaDataPtr->name);
//				printf("Server File LRC Checksum: %u\n", recievedMetaDataPtr->checkSum);
//
//				strncpy(serversMeta.name, recievedMetaDataPtr->name, sizeof(recievedMetaDataPtr));
//				serversMeta.checkSum = recievedMetaDataPtr->checkSum;
//
//				initialSetup++;
//			}
//
//		}
//
//		// --------------------------------------------------------------------------------------------------------------------------
//
//		sendAccumulator += DeltaTime;
//
//		while (sendAccumulator > 1.0f / sendRate)
//		{
//			unsigned char packet[PacketSize];
//
//			// BREAKING AND SENDING FILE DATA PIECES ---------------------------------------------------------------------------------------------------------------------
//
//			/*
//			*  We would send the data pieces through the memset() method below. Each packet would include the data of the file being transferred.
//			*  As of right now, I would assume this is in binary. Each separate part of the file will be an individual packet which will be sent
//			*  with a sequence number (to put them back together).
//			*/
//			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//			// Sending content as packet.
//			if (sendingFile != NULL) {
//				//sendFileInPieces(sendingFile, connection);
//
//				std::ifstream file(sendingFile, std::ios::binary);
//
//				if (!file.is_open()) {
//					std::cerr << "Failed to open file!" << std::endl;
//				}
//
//				// file does not close when it meets end of file.
//
//				char buffer[PacketSize];
//				while (!file.eof()) {
//					file.read(buffer, PacketSize);
//					int bytesRead = file.gcount();
//					connection.SendPacket(reinterpret_cast<unsigned char*>(buffer), bytesRead);
//				}
//
//				if (file.eof()) {
//					statusOfSendingData = true;
//				}
//				file.close();
//			}
//			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//			// -----------------------------------------------------------------------------------------------------------------------------
//			memset(packet, 0, sizeof(packet));
//			connection.SendPacket(packet, sizeof(packet));
//
//			sendAccumulator -= 1.0f / sendRate;
//		}
//
//		while (true)
//		{
//			unsigned char packet[256];
//
//
//			/* RECEIVING FILE PIECES --------------------------------------------------------------------------------------------------------------------
//			*
//			*  This is where we would recieve the file pieces using the ReceivePacket() function. This will take in the piece of the file and its
//			*  and with its header, it can be properly pieced together. When receiving each packet, we can also determine the sequence that will be
//			*  used to set the packets in order.
//			*/
//
//
//			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//			std::string fileName = "./receiveFile.txt";					// This is a sample of the names of the files we receive from our server. We will need to change this later. 
//
//			// Write data into file from packet
//			// but it will go trash value. need to fix
//			if (statusOfSendingData) {
//				writePacketToFile(packet, sizeof(packet), fileName);
//			}
//			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//			// --------------------------------------------------------------------------------------------------------------------------
//
//			int bytes_read = connection.ReceivePacket(packet, sizeof(packet));
//#pragma warning(suppress : 4996)
//			strcpy((char*)copyOfPackets, (char*)packet);
//
//			if (bytes_read == 0)
//				break;
//
//			/* WRITE FILE CONTENTS TO DISK (One package piece at a time) ---------------------------------------------------------------------------------
//			*
//			*  Each time we take in a new packet, we can also write the packet to disk. This will be written to the new file in increments. Each time
//			*  we get new file information, we would write the information to the file. This is continued until no bytes are read, so we can put it
//			*  after the bytes_read check to break out.
//			*/
//
//			// ----------------------------------------------------------------------------------------------------------------------------
//		}
//
//		// show packets that were acked this frame
//
//#ifdef SHOW_ACKS
//		unsigned int* acks = NULL;
//		int ack_count = 0;
//		connection.GetReliabilitySystem().GetAcks(&acks, ack_count);
//		if (ack_count > 0)
//		{
//			printf("acks: %d", acks[0]);
//			for (int i = 1; i < ack_count; ++i)
//				printf(",%d", acks[i]);
//			printf("\n");
//		}
//#endif
//
//		// DETERMINE FILE INTEGRITY AS THE SERVER --------------------------------------------------------------------------------------------------------
//
//		/*
//		*  This is where we will determine the files integrity. This will need to be done after the entire file has been received. We would then determine
//		*  if the file size is the same and the metadata matches up, etc.
//		*/
//
//		// --------------------------------------------------------------------------------------------------------------------------------------
//
//		// update connection
//
//		connection.Update(DeltaTime);
//
//		// show connection stats
//
//		statsAccumulator += DeltaTime;
//
//		while (statsAccumulator >= 0.25f && connection.IsConnected())
//		{
//			float rtt = connection.GetReliabilitySystem().GetRoundTripTime();
//
//			unsigned int sent_packets = connection.GetReliabilitySystem().GetSentPackets();
//			unsigned int acked_packets = connection.GetReliabilitySystem().GetAckedPackets();
//			unsigned int lost_packets = connection.GetReliabilitySystem().GetLostPackets();
//
//			float sent_bandwidth = connection.GetReliabilitySystem().GetSentBandwidth();
//			float acked_bandwidth = connection.GetReliabilitySystem().GetAckedBandwidth();
//
//
//			printf("rtt %.1fms, sent %d, acked %d, lost %d (%.1f%%), sent bandwidth = %.1fkbps, acked bandwidth = %.1fkbps\n",
//				rtt * 1000.0f, sent_packets, acked_packets, lost_packets,
//				sent_packets > 0.0f ? (float)lost_packets / (float)sent_packets * 100.0f : 0.0f,
//				sent_bandwidth, acked_bandwidth);
//
//			statsAccumulator -= 0.25f;
//		}
//
//		net::wait(DeltaTime);
//	}
//
//	ShutdownSockets();
//
//	return 0;
//}
//
//unsigned char calculateLRC(const std::string& filePath) {
//	std::ifstream file(filePath, std::ios::binary);
//
//	if (!file.is_open()) {
//		std::cerr << "Error opening file: " << filePath << std::endl;
//		return 0; // You might want to handle this error case differently
//	}
//
//	unsigned char lrc = 0;
//	char byte;
//
//	while (file.get(byte)) {
//		lrc ^= byte;
//	}
//
//	file.close();
//
//	return lrc;
//}
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///*  -- Function Header Comment
//	Name	: sendFileInPieces
//	Purpose : Truncates the content to fit the buffer size, stores it in a packet, and sends it as a packet. The problem here is that the file should terminate when it's finished reading, but it doesn't.
//	Inputs	: const string&		fileName		the file name
//			  Connection&		connection		Variables that store connection state, such as IPs.
//	Outputs	: NONE
//	Returns	: NONE
//*/
//void sendFileInPieces(const std::string& fileName, Connection& connection) {
//	std::ifstream file(fileName, std::ios::binary);
//
//	if (!file.is_open()) {
//		std::cerr << "Failed to open file!" << std::endl;
//		return;
//	}
//
//	// file does not close when it meets end of file.
//
//	char buffer[PacketSize];
//	while (!file.eof()) {
//		file.read(buffer, PacketSize);
//		int bytesRead = file.gcount();
//		connection.SendPacket(reinterpret_cast<unsigned char*>(buffer), bytesRead);
//	}
//
//	file.close();
//}
//
//
///*  -- Function Header Comment
//	Name	: writePacketToFile
//	Purpose : Change the content received in the packet and write it to a file.
//	Inputs	: const unsigned char*		packet			a packet value
//			  int&						packetSize		size of packet
//			  const string&				fileName		the file name
//	Outputs	: NONE
//	Returns	: NONE
//*/
//void writePacketToFile(const unsigned char* packet, int packetSize, const std::string& fileName) {
//	std::ofstream file(fileName, std::ios::binary | std::ios::app); // Open a file in continuation mode
//
//	if (!file.is_open()) {
//		std::cerr << "Failed to open file for writing!" << std::endl;
//		return;
//	}
//
//	file.write(reinterpret_cast<const char*>(packet), packetSize); // Log packet data to a file
//
//	file.close();
//}