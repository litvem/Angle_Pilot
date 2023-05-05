/*
 * Copyright (C) 2020  Christian Berger
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <time.h>
#include <chrono>
#include <ctime>
#include <iostream>
// Include the single-file, header-only middleware libcluon to create high-performance microservices
#include "cluon-complete.hpp"
// Include the OpenDLV Standard Message Set that contains messages that are usually exchanged for automotive or robotic applications 
#include "opendlv-standard-message-set.hpp"
// include pos-api header file
#include "../api/position.hpp"

// Include the GUI and image processing header files from OpenCV
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;


// Function declaration
void handleExit(int sig);


int32_t main(int32_t argc, char **argv) {

//---------------------------------------
    signal(SIGINT, handleExit);
//---------------------------------------

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
    }
    else {
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

    /*-------------------------------------------- v ---------------------------------------------*/

            // We try to create a shared memory so the two microservices will be able to communicate
            // with eachother
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
             // Create an OpenCV image header using the data in the shared memory.
            IplImage *iplimage{nullptr};
            CvSize size;
            size.width = WIDTH;
            size.height = HEIGHT;

            iplimage = cvCreateImageHeader(size, IPL_DEPTH_8U, 4 /* four channels: ARGB */);
            sharedMemory->lock();
            {
                iplimage->imageData = sharedMemory->data();
                iplimage->imageDataOrigin = iplimage->imageData;
            }
            sharedMemory->unlock();

           // cv::namedWindow("Inspector", CV_WINDOW_AUTOSIZE);
            int minH{0};
            int maxH{179};
            cvCreateTrackbar("Hue b (min)", "Inspector", &minH, 179);
            cvCreateTrackbar("Hue b (max)", "Inspector", &maxH, 179);

            int minS{0};
            int maxS{255};
            cvCreateTrackbar("Sat b (min)", "Inspector", &minS, 255);
            cvCreateTrackbar("Sat b (max)", "Inspector", &maxS, 255);

            int minV{0};
            int maxV{255};
            cvCreateTrackbar("Val b (min)", "Inspector", &minV, 255);
            cvCreateTrackbar("Val b (max)", "Inspector", &maxV, 255);

             int minHdos{0};
            int maxHdos{179};
            cvCreateTrackbar("Hue y (min)", "Inspector", &minHdos, 179);
            cvCreateTrackbar("Hue y (max)", "Inspector", &maxHdos, 179);

            int minSdos{0};
            int maxSdos{255};
            cvCreateTrackbar("Sat y (min)", "Inspector", &minSdos, 255);
            cvCreateTrackbar("Sat y (max)", "Inspector", &maxSdos, 255);

            int minVdos{0};
            int maxVdos{255};
            cvCreateTrackbar("Val y (min)", "Inspector", &minVdos, 255);
            cvCreateTrackbar("Val y (max)", "Inspector", &maxVdos, 255);
           

/*--------------------------------------------- ^ --------------------------------------------*/


            opendlv::proxy::GroundSteeringRequest gsr;
            std::mutex gsrMutex;
            auto onGroundSteeringRequest = [&gsr, &gsrMutex](cluon::data::Envelope &&env){
                // The envelope data structure provide further details, such as sampleTimePoint as shown in this microseconds case:
                // https://github.com/chrberger/libcluon/blob/master/libcluon/testsuites/TestEnvelopeConverter.cpp#L31-L40
                std::lock_guard<std::mutex> lck(gsrMutex);
                gsr = cluon::extractMessage<opendlv::proxy::GroundSteeringRequest>(std::move(env));
                std::cout << "lambda: groundSteering = " << gsr.groundSteering() << std::endl;
            };

            int frameCount = 0;        // to count frames
            bool clockwise = true;     // to check if we are going clockwise or not
            int hiThresh = 100;        // for threshold method 
            int lowThresh = 50;        // for threshold method  
            int threshValue = 120;     // the threshold value for the threshold() method 
            int maxValue = 255;        // max value for the threshold() method 

            od4.dataTrigger(opendlv::proxy::GroundSteeringRequest::ID(), onGroundSteeringRequest);

            // Endless loop; end the program by pressing Ctrl-C.
            while (od4.isRunning()) {
                // OpenCV data structure to hold an image.
                cv::Mat img;
		        std::pair<bool, cluon::data::TimeStamp> sampleTimePoint;
/*-------------------------------- v ---------------------------------------------------------*/
                // for the sliders to play with HSV values
                cv::Mat inspectorImg;
/*---------------------------------- ^-------------------------------------------------------*/
                // test git
                // Wait to receive a notification of a new frame.
                sharedMemory->wait();

                // Lock the shared memory.
                sharedMemory->lock();
                {
                   
                    // Copy the pixels from the shared memory into our own data structure.
                    // cv::Mat is a 2D matrix where HEIGHT is rows and WIDTH is columns, the pixeldata for the matrix is taken from
                    // the shared memory that has been created byt the decoder. so wrapped is a matrix that contains pixeldata of an image stored
                    // in a shared memory.
                    cv::Mat wrapped(HEIGHT, WIDTH, CV_8UC4, sharedMemory->data());
                    // here a clone is made of the wrapped matrix, changes made to img will not affect wrapped and vice cersa
                    img = wrapped.clone();

/*------------------------------------------- v ----------------------------------------------*/
                    /* CREATE TIMESTAMP */
                    //create a std::pair class template that stores a boolean and a timestamp
                    // std::pair<bool, cluon::data::TimeStamp> sampleTimePoint;
                    // call getTimeSTamp method to get current timestamp returned as a std::pair
                    sampleTimePoint = sharedMemory->getTimeStamp();

                     // Copy image into cvMat structure.
                     // Be aware of that any code between lock/unlock is blocking
                     // the camera to provide the next frame. Thus, any
                     // computationally heavy algorithms should be placed outside
                     // lock/unlock.
                    inspectorImg = cv::cvarrToMat(iplimage);
/*----------------------------------------- ^------------------------------------------------*/

                }
                sharedMemory->unlock();
            
                // Crop original image
                img = img(Range(290, 400), Range(0, 640));
            
                // convert image to HSV format
                cv::Mat imgHSV;
                cvtColor(inspectorImg, imgHSV, cv::COLOR_BGR2HSV);

                // The ranges for blue and yellow filtering are set
                cv::Scalar lower_blue = cv::Scalar(90, 100, 23);
                cv::Scalar upper_blue = cv::Scalar(128, 179, 255);
                cv::Scalar lower_yellow = cv::Scalar(15, 100, 120);
                cv::Scalar upper_yellow = cv::Scalar(35, 243, 255);

                /**
                 * Filter by blue
                 * The resulting blue_mask is a binary mask which will have 1's where the pixels fall in the range of lower_blue and upper_blue, meaning the pixels will be white there, and 0 where they don't
                 *, which means the pixels will be black
                 */
                cv::Mat blue_mask;
                cv::inRange(imgHSV, lower_blue, upper_blue, blue_mask);

                // filter by yellow, same procedure as for blue
                cv::Mat yellow_mask;
                cv::inRange(imgHSV, lower_yellow, upper_yellow, yellow_mask);


                /**
                 * comparing the masks to original images. We are using bitwise_and operation on the same images (imgHSV). However, we are including
                 * the blue and yellow mask, this means that only the pixels that are set to white in the blue or yellow mask will be evaluated in the coprresponding pixels of the 
                 * imgHSv by the bitwise_and operation, the rest of the pixels in the result will automatically be set to black.
                 */
                Mat result_blue, result_yellow; 
                cv::bitwise_and(imgHSV, imgHSV, result_blue, blue_mask);
                cv::bitwise_and(imgHSV, imgHSV, result_yellow, yellow_mask);
            
                
                // create gray versions of the masked images
                Mat result_blue_gray, result_yellow_gray;
                cvtColor(result_blue, result_blue_gray, cv::COLOR_BGR2GRAY);
                cvtColor(result_yellow, result_yellow_gray, cv::COLOR_BGR2GRAY);
   
                /**
                 * Detect if car is coing clockwise or not.   
                 * We use a counter to only check the first 25 frames of the video. During those 25 frames, we check the left ~40%
                 * of the yellow mask image and count the number of pixels found there that are not black. If the count is > 0 it means a yellow cone
                 * has been  detected i.e. we are going counter clockwise 
                 */
                cv::Mat imgDirection;
                frameCount++;
                //std::cout << "Frame count: " << frameCount << std::endl; 

                imgDirection = result_yellow_gray(Range(300, 400), Range(0, 250));
                int numPixels = countNonZero(imgDirection);
                // std::cout << "Pixels: " << numPixels << std::endl;

                if(numPixels > 0 && frameCount < 25) {
                    clockwise = false;
                }
                // std::cout << "Clockwise: " << clockwise << std::endl;
                //namedWindow("Direction", CV_WINDOW_AUTOSIZE);
                // moveWindow("Direction", 700, 1050);
                //imshow("Direction", imgDirection);

                // Crop images 
                result_blue_gray = result_blue_gray(Range(290, 400), Range(0, 640));
                result_yellow_gray = result_yellow_gray(Range(290, 400), Range(0, 640));
                            
                /** 
                 * The Canny method will find edges in an image using the Canny algorithm. It marks the edges and saves them in the gray masks Mat. 
                 * The thresholds are used for determining which edges to detect. 
                 * @param highThresh is used to identify "strong" edges in the image. Any edge with an intensity gradient magnitude above the threshold will be kept.
                 * @param lowThresh  is used to identify weak edges, any edges with an intensity gradient magnitude below loThresh will be discarded.
                 * Edges between low and high: any edge pixel that is connected to a strong edge pixel is also considered a strong edge pixel and is retained. 
                 */
                Canny(result_blue_gray, result_blue_gray, lowThresh, hiThresh);
                Canny(result_yellow_gray, result_yellow_gray, lowThresh, hiThresh);

                /** 
                 * Threshold the images, it means that each pixel is compared to the threshold value (120) and if it is larger, then it will be set to 255 == white,
                 * if it is lower it will be set to 0 == black. This is to remove noise from the images. It removes low-intensity pixels and preserves high-intensity pixels.
                 * @param THRESH_BINARY make the pixels either white or black, depending on which side of the threshold value they are
                 * @param threshValue the threshold value t be used 
                 * @param maxValue the maximum value for a pixel in the output image
                 */
                threshold(result_blue_gray, result_blue_gray, threshValue, maxValue, cv::THRESH_BINARY);
                threshold(result_yellow_gray, result_yellow_gray, threshValue, maxValue, cv::THRESH_BINARY);

                // 2D dynamic arrays to store the points of the contours
                std::vector<std::vector<Point>> contours_blue;
                std::vector<std::vector<Point>> contours_yellow;
                

                // we create a structuring element 5x5 pixels large, with the shape of a rectangle. 
                    Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));

                /**
                 * This method combines the dilate() and erode() methods to fill in gaps and holes of the contours in the image to make them more uniform.
                 * Dilation: expands the bright regions in the image(the foregound), it takes the max pixel value within the structuring element
                 * and sets the center pixel to that value. This will make the bright regions expand.
                 * Erosion: takes the min pixel value within the structuring element and sets the center pixel to that value. This causes bright regions in the
                 * image to shrink. 
                 * By combining these two with the same structuring element the closing operation can remove small holes or gaps in the image while 
                 * preserving the overall shape of the objects. 
                 * @param MORPH_CLOSE specifies that we want to do that kind of operation (i.e close the gaps) on the contours. 
                 * @param kernel the structuring element to be used
                 */
                morphologyEx(result_blue_gray, result_blue_gray, MORPH_CLOSE, kernel);
                morphologyEx(result_yellow_gray, result_yellow_gray, MORPH_CLOSE, kernel);


                /** 
                 * Finds the contours of objects in the image. 
                 * @param RETR_EXTERNAL retrieves only the extreme outer contours. 
                 * @param CHAIN_APPROX_SIMPLE compresses horizontal, vertical, and diagonal segments and leaves only their end points. For example, an up-right rectangular contour is encoded with 4 points.
                 */
                findContours(result_blue_gray, contours_blue, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
                findContours(result_yellow_gray, contours_yellow, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
                //cout << "----------------" << endl;
                
                // selection sort on blue contours, descending order
                for(size_t i = 0; i < contours_blue.size(); i++) {
                     size_t maxIndex = i;
                     double maxArea = contourArea(contours_blue[maxIndex]);
                     for(size_t j = i + 1; j < contours_blue.size(); j++) {
                        double currentArea = contourArea(contours_blue[j]);
                        if(currentArea > maxArea) {
                            maxIndex = j;
                        }
                     } 
                     if(maxIndex != i) {
                        vector<Point> temp = contours_blue[i];
                        contours_blue[i] = contours_blue[maxIndex];
                        contours_blue[maxIndex] = temp;
                    }
                    //cout << "Contour area blue[" << i << "] = " << contourArea(contours_blue[i]) << endl;                                                                                                                                                                                                                                                                                                
                }
               // cout << "---------------" << endl;

                // selection sort on yellow contours, descending order
                 for(size_t i = 0; i < contours_yellow.size(); i++) {
                     size_t maxIndex = i;
                     double maxArea = contourArea(contours_yellow[maxIndex]);
                     for(size_t j = i + 1; j < contours_yellow.size(); j++) {
                        double currentArea = contourArea(contours_yellow[j]);
                        if(currentArea > maxArea) {
                            maxIndex = j;
                        }
                     } 
                     if(maxIndex != i) {
                        vector<Point> temp = contours_yellow[i];
                        contours_yellow[i] = contours_yellow[maxIndex];
                        contours_yellow[maxIndex] = temp;
                    }
                    //cout << "Contour area yellow[" << i << "] = " << contourArea(contours_yellow[i]) << endl;                                                                                                                                                                                                                                                                                                
                }
                //cout << "---------------" << endl;

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
                std::vector<Moments> moms_blue(contours_blue.size());
                for(size_t i = 0; i < contours_blue.size(); i++) {
                    moms_blue[i] = moments(contours_blue[i]);
                }
                // we loop through the moments and perform operations on them to get the centroid values 
                std::vector<Point2f> centroids_blue(contours_blue.size());
                for(size_t i = 0; i < contours_blue.size(); i++) {
                     // m00: represents the total area of the contour. 
                     // m10: represents the sum of the x coordinates of all the pixels in the shape. 
                     // m01: represents the sum of the y coordinatese of all pixels in the shape. 
                     // m10/m00 gives us the average x coordinate of the shape which is also the x coordinate of the centroid
                     // m01/m00 gives us the average y coordinate of the shape which is also the y coordinate of the centroid  
                     // we create a Point out of the centroid x and y coordinates and store in centroids vector
                     // m10, m00, and m01 operations return double, so we cast to float
                    centroids_blue[i] = Point2f(static_cast<float>(moms_blue[i].m10/moms_blue[i].m00), static_cast<float>(moms_blue[i].m01/moms_blue[i].m00));
                }

                // We do the same thing for the yellow contours
                std::vector<Moments> moms_yellow(contours_yellow.size());
                for(size_t i = 0; i < contours_yellow.size(); i++) {
                    moms_yellow[i] = moments(contours_yellow[i]);
                }
                // centroids yellow
                std::vector<Point2f> centroids_yellow(contours_yellow.size());
                for(size_t i = 0; i < contours_yellow.size(); i++) {
                 
                    centroids_yellow[i] = Point2f(static_cast<float>(moms_yellow[i].m10/moms_yellow[i].m00), static_cast<float>(moms_yellow[i].m01/moms_yellow[i].m00));
                }

                // declare variables to hold coordinates of blue cones
                uint16_t bCloseX;
                uint16_t bCloseY;
                uint16_t bFarX;
                uint16_t bFarY;

                // declare cone structs to hold the variables declared previously
                pos_api::cone_t bClose{};
                pos_api::cone_t bFar{};
                pos_api::cone_t yClose{};
                pos_api::cone_t yFar{};
                
                if(contours_blue.size() != 0) {
                // loop through the contours and draw them out on an image
                    for(size_t i = 0; i < contours_blue.size(); i++) {
                        
                        // draws the contours out on the imgContours_blue image.
                        drawContours(imgContours_blue, contours_blue, (int)i, Scalar(0, 255, 0), 1);
                        
                        // boundingRect finds the smallest rectangle that completely encloses a given contour or set of points. 
                        // boundingRect returns the x and y coordinates of the top left corner as one Point, the width and the height and stores it in a Rect object.
                        cv::Rect rectAroundCone = cv::boundingRect(contours_blue[i]);
                    
                        // We try to ignore the smallest contours spotted by only drawing rectangles for Rects that have width and height > 5 to reduce noise
                        if(rectAroundCone.height > 5 && rectAroundCone.width > 5) {
                            
                            // draw a rectangle with the Rect as base
                            cv::rectangle(img, rectAroundCone, cv::Scalar(0, 255, 0), 2);
                            
                            // We store the centroid coordinates of the largest contour (i.e. centroids_blue[i - 1]) and the second largest contour (i.e centroids_blue[i])
                            // in variables to send to shared memory later. And we draw a line between the cones. 
                            if(i == 1) {
                                line(img, Point(centroids_blue[i - 1].x, centroids_blue[i - 1].y), Point(centroids_blue[i].x, centroids_blue[i].y), cv::Scalar(0, 0, 255), 2);
                                // We need to know if the car is goind clockwise or not to draw the second line correctly
                                if(clockwise) {
                                    // line(img, Point(centroids_blue[i - 1].x, centroids_blue[i - 1].y), Point(centroids_blue[i - 1].x + 100, centroids_blue[i - 1].y), cv::Scalar(0, 0, 255), 2);
                                } else {
                                    // line(img, Point(centroids_blue[i - 1].x, centroids_blue[i - 1].y), Point(centroids_blue[i - 1].x - 100, centroids_blue[i - 1].y), cv::Scalar(0, 0, 255), 2);
                                }
                            //since we only care about sending data about the closeest two cones, no need to continue loop if i > 1   
                            } else if(i > 1) {
                                break;
                            }
                        }
                    }
                }

                
                 
                // if there are atleast 2 contours, add the centroids of the biggest and second biggest contour to the
                // cone structs declared previously.
                if(contours_blue.size() > 1) {
                    bCloseX = centroids_blue[0].x;
                    bCloseY = centroids_blue[0].y;
                    bFarX = centroids_blue[1].x;
                    bFarY = centroids_blue[1].y;
                    pos_api::cone_t tmpClose{bCloseX, bCloseY};
                    pos_api::cone_t tmpFar{bFarX, bFarY};
                    memcpy(&bClose, &tmpClose, sizeof(pos_api::cone_t));
                    memcpy(&bFar, &tmpFar, sizeof(pos_api::cone_t));
                // if there are < 2 contours found, send NO_CONE_POS to represent it.   
                } else {
                    memcpy(&bFar, &pos_api::NO_CONE_POS, sizeof(pos_api::cone_t));
                    memcpy(&bClose, &pos_api::NO_CONE_POS, sizeof(pos_api::cone_t));
                }
                 
                // Define variables for yellow cone position data
                 uint16_t yCloseX;
                 uint16_t yCloseY;
                 uint16_t yFarX;
                 uint16_t yFarY;

                 if(contours_yellow.size() != 0) {
                 // we do the same operations for the yellow cones as for the blue ones
                 for(size_t i = 0; i < contours_yellow.size(); i++) {
                    
                    //draw contours
                    drawContours(imgContours_yellow, contours_yellow, (int)i, Scalar(0, 255, 0), 1);
                    
                    // get Rect object
                    cv::Rect rectAroundCone = cv::boundingRect(contours_yellow[i]);

                    // if(rectAroundCone.height > 5 && rectAroundCone.width > 5) {
                    // draw rectangle
                    cv::rectangle(img, rectAroundCone, cv::Scalar(0, 255, 0), 2);
                   
                    // same procedure for yellow as for blue
                    if(i == 1) {
                        line(img, Point(centroids_yellow[i - 1].x, centroids_yellow[i - 1].y), Point(centroids_yellow[i].x, centroids_yellow[i].y), cv::Scalar(0, 0, 255), 2);
                        if(clockwise) {
                            // line(img, Point(centroids_yellow[i - 1].x, centroids_yellow[i - 1].y), Point(centroids_yellow[i - 1].x - 100, centroids_yellow[i - 1].y), cv::Scalar(0, 0, 255), 2);
                        } else {
                            // line(img, Point(centroids_yellow[i - 1].x, centroids_yellow[i - 1].y), Point(centroids_yellow[i - 1].x + 100, centroids_yellow[i - 1].y), cv::Scalar(0, 0, 255), 2);
                        }
                    } else if(i > 1) {
                        break;
                    }
                 }
                 
                }
                 
                // Same procedure for yellow cones as for blue
                if(contours_yellow.size() > 1) {
                    yCloseX = centroids_yellow[0].x;
                    yCloseY = centroids_yellow[0].y;
                    yFarX = centroids_yellow[1].x;
                    yFarY = centroids_yellow[1].y;
                    pos_api::cone_t tmpClose{yCloseX, yCloseY};
                    pos_api::cone_t tmpFar{yFarX, yFarY};
                    memcpy(&yClose, &tmpClose, sizeof(pos_api::cone_t));
                    memcpy(&yFar, &tmpFar, sizeof(pos_api::cone_t));
                } else {
                    memcpy(&yFar, &pos_api::NO_CONE_POS, sizeof(pos_api::cone_t));
                    memcpy(&yClose, &pos_api::NO_CONE_POS, sizeof(pos_api::cone_t));
                }


                // Display the windows
                namedWindow("Blue", CV_WINDOW_AUTOSIZE);
                //moveWindow("Blue", 300, 200);               // make the windows appear at a fixed place on the screen when program runs
                imshow("Blue", imgContours_blue);
                namedWindow("Yellow", CV_WINDOW_AUTOSIZE);
                //moveWindow("Yellow", 400, 300);
                imshow("Yellow", imgContours_yellow);

                // call the toMicroseconds function to get the timestamp converted to microseconds. 
                // the std::get will get the second element in the sampleTimePoint std::pair, which is the timestamp
                uint32_t seconds = sampleTimePoint.second.seconds();
                uint32_t microseconds = cluon::time::toMicroseconds(std::get<1>(sampleTimePoint));

                pos_api::timestamp_t vidTimeStamp {
                    seconds,    
                    microseconds
                };
    
                // Get the UNIX timestamp
                cluon::data::TimeStamp t = cluon::time::now();

                // Fill the struct with all the cordinates of the two closest yellow and blue cones and the current timestamp
                pos_api::data_t coneData {
                    bClose,
                    bFar,
                    yClose,
                    yFar,
                    {t.seconds(), t.microseconds()},   
                    vidTimeStamp   // getTimeStamp(from cluon) here!!!!!!!!!!!!!!!!!!!!!!!!.------------------
                };

                // put the cone data into the shared memory to be extracted by the steering calculator microservice
                pos_api::put(coneData);

    
     //-------------------------------------------------^-----------------------------------------------------

                // If you want to access the latest received ground steering, don't forget to lock the mutex:
                {
                    std::lock_guard<std::mutex> lck(gsrMutex);
                    std::cout << "main: groundSteering = " << gsr.groundSteering() << std::endl;
                }


                // Display image on your screen.
                if (VERBOSE) {
                    //moveWindow(sharedMemory->name().c_str(), 500, 400);
                    cv::imshow(sharedMemory->name().c_str(), img);
                    cv::waitKey(1);
                }
            }
/*---------------------------------- v -------------------------------------------------------*/
             if (nullptr != iplimage) {
                cvReleaseImageHeader(&iplimage);
            }
/*----------------------------------- ^ ------------------------------------------------------*/
        }
        retCode = 0;
    }
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

