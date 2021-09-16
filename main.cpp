#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <math.h>
#include <vector>
#include "udpServer.h"

struct DroneTrackingMessageReponse
{
	cv::Point centerCordinate;
	cv::Point boxPoint1;
	cv::Point boxPoint2;
	int scale;
	std::string clasName;
};

const cv::Scalar SCALAR_BLACK = cv::Scalar(0.0, 0.0, 0.0);
const cv::Scalar SCALAR_WHITE = cv::Scalar(255.0, 255.0, 255.0);
const cv::Scalar SCALAR_BLUE = cv::Scalar(255.0, 0.0, 0.0);
const cv::Scalar SCALAR_GREEN = cv::Scalar(0.0, 200.0, 0.0);
const cv::Scalar SCALAR_RED = cv::Scalar(0.0, 0.0, 255.0);

std::vector<std::string> ProcessResponseMessage(const char* msg, char separator);
DroneTrackingMessageReponse DrawProcessedResult(std::vector<std::string>& parameters);

int main(int argc, char* argv[]) 
{
	std::cout << "start drone project ..." << std::endl;
	std::cout << "Hello Open CV" << std::endl;

	UDPServer udpServer;

	bool result = udpServer.initializeWinSocket();

	if (result)
	{
		result = udpServer.createAndBinSocket(54000);

		if (result) 
		{
			sockaddr_in client;
			int clientLength = sizeof(client);
			char buffer[1024];

			ZeroMemory(&client, clientLength);

			while (true)
			{
				ZeroMemory(buffer, 1024);

				int byteIn = recvfrom(udpServer.listening, buffer, 1024, 0, (sockaddr*)&client, &clientLength);

				if (byteIn == SOCKET_ERROR)
				{
					std::cout << "Error receiving from cleint " << WSAGetLastError() << std::endl;
					continue;
				}

				std::vector<std::string> result = ProcessResponseMessage(buffer, '|');
				
				// converting message 
				DroneTrackingMessageReponse processedResult = DrawProcessedResult(result);

				//--------------- drawing
				cv::Mat droneDetection(720, 1280, CV_8UC3);

				cv::rectangle(droneDetection, processedResult.boxPoint1, processedResult.boxPoint2, SCALAR_RED);
				cv::circle(droneDetection, processedResult.centerCordinate, 15, SCALAR_GREEN, -1);

				cv::imshow("droneDetection", droneDetection);

				if (cv::waitKey(25) >= 0) break;
			}
		}
	}

	udpServer.CloseUDPSocket();

	return(0);

}

std::vector<std::string> ProcessResponseMessage(const char* msg, char separator)
{
	// message format centerX,centerY|boxPoint1X,boxPoint1Y,boxPoint2X,boxPoint2Y|Scale|ClassNAme
	try
	{
		std::vector<std::string> foundItem;

		std::string response(msg);

		if (response.length() > 0)
		{
			do
			{
				std::size_t index = response.find_first_of(separator);

				// find the index
				std::string part = response.substr(0, index);

				foundItem.push_back(part);

				std::size_t indextoRemove = (response.find(separator) != std::string::npos) ? (index + 1) : response.length();

				response.erase(0, indextoRemove);

			} while (response.length() > 0);

			return foundItem;
		}
	}
	catch (const std::exception& ex)
	{
		std::cout << "cannot convert message " << ex.what() << std::endl;
	}

}

DroneTrackingMessageReponse DrawProcessedResult(std::vector<std::string> &parameters) {
	try
	{
		DroneTrackingMessageReponse result;

		if (parameters.size() > 0) {
			for (size_t i = 0; i < parameters.size(); i++)
			{
				std::cout << parameters[i] << " | " << std::endl;
			}

			std::cout << "------------------" << std::endl;

			// get center conrdinate - 0
			std::vector<std::string> section = ProcessResponseMessage(parameters[0].c_str(), ',');
			if (section.size() == 2)
				result.centerCordinate = cv::Point(std::stoi(section[0]), std::stoi(section[1]));

			//get box
			section = ProcessResponseMessage(parameters[1].c_str(), ',');
			if (section.size() == 4) {
				result.boxPoint1 = cv::Point(std::stoi(section[0]), std::stoi(section[1]));
				result.boxPoint2 = cv::Point(std::stoi(section[2]), std::stoi(section[3]));
			}

			result.scale = std::stoi(parameters[2]);

			result.clasName = parameters[3];
		}

		return result;
	}
	catch (const std::exception& ex)
	{
		std::cout << "failed to process and draw image " << ex.what() << std::endl;
	}
}