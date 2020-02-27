//Kapil Varma W1483983 Programming Assignment 1 -> Client using customized protocol on top of UDP protocol for sending information to the server.

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/time.h>
#include <stdint.h>
#include <string.h>

//Primitives
#define PORT 8080
#define STARTPACKETID 0XFFFF
#define ENDPACKETID 0XFFFF
#define CLIENTID 0XFF
#define TIMEOUT 5

//Packet Types
#define DATATYPE 0XFFF1
#define ACKPACKETTYPE 0XFFF2
#define REJECTCODETYPE 0XFFF3

//Reject Sub codes
#define OUTOFSEQUENCECODE 0XFFF4
#define LENGTHMISMATCHCODE 0XFFF5
#define ENDOFPACKETMISSINGCODE 0XFFF6
#define DUPLICATECODE 0XFFF7

//Data Packet Definition
struct DataPacket{
	uint16_t startPacketID;
	uint8_t clientID;
	uint16_t  data;
	uint8_t segmentNumber;
	uint8_t length;
	char payload[255];
	uint16_t endPacketID;
};

//Structure For Reject Packet
struct RejectPacket {
	uint16_t startPacketID;
	uint8_t clientID;
	uint16_t  data;
	uint16_t subCode;
	uint8_t segmentNumber;
	uint16_t endPacketID;
};

//Setting the data packet that has to be sent to server
struct DataPacket SetDataPacket() {
	struct DataPacket dp;
	dp.startPacketID = STARTPACKETID;
	dp.clientID = CLIENTID;
	dp. data = DATATYPE;
	dp.endPacketID = ENDPACKETID;
	return dp;
}

// Function to print the data packet
void show(struct DataPacket dP) {
  printf(" \n ******* CLIENT SENDING ******* \n");
	printf("START PACKET ID : %x\n", dP.startPacketID);
	printf("CLIENT ID : %hhx\n", dP.clientID);
	printf("DATA : %x\n", dP.data);
	printf("SEGMENT NUMBER : %d \n", dP.segmentNumber);
	printf("LENGTH : %d\n", dP.length);
	printf("PAYLOAD : %s\n", dP.payload);
	printf("END DATA PACKET ID : %x\n", dP.endPacketID);
}

int main(){

	struct DataPacket dataPacket;
	struct RejectPacket receivedPacket;
	struct sockaddr_in clientAddress;
	socklen_t addr_size;
	FILE *filePointer; //to access file
	char line[255];
	int sockfd;
	int n = 0;
	int counter = 0;
	int segmentNum = 1;

  //Create and Verify Socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0) {
		printf("*******SOCKET CREATION FAILED*******\n");
	}
  //Assign IP, PORT
	bzero(&clientAddress, sizeof(clientAddress));
	clientAddress.sin_family = AF_INET;
	clientAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	clientAddress.sin_port=htons(PORT);
	addr_size = sizeof clientAddress;

	//Timeout As required: 3 sec
	struct timeval timeValue;
	timeValue.tv_sec = TIMEOUT;
	timeValue.tv_usec = 0;

	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeValue, sizeof(struct timeval));
	dataPacket = SetDataPacket();

  //getting data from input file
	filePointer = fopen("input.txt", "rt");
	if(filePointer == NULL)
	{
		printf("*******FILE CANNOT BE OPENED*******\n");
		exit(0);
	}

  //Setting the data from the input file
	while (fgets(line, sizeof(line), filePointer) != NULL) {
		n = 0;
		counter = 0;
		dataPacket.segmentNumber = segmentNum;
		strcpy(dataPacket.payload,line);
		dataPacket.length = strlen(dataPacket.payload);
    //Setting values to show error types
		if (segmentNum == 8) {
			dataPacket.length++;
		}
		if (segmentNum == 10) {
			dataPacket.segmentNumber = dataPacket.segmentNumber + 4;
		}
		if (segmentNum == 7) {
			dataPacket.segmentNumber = 3;
		}
		if (segmentNum == 9) {
			dataPacket.endPacketID = 0;
		}
		if (segmentNum != 9) {
			dataPacket.endPacketID = ENDPACKETID;
		}

		show(dataPacket);

		while (n <= 0 && counter < 3) {
      //sending data to server
			sendto(sockfd,&dataPacket,sizeof(struct DataPacket),0,(struct sockaddr *)&clientAddress,addr_size);
      //recieving response
			n = recvfrom(sockfd,&receivedPacket,sizeof(struct RejectPacket),0,NULL,NULL);

			if (n <= 0 ) {
				printf("******* Response not recived, trying again *******\n");
				counter ++;
			}
			else if (receivedPacket.data == ACKPACKETTYPE  ) {
				printf("******* Received ACK from server *******\n \n");

			}
			else if (receivedPacket.data == REJECTCODETYPE ) {
				printf("******* Reject Packet: Packet Rejected *******\n");
				printf("The Type is: %x \n" , receivedPacket.subCode);
				if (receivedPacket.subCode == LENGTHMISMATCHCODE ) {
					printf("LENGTH MISMATCH\n");
				}
				else if (receivedPacket.subCode == ENDOFPACKETMISSINGCODE ) {
					printf("END OF PACKET MISSING\n");
				}
				else if (receivedPacket.subCode == OUTOFSEQUENCECODE ) {
					printf("OUT OF SEQUENCE\n");
				}
				else if (receivedPacket.subCode == DUPLICATECODE) {
					printf("DUPLICATE PACKET RECIEVED\n");
				}
			}
		}
    //No resp
		if (counter >= 3 ) {
			printf("******* SERVER NOT RESPONDING *******");
			exit(0);
		}
		segmentNum++;
		printf("\n --------------------------- NEXT PACKET STARTS HERE---------------------------  \n");
	}
}
