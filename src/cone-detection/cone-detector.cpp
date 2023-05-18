/*
* AnglePilot - A system that determines steering angles for miniature cars
* Copyright (C) 2023  Robert Einer, Emma Litvin, Ossian Ålund, Bao Quan Lindgren, Khaled Adel Saleh Mohammed Al-Baadani
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see https://www.gnu.org/licenses/. 
*/

//include section
#include <time.h>
#include <chrono>
#include <ctime>
#include <iostream>
// Include the single-file, header-only middleware libcluon to create high-performance microservices
#include "../cluon-complete-v0.0.127.hpp"
// Include the OpenDLV Standard Message Set that contains messages that are usually exchanged for automotive or robotic applications 
#include "opendlv-standard-message-set.hpp"
// include pos-api header file
#include "../api/position.hpp"

// Include the GUI and image processing header files from OpenCV
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


// Define section

/* These are the min and max values for HSV that is used when filtering for blue and yellow color */
// Min HSV values for blue
#define B_MIN_H 90
#define B_MIN_S 100
#define B_MIN_V 23
// Max HSV values for blue
#define B_MAX_H 128 
#define B_MAX_S 179
#define B_MAX_V 255
//Min HSV values for yellow
#define Y_MIN_H 15
#define Y_MIN_S 100
#define Y_MIN_V 120
//Max HSV values for yellow
#define Y_MAX_H 35
#define Y_MAX_S 243 
#define Y_MAX_V 255 

/* Image dimensions */
// Image width
#define IMG_WIDTH_MIN 0
#define IMG_WIDTH_MAX 640
//Image height
#define IMG_HEIGHT_MIN 270
#define IMG_HEIGHT_MAX 400

/* Default setting is that the y value is 0 at the top left corner of the image, this value is to set y as 0 in the bottom left corner */
#define Y_TOTAL 110

// Namespaces
using cv::Mat;
using std::cout;
using std::size_t;
using std::endl;

// Vartiable declaration
int hiThresh = 100;        // for Canny method 
int lowThresh = 50;        // for Canny method  

// Function declaration
/**
 * This method clears the memory upon all termination events, such as ctrl+C or closing the terminal window
 * @param sig the type of termination signal
*/
void handleExit(int sig);

/**
 * Create masks of the original image.
 * The resulting blue/yellow mask is a binary mask which will have 1's where the pixels fall in the range of lower_bound and upper_bound, 
 * meaning the pixels will be white there, and 0 where they don't,
 * which means the pixels will be black there. 
 * @param imgHSV the original image in HSV-format
 * @param lower_bound the lower limit for the color range
 * @param upper_bound the upper limit for the color range
 * @param img_mask the resulting mask
*/
Mat maskImg(Mat imgHSV, cv::Scalar lower_bound, cv::Scalar upper_bound, Mat img_mask);

/**
 * Sorts the vectors holding the contours in descending order using selection sort.
 * @param contours the vector of contours to be sorted
*/
void sortContours(std::vector<std::vector<cv::Point>>& contours);

/**
 * This method iterates through the contours twice, the first time to find the moments of each contour. The second time ot calculates 
 * the centroid of the contour. 
 * @param moms vector to store the moments
 * @param centroids vector to store the centroids
 * @param contours the vector of contours
*/
void findCentroids(std::vector<cv::Moments>& moments, std::vector<cv::Point2f>& centroids, std::vector<std::vector<cv::Point>>& contours);

/**
 * This method puts a rectangle on each cone and then draws a line between the two closest cones.
 * @param contours vector of contours
 * @param centroids the vector with the centroids of the contours
 * @param imgContours the img to draw the contours on
 * @param img the original image where the line and rectangles will be drawm
*/
void drawPath(std::vector<std::vector<cv::Point>>& contours, std::vector<cv::Point2f>& centroids, Mat imgContours, Mat img);


/**
 * This method will populate the cone structs with the x and y coordinates for the two closest cones on one side.
 * @param coneClose the cone closest to the car
 * @param coneFar the cone second closest to the car
 * @param centroids the vector with the centroids of the cones
 * @param contoursSize the size of the vector with the contours
*/
void fillConePositions(pos_api::cone_t& coneClose, pos_api::cone_t& coneFar, std::vector<cv::Point2f> centroids, int contoursSize); 

// main function
int32_t main(int32_t argc, char **argv) {

    int32_t retCode{1};
    // Parse the command line parameters as we require the user to specify some mandatory information on startup.
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
    if ( (0 == commandlineArguments.count("cid")) ||
         (0 == commandlineArguments.count("name")) ||
         (0 == commandlineArguments.count("width")) ||
         (0 == commandlineArguments.count("height")) ) {
        std::cerr << argv[0] << " attaches to a shared memory area containing an ARGB image." << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --cid=<OD4 session> --name=<name of shared memory area> [--verbose]" << std::endl;
        std::cerr << "         --cid:    CID of the OD4Session to send and receive messages" << std::endl;
        std::cerr << "         --name:   name of the shared memory area to attach" << std::endl;
        std::cerr << "         --width:  width of the frame" << std::endl;
        std::cerr << "         --height: height of the frame" << std::endl;
        std::cerr << "Example: " << argv[0] << " --cid=253 --name=img --width=640 --height=480 --verbose" << std::endl;
        return retCode;
    }

    // signal methods to handle termination events such as ctrl + C or closing the terminal window
    signal(SIGINT, handleExit);
    signal(SIGTERM, handleExit);
    signal(SIGQUIT, handleExit);
    signal(SIGHUP, handleExit);

    // Extract the values from the command line parameters
    const std::string NAME{commandlineArguments["name"]};
    const uint32_t WIDTH{static_cast<uint32_t>(std::stoi(commandlineArguments["width"]))};
    const uint32_t HEIGHT{static_cast<uint32_t>(std::stoi(commandlineArguments["height"]))};
    const bool VERBOSE{commandlineArguments.count("verbose") != 0};

    // Attach to the shared memory.
    std::unique_ptr<cluon::SharedMemory> sharedMemory{new cluon::SharedMemory{NAME}};
    if (sharedMemory && sharedMemory->valid()) {
        std::clog << argv[0] << ": Attached to shared memory '" << sharedMemory->name() << " (" << sharedMemory->size() << " bytes)." << std::endl;

        // Interface to a running OpenDaVINCI session where network messages are exchanged.
        // The instance od4 allows you to send and receive messages.
        cluon::OD4Session od4{static_cast<uint16_t>(std::stoi(commandlineArguments["cid"]))};

        // We try to create a shared memory so the two microservices will be able to communicate
        // with eachother through shared memory
        try
        {
            pos_api::create();
        }
        catch (const pos_api::APIException& e)
        {
            switch (e)
            {
                case pos_api::APIException::CREATED:
                    std::cerr << "Shared memory already exists" << std::endl;
                    break;
                default:
                    std::cerr << "Oops! Something went wrong" << std::endl;
            }
            handleExit(0);
        }

        opendlv::proxy::GroundSteeringRequest gsr;
        _Float32 gsrVal;
        std::mutex gsrMutex;
        auto onGroundSteeringRequest = [&gsr, &gsrMutex, &gsrVal](cluon::data::Envelope &&env){
            // The envelope data structure provide further details, such as sampleTimePoint as shown in this microseconds case:
            // https://github.com/chrberger/libcluon/blob/master/libcluon/testsuites/TestEnvelopeConverter.cpp#L31-L40
            std::lock_guard<std::mutex> lck(gsrMutex);
            gsr = cluon::extractMessage<opendlv::proxy::GroundSteeringRequest>(std::move(env));
            gsrVal = gsr.groundSteering();
            //std::cout << "lambda: groundSteering = " << gsr.groundSteering() << std::endl;
            
        };


        od4.dataTrigger(opendlv::proxy::GroundSteeringRequest::ID(), onGroundSteeringRequest);

        // Endless loop; end the program by pressing Ctrl-C.
        while (od4.isRunning()) {

            // OpenCV data structure to hold an image.
            Mat img;
            //create a std::pair class template that stores a boolean and a timestamp
            std::pair<bool, cluon::data::TimeStamp> sampleTimePoint;

            // Wait to receive a notification of a new frame.
            sharedMemory->wait();

            // Lock the shared memory.
            sharedMemory->lock();
            {
                // Copy the pixels from the shared memory into our own data structure.
                // cv::Mat is a 2D matrix where HEIGHT is rows and WIDTH is columns, the pixeldata for the matrix is taken from
                // the shared memory that has been created byt the decoder. so wrapped is a matrix that contains pixeldata of an image stored
                // in a shared memory.
                Mat wrapped(HEIGHT, WIDTH, CV_8UC4, sharedMemory->data());
                // here a clone is made of the wrapped matrix, changes made to img will not affect wrapped and vice cersa
                img = wrapped.clone();

                // call getTimeSTamp method to get current timestamp returned as a std::pair
                sampleTimePoint = sharedMemory->getTimeStamp();
            }
            sharedMemory->unlock();
        
            // convert image to HSV format
            Mat imgHSV;
            cv::cvtColor(img, imgHSV, cv::COLOR_BGR2HSV);

            // Crop original image
            img = img(cv::Range(IMG_HEIGHT_MIN, IMG_HEIGHT_MAX), cv::Range(IMG_WIDTH_MIN, IMG_WIDTH_MAX));

            // The ranges for blue and yellow filtering are set
            cv::Scalar lower_blue = cv::Scalar(B_MIN_H, B_MIN_S, B_MIN_V);
            cv::Scalar upper_blue = cv::Scalar(B_MAX_H, B_MAX_S, B_MAX_V);
            cv::Scalar lower_yellow = cv::Scalar(Y_MIN_H, Y_MIN_S, Y_MIN_V);
            cv::Scalar upper_yellow = cv::Scalar(Y_MAX_H, Y_MAX_S, Y_MAX_V);

            // declare masks for blue and yellow
            Mat blue_mask;
            Mat yellow_mask;

            // declare images that will hold the result of the masking operation
            Mat result_blue, result_yellow; 

            // create masks for blue and yellow
            result_blue = maskImg(imgHSV, lower_blue, upper_blue, blue_mask);
            result_yellow = maskImg(imgHSV, lower_yellow, upper_yellow, yellow_mask);

            // create gray versions of the masked images
            Mat result_blue_gray, result_yellow_gray;
            cv::cvtColor(result_blue, result_blue_gray, cv::COLOR_BGR2GRAY);
            cv::cvtColor(result_yellow, result_yellow_gray, cv::COLOR_BGR2GRAY);

            // Crop images 
            result_blue_gray = result_blue_gray(cv::Range(IMG_HEIGHT_MIN, IMG_HEIGHT_MAX), cv::Range(IMG_WIDTH_MIN, IMG_WIDTH_MAX));
            result_yellow_gray = result_yellow_gray(cv::Range(IMG_HEIGHT_MIN, IMG_HEIGHT_MAX), cv::Range(IMG_WIDTH_MIN, IMG_WIDTH_MAX));
                        
            /** 
             * The Canny method will find edges in an image using the Canny algorithm. It marks the edges and saves them in the gray masks Mat. 
             * The thresholds are used for determining which edges to detect. 
             * @param highThresh is used to identify "strong" edges in the image. Any edge with an intensity gradient magnitude above the threshold will be kept.
             * @param lowThresh  is used to identify weak edges, any edges with an intensity gradient magnitude below loThresh will be discarded.
             * Edges between low and high: any edge pixel that is connected to a strong edge pixel is also considered a strong edge pixel and is retained. 
             */
            cv::Canny(result_blue_gray, result_blue_gray, lowThresh, hiThresh);
            cv::Canny(result_yellow_gray, result_yellow_gray, lowThresh, hiThresh);

            // 2D dynamic arrays to store the points of the contours
            std::vector<std::vector<cv::Point>> contours_blue;
            std::vector<std::vector<cv::Point>> contours_yellow;
            
            // we create a structuring element 5x5 pixels large, with the shape of a rectangle. 
            Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));

            /**
             * This method is used to fill in gaps and holes of the contours in the image to make them more uniform.
             * The structuring element performs this so called closing operation while 
             * preserving the overall shape of the objects. 
             * @param MORPH_CLOSE specifies that we want to do that kind of operation (i.e close the gaps) on the contours. 
             * @param kernel the structuring element to be used
             */
            cv::morphologyEx(result_blue_gray, result_blue_gray, cv::MORPH_CLOSE, kernel);
            cv::morphologyEx(result_yellow_gray, result_yellow_gray, cv::MORPH_CLOSE, kernel);


            /** 
             * Finds the contours of objects in the image. 
             * @param RETR_EXTERNAL retrieves only the extreme outer contours. 
             * @param CHAIN_APPROX_SIMPLE compresses horizontal, vertical, and diagonal segments and leaves only their end points. For example, an up-right rectangular contour is encoded with 4 points.
             */
            cv::findContours(result_blue_gray, contours_blue, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            cv::findContours(result_yellow_gray, contours_yellow, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            
           // sort contours in descending order
            sortContours(contours_blue);
            sortContours(contours_yellow);

            /** 
             * Initializes the imgContours matrices to be the same size as their gray versions and with all pixels set to 0 i.e. black
             * without doing this we get an error saying that the imgContours is not the right size
             */
            Mat imgContours_blue, imgContours_yellow;
            imgContours_blue = Mat::zeros(result_blue_gray.size(), CV_8UC3);
            imgContours_yellow = Mat::zeros(result_yellow_gray.size(), CV_8UC3);
   
            /**
             * We define vectors of Moments called moms, the same size as the contours vectors.
             * Moments are objects that will contain some information about each shape. We can use it to calculate area, orientation, size etc.,
             * in our case we are interested in the centroid = geometric center of the object.
             */ 
             std::vector<cv::Moments> moms_blue(contours_blue.size());
             std::vector<cv::Moments> moms_yellow(contours_yellow.size());
             // declaration of vectors of Points to hold the centroids
             std::vector<cv::Point2f> centroids_blue(contours_blue.size());
             std::vector<cv::Point2f> centroids_yellow(contours_yellow.size());

            // Calculate centroids of each blue and yellow shape
            findCentroids(moms_blue, centroids_blue, contours_blue);
            findCentroids(moms_yellow, centroids_yellow, contours_yellow);

            // declare cone structs to hold the centroids x and y coordinate values of the cones
            pos_api::cone_t bClose{};
            pos_api::cone_t bFar{};
            pos_api::cone_t yClose{};
            pos_api::cone_t yFar{};
            
            // draw rectangles on top of cones as well as lines between them
            drawPath(contours_blue, centroids_blue, imgContours_blue, img);
            drawPath(contours_yellow, centroids_yellow, imgContours_yellow, img);

            // extract x and y coordinate and populate the cone structs with them
            fillConePositions(bClose, bFar, centroids_blue, contours_blue.size()); 
            fillConePositions(yClose, yFar, centroids_yellow, contours_yellow.size()); 

            // call the toMicroseconds function to get the timestamp converted to microseconds. 
            int64_t microseconds = cluon::time::toMicroseconds(sampleTimePoint.second);

            // Get the UNIX timestamp
            int64_t t = cluon::time::toMicroseconds(cluon::time::now());

            // Fill the struct with all the cordinates of the two closest yellow and blue cones, the UNIX timestamp, the video frame timestamp and
            // the original ground steering values 
            pos_api::data_t coneData {
                bClose,
                bFar,
                yClose,
                yFar,
                {t},   
                {microseconds},
                gsrVal   
            };

            // put the cone data into the shared memory to be extracted by the steering calculator microservice
            pos_api::put(coneData);
    //-------------------------------------------------^-----------------------------------------------------

            // If you want to access the latest received ground steering, don't forget to lock the mutex:
            {
                std::lock_guard<std::mutex> lck(gsrMutex);
                ////std::cout << "main: groundSteering = " << gsr.groundSteering() << std::endl;
            }

            // Display images on your screen.
            if (VERBOSE) {
                cv::imshow(sharedMemory->name().c_str(), img);
                cv::waitKey(1);
                cv::namedWindow("Blue", CV_WINDOW_AUTOSIZE);
                cv::namedWindow("Yellow", CV_WINDOW_AUTOSIZE);
                cv::imshow("Blue", imgContours_blue);
                cv::imshow("Yellow", imgContours_yellow);
            }
        }
    }
    retCode = 0;
    
    // free the shared memory
    pos_api::clear();
    return retCode;
}

void handleExit(int sig)
{
    std::clog << std::endl << "Cleaning up..." << std::endl;
    pos_api::clear();
    std::clog << "Exiting programme..." << std::endl;
    exit(0);
}

Mat maskImg(Mat imgHSV, cv::Scalar lower_bound, cv::Scalar upper_bound, Mat img_mask)
{   
    Mat result;      // image to store the result of the operations
    cv::inRange(imgHSV, lower_bound, upper_bound, img_mask);
    
    // comparing the masks to original images. We are using bitwise_and operation on the same images (imgHSV). However, we are including
    // the blue and yellow mask, this means that only the pixels that are set to white in the blue or yellow mask will be evaluated in the coprresponding pixels of the 
    // imgHSV by the bitwise_and operation, the rest of the pixels in the result will automatically be set to black.
    cv::bitwise_and(imgHSV, imgHSV, result, img_mask);
    return result;
}

void sortContours(std::vector<std::vector<cv::Point>>& contours) 
{   
    for(size_t i = 0; i < contours.size(); i++) {
        size_t maxIndex = i;
        double maxArea = cv::contourArea(contours[maxIndex]);
        for(size_t j = i + 1; j < contours.size(); j++) {
            double currentArea = cv::contourArea(contours[j]);
            if(currentArea > maxArea) {
                maxIndex = j;
                maxArea = cv::contourArea(contours[j]);
            }
        }
        if(maxIndex != i) {
            std::vector<cv::Point> temp = contours[i];
            contours[i] = contours[maxIndex];
            contours[maxIndex] = temp;
        }
    }
}

void findCentroids(std::vector<cv::Moments>& moms, std::vector<cv::Point2f>& centroids, std::vector<std::vector<cv::Point>>& contours)
{
    for(size_t i = 0; i < contours.size(); i++) {
        moms[i] = cv::moments(contours[i]);
    }
    // we loop through the moments and perform operations on them to get the centroid values 
    for(size_t i = 0; i < contours.size(); i++) {
            // m00: represents the total area of the contour. 
            // m10: represents the sum of the x coordinates of all the pixels in the shape. 
            // m01: represents the sum of the y coordinatese of all pixels in the shape. 
            // m10/m00 gives us the average x coordinate of the shape which is also the x coordinate of the centroid
            // m01/m00 gives us the average y coordinate of the shape which is also the y coordinate of the centroid  
            // we create a Point out of the centroid x and y coordinates and store in centroids vector
            // m10, m00, and m01 operations return double, so we cast to float
        centroids[i] = cv::Point2f(static_cast<float>(moms[i].m10/moms[i].m00), static_cast<float>(moms[i].m01/moms[i].m00));

    }
}

void drawPath(std::vector<std::vector<cv::Point>>& contours, std::vector<cv::Point2f>& centroids, Mat imgContours, Mat img)
{
    if(contours.size() != 0) {
    // loop through the contours and draw them out on an image, also draws out rectangles around cones and lines between them
        for(size_t i = 0; i < contours.size(); i++) {
            
            // draws the contours out on the imgContours_blue image.
            cv::drawContours(imgContours, contours, (int)i, cv::Scalar(0, 255, 0), 1);
            
            // boundingRect finds the smallest rectangle that completely encloses a given contour or set of points. 
            // boundingRect returns the x and y coordinates of the top left corner as one Point, the width and the height and stores it in a Rect object.
            cv::Rect rectAroundCone = cv::boundingRect(contours[i]);
        
            // We try to ignore the smallest contours spotted by only drawing rectangles for Rects that have width and height > 5 to reduce noise
            if(rectAroundCone.height > 5 && rectAroundCone.width > 5) {
                
                // draw a rectangle with the Rect as base
                cv::rectangle(img, rectAroundCone, cv::Scalar(0, 255, 0), 2);
                
                // Draw lines between the two closest contours
                if(i == 1) {
                    cv::line(img, cv::Point(centroids[i - 1].x, centroids[i - 1].y), cv::Point(centroids[i].x, centroids[i].y), cv::Scalar(0, 0, 255), 2);
                //since we only care about sending data about the closeest two cones, no need to continue loop if i > 1   
                } else if(i > 1) {
                    break;
                }
            }
        }
    }
}

void fillConePositions(pos_api::cone_t& coneClose, pos_api::cone_t& coneFar, std::vector<cv::Point2f> centroids, int contoursSize) 
{
    // if there are atleast two cones visible, enter if block and get x and y coordinates
    if(contoursSize > 1) {
        uint16_t closeX = centroids[0].x;
        uint16_t closeY = Y_TOTAL - centroids[0].y;
        uint16_t farX = centroids[1].x;
        uint16_t farY = Y_TOTAL - centroids[1].y;
        pos_api::cone_t tmpClose{closeX, closeY};
        pos_api::cone_t tmpFar{farX, farY};
        std::memcpy(&coneClose, &tmpClose, sizeof(pos_api::cone_t));
        std::memcpy(&coneFar, &tmpFar, sizeof(pos_api::cone_t));
    // if there are < 2 contours found, send NO_CONE_POS to represent it.   
    } else {
        std::memcpy(&coneClose, &pos_api::NO_CONE_POS, sizeof(pos_api::cone_t));
        std::memcpy(&coneFar, &pos_api::NO_CONE_POS, sizeof(pos_api::cone_t));
    }
}
