#pragma once

//todo: Create new singleton class "PianoHandler"
//	* checks if camera connected
//	* starts rs2::pipeline
//	* detects piano (aligned to color)
// 	* has a private class "PianoNotes" in it, which:
//		- Preloads notes on init
// 	    - Frees notes on end
// 	    - has function play_note(int note) 
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

//set to true/false
#define DEBUG_IMAGE false
#define HIGH_QUALITY true
#define PIANO_PRINT true

#if PIANO_PRINT == true
#define PRINT_DEBUG(x) \
do{ \
	cout << x << endl; \
}while(0)
#else
#define PRINT_DEBUG(x) \
do{ \
} while(0)
#endif


//------------------------------------------------------
/*
* PlaySound Includes
*/
#include <Windows.h>
#include <fstream>
#include <conio.h>
#pragma comment(lib, "Winmm.lib")
//------------------------------------------------------
#include "OpenCV_Sandbox2.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;
//using std::cout;
//using std::endl;

#include <iostream>


#include "Colors.h"
#include "Detector.h"
#include "PianoDetector.h"
#include "HandDetector.h"

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>   // Include OpenCV API

//#define MAX_DELTA_LOWER_THRESH 0.0025f
#define MAX_DELTA_LOWER_THRESH 0.015f
//#define MAX_DELTA_UPPER_THRESH 0.006f
#define MAX_DELTA_UPPER_THRESH 0.030f
//#define DEPTH_MIN_Z 0.200f
#define DEPTH_MIN_Z 0.100f

#define NUM_WHITE_NOTES 8
#define NUM_BLACK_NOTES 5

class PianoHandler {
private:

	// Declare depth colorizer for pretty visualization of depth data
	rs2::colorizer color_map;

	// Declare RealSense pipeline, encapsulating the actual device and sensors
	rs2::pipeline pipe;

	// Optional: set your own configuration
	//rs2::config cfg;

	// Enable hole-filling
	// Hole filling is an agressive heuristic and it gets the depth wrong many times
	// However, this demo is not built to handle holes
	// (the shortest-path will always prefer to "cut" through the holes since they have zero 3D distance)
	//rs2::spatial_filter spat;

	//todo: for better depth results, instead of align_to_color, try align_to_depth \
				from the beginning (piano detection), and use raw data of depth
	rs2::align align_to_color;
	//rs2::align align_to_depth; (RS2_STREAM_DEPTH);

	// Piano Detector
	PianoDetector mPianoDetector;
	Mat m_color_piano;
	rs2::frame f_depth_piano;
	// Hand Detector
	HandDetector mHandDetector;
	Mat m_color_hand;
	Mat m_depth_hand_clr;

	float scale = 0.0f;

	int prev_note = -1;


	vector<vector<Point>> mWhiteKeysLMOP;
	vector<vector<Point>> mBlackKeysLMOP;




	HINSTANCE HInstance;
	char* white_notes_buffers[NUM_WHITE_NOTES];
	char* white_notes_paths[NUM_WHITE_NOTES] = {
		"../../../assets/001.wav", "../../../assets/003.wav",
		"../../../assets/005.wav", "../../../assets/006.wav",
		"../../../assets/008.wav", "../../../assets/00A.wav",
		"../../../assets/00C.wav", "../../../assets/00D.wav" };

	char* black_notes_buffers[NUM_BLACK_NOTES];
	char* black_notes_paths[NUM_BLACK_NOTES] = {
		"../../../assets/002.wav", "../../../assets/004.wav",
		"../../../assets/007.wav", "../../../assets/009.wav",
		"../../../assets/00B.wav" };


	void PreloadNotes() {
		for (int i = 0; i < NUM_WHITE_NOTES; i++) {
			
			cout << "Preload - " << white_notes_paths[i] << endl;
			
			white_notes_buffers[i] = 0;

			std::ifstream infile(white_notes_paths[i], std::ios::binary);

			if (!infile)
			{
				std::cout << "Wave::file error: " << white_notes_paths[i] << std::endl;
				return;
			}

			infile.seekg(0, std::ios::end);   // get length of file
			int length = infile.tellg();
			white_notes_buffers[i] = new char[length];    // allocate memory
			infile.seekg(0, std::ios::beg);   // position to start of file
			infile.read(white_notes_buffers[i], length);  // read entire file

			infile.close();
		}

		for (int i = 0; i < NUM_BLACK_NOTES; i++) {

			cout << "Preload - " << black_notes_paths[i] << endl;

			black_notes_buffers[i] = 0;

			std::ifstream infile(black_notes_paths[i], std::ios::binary);

			if (!infile)
			{
				std::cout << "Wave::file error: " << black_notes_paths[i] << std::endl;
				return;
			}

			infile.seekg(0, std::ios::end);   // get length of file
			int length = infile.tellg();
			black_notes_buffers[i] = new char[length];    // allocate memory
			infile.seekg(0, std::ios::beg);   // position to start of file
			infile.read(black_notes_buffers[i], length);  // read entire file

			infile.close();
		}
	}

	int checkKeyPressed(const rs2::depth_frame& piano, rs2::depth_frame& hand, Point& point)
	{
		if (point.x == -1 || point.y == -1) {
			return -1;
		}

		//check if point outside piano mask.
		int current_note = -1;
		for (int i = 0; (current_note == -1) && (i < mWhiteKeysLMOP.size()); i++) {
			if (pointPolygonTest(mWhiteKeysLMOP.at(i), point, false) == 0
				|| pointPolygonTest(mWhiteKeysLMOP.at(i), point, false) == 1) {
				current_note = i;
			}
		}
		for (int i = 0; (current_note == -1) && (i < mBlackKeysLMOP.size()); i++) {
			if (pointPolygonTest(mBlackKeysLMOP.at(i), point, false) == 0
				|| pointPolygonTest(mBlackKeysLMOP.at(i), point, false) == 1) {
				current_note = 1000 + i;
			}
		}

		//if point outside mask, return (with prev_note=-1)
		if (current_note == -1) {
			PRINT_DEBUG("N.F. current_note=" << current_note);
			prev_note = -1;
			return -1;
		}
		//else inside mask.

		//calculate depth.
		float handDepth = hand.get_distance(point.x, point.y);
		float pianoDepth = piano.get_distance(point.x, point.y);



#if HIGH_QUALITY == true
		//if hand has wrong depth data, get mean.
		if (handDepth <= DEPTH_MIN_Z) {
			const int w_d = hand.as<rs2::video_frame>().get_width();
			const int h_d = hand.as<rs2::video_frame>().get_height();
			Mat mask = Mat::zeros(h_d, w_d, CV_8U);
			circle(mask, point, 5, 255, -1);
			//imshow("mymask", mask);
			Mat handDepthMat(Size(w_d, h_d), CV_16UC1, (void*)hand.get_data(), Mat::AUTO_STEP);
			handDepthMat.convertTo(handDepthMat, CV_64F);
			handDepthMat = handDepthMat * scale;
			Mat nonZeroHand = (handDepthMat != 0);
			nonZeroHand = mask & nonZeroHand;
			Scalar handDepthMean = mean(handDepthMat, nonZeroHand);
			handDepth = handDepthMean[0];
		}

		if (pianoDepth <= DEPTH_MIN_Z) {
			const int w_d = hand.as<rs2::video_frame>().get_width();
			const int h_d = hand.as<rs2::video_frame>().get_height();
			Mat mask = Mat::zeros(h_d, w_d, CV_8U);
			circle(mask, point, 5, 255, -1);
			//imshow("mymask", mask);
			Mat pianoDepthMat(Size(w_d, h_d), CV_16UC1, (void*)piano.get_data(), Mat::AUTO_STEP);
			pianoDepthMat.convertTo(pianoDepthMat, CV_64F);
			pianoDepthMat = pianoDepthMat * scale;
			Mat nonZeroPiano = (pianoDepthMat != 0);
			nonZeroPiano = mask & nonZeroPiano;
			Scalar pianoDepthMean = mean(pianoDepthMat, nonZeroPiano);
			pianoDepth = pianoDepthMean[0];
		}
#endif

		PRINT_DEBUG(endl << endl);
		PRINT_DEBUG("pianoDepth=" << pianoDepth << ", handDepth=" << handDepth);

		if (handDepth <= DEPTH_MIN_Z) {
			cout << "ERROR: handDepth=" << handDepth << endl;
			PRINT_DEBUG("ERROR: prev_note=" << prev_note);
			return -1;
		}
		if (pianoDepth <= DEPTH_MIN_Z) {
			cout << "ERROR: pianoDepth=" << pianoDepth << endl;
			PRINT_DEBUG("ERROR: prev_note=" << prev_note);
			return -1;
		}

		float currPoint_depth = pianoDepth - handDepth;

		//if depth > upper threshold, return (with prev_note=-1)
		if (currPoint_depth > MAX_DELTA_UPPER_THRESH) {
			PRINT_DEBUG("H.R. currPoint_depth=" << currPoint_depth);
			PRINT_DEBUG("H.R. curr_note=-1");
			prev_note = -1;
			return -1;
		}
		//else depth < upper threshold, either same or different note

		//if different && depth < lower threshold, play and return
		if (current_note != prev_note && currPoint_depth <= MAX_DELTA_LOWER_THRESH) {
			//if white key
			if (current_note < 1000) {
				PRINT_DEBUG("------------------------------------------------------");
				PRINT_DEBUG("!!FOUND KEY!! W" << current_note + 1);
				PRINT_DEBUG("P.W. currPoint_depth=" << currPoint_depth);
				PRINT_DEBUG("P.W. current_note=" << current_note);
				/*if (current_note >= 0 && current_note < NUM_WHITE_NOTES) {
					PlaySound(notes_buffers[current_note], HInstance, SND_MEMORY | SND_ASYNC);
					//Sleep(50);
				}*/
				prev_note = current_note;
				return current_note;
			}
			//else black key
			PRINT_DEBUG("------------------------------------------------------");
			PRINT_DEBUG("!!FOUND KEY!! B" << (current_note - 1000) + 1);
			PRINT_DEBUG("P.W. currPoint_depth=" << currPoint_depth);
			PRINT_DEBUG("P.W. current_note=" << current_note);
			/*if ((current_note - 1000) >= 0 && (current_note - 1000) < NUM_WHITE_NOTES) {
				PlaySound(notes_buffers[NUM_WHITE_NOTES - 1 - (current_note - 1000)], HInstance, SND_MEMORY | SND_ASYNC);
				//Sleep(50);
			}*/
			prev_note = current_note;
			return current_note;

		}
		//else either same note, or different but depth > lower threshold

		//if same note, return.
		if (current_note == prev_note) {
			PRINT_DEBUG("S.N. currPoint_depth=" << currPoint_depth);
			PRINT_DEBUG("S.N. current_note=" << current_note << ", prev_note=" << prev_note);
			return -1;
		}
		//else different but depth > lower threshold => hand removed

		prev_note = -1;
		PRINT_DEBUG("H.R.#2 currPoint_depth=" << currPoint_depth);
		PRINT_DEBUG("H.R.#2 current_note=-1");
		return -1;
	}

	PianoHandler() : align_to_color(RS2_STREAM_COLOR), HInstance(GetModuleHandle(0)),
		white_notes_buffers(), black_notes_buffers() {
		// Start with default configuration
		pipe.start();

		// Or with your own configuration
		//cfg.enable_stream(RS2_STREAM_COLOR,640,480, RS2_FORMAT_RGB8, 30);
		//cfg.enable_stream(RS2_STREAM_DEPTH,848,480, RS2_FORMAT_Z16, 30);
		//pipe.start(cfg);

		// Enable hole-filling
		//spat.set_option(RS2_OPTION_HOLES_FILL, 5); // 5 = fill all the zero pixels

		PreloadNotes();
	}

public:
	//PianoHandler(PianoHandler& other) = delete;
	//void operator=(const PianoHandler&) = delete;

	static PianoHandler GetInstance() {
		static PianoHandler singleton_;
		return singleton_;
	}

	//~PianoHandler() {
	//	//todo: add 8th note and delete the -1
	//	for (int i = 0; i < NUM_WHITE_NOTES - 1; i++) {
	//		delete[] white_notes_buffers[i];
	//	}
	//}


	int ConfigurePiano() {

		//cv::Scalar wLowerThresh(0, 0, 100);
		//cv::Scalar wUpperThresh(179, 255, 255);
		cv::Scalar lowerThresh;
		cv::Scalar upperThresh;

		const auto piano_window = "Piano Configuration";
		namedWindow(piano_window);

		// ------------------------------------------------------------------
		// Calibrate White
		char* piano_mask_control_window = "Mask Control";
		namedWindow(piano_mask_control_window, WINDOW_AUTOSIZE);
		int desiredWidth = 640, desiredheight = 480;
		resizeWindow(piano_mask_control_window, desiredWidth, desiredheight);
		//resizeWindow(piano_mask_control_window, Size(srcRGB.cols, srcRGB.rows));

		const auto piano_mask_window = "Piano Mask";
		namedWindow(piano_mask_window);

		createTrackbar("HMin", piano_mask_control_window, 0, 179); // Hue is from 0 - 179 for Opencv
		createTrackbar("SMin", piano_mask_control_window, 0, 255);
		createTrackbar("VMin", piano_mask_control_window, 0, 255);
		createTrackbar("HMax", piano_mask_control_window, 0, 179);
		createTrackbar("SMax", piano_mask_control_window, 0, 255);
		createTrackbar("VMax", piano_mask_control_window, 0, 255);

		// Expected range: (H,S,V)
		// sensitivity = 15 (change this)
		// lower_white = (0, 0, 255 - sensitivity)
		// upper_white = (179, sensitivity, 255)
		const int sensitivity = 70;
		// Set default value for MIN HSV trackbars.
		setTrackbarPos("HMin", piano_mask_control_window, 0);
		setTrackbarPos("SMin", piano_mask_control_window, 0);
		setTrackbarPos("VMin", piano_mask_control_window, 255 - sensitivity);
		// Set default value for MAX HSV trackbars.
		setTrackbarPos("HMax", piano_mask_control_window, 179);
		setTrackbarPos("SMax", piano_mask_control_window, sensitivity);
		setTrackbarPos("VMax", piano_mask_control_window, 255);

		// Initialize to check if HSV min / max value changes
		int hMin = 0, sMin = 0, vMin = 0, hMax = 0, sMax = 0, vMax = 0;
		int phMin = 0, psMin = 0, pvMin = 0, phMax = 0, psMax = 0, pvMax = 0;
		// ------------------------------------------------------------------

		cout << "Configure piano: press any key to set.." << endl;
		//while (waitKey(1) < 0 && getWindowProperty(window_name1, WND_PROP_AUTOSIZE) >= 0 && getWindowProperty(window_name2, WND_PROP_AUTOSIZE) >= 0)
		//while (!kbhit() && waitKey(100) && getWindowProperty(piano_window, WND_PROP_AUTOSIZE) >= 0)
		while ((waitKey(100) == -1) && getWindowProperty(piano_window, WND_PROP_AUTOSIZE) >= 0
			&& getWindowProperty(piano_mask_control_window, WND_PROP_AUTOSIZE) >= 0
			&& getWindowProperty(piano_mask_window, WND_PROP_AUTOSIZE) >= 0)
		{
			// Wait for next set of frames from the camera
			rs2::frameset data = pipe.wait_for_frames();

			// Apply filter
			//data.apply_filter(spat);

			// Align to a sensor (color/depth)
			data = align_to_color.process(data);

			// Get data from frame
			f_depth_piano = data.get_depth_frame();
			rs2::frame f_color_piano = data.get_color_frame();

			// Query frame size (width and height)
			const int w_c = f_color_piano.as<rs2::video_frame>().get_width();
			const int h_c = f_color_piano.as<rs2::video_frame>().get_height();


			// Create OpenCV matrix of size (w,h) from the RGB data
			m_color_piano = Mat(Size(w_c, h_c), CV_8UC3, (void*)f_color_piano.get_data(), Mat::AUTO_STEP);
			cvtColor(m_color_piano, m_color_piano, COLOR_BGR2RGB);

			Mat output = m_color_piano;
			{
				// get current positions of all trackbars
				hMin = getTrackbarPos("HMin", piano_mask_control_window);
				sMin = getTrackbarPos("SMin", piano_mask_control_window);
				vMin = getTrackbarPos("VMin", piano_mask_control_window);

				hMax = getTrackbarPos("HMax", piano_mask_control_window);
				sMax = getTrackbarPos("SMax", piano_mask_control_window);
				vMax = getTrackbarPos("VMax", piano_mask_control_window);

				// Set minimum and max HSV values to display
				lowerThresh = Scalar(hMin, sMin, vMin);
				upperThresh = Scalar(hMax, sMax, vMax);

				// Create HSV Imageand threshold into a range.
				Mat hsvMat;
				cvtColor(m_color_piano, hsvMat, COLOR_RGB2HSV);
				Mat myMask;
				inRange(hsvMat, lowerThresh, upperThresh, myMask);
				bitwise_and(m_color_piano, m_color_piano, output, myMask);

				// Print if there is a change in HSV value
				if ((phMin != hMin) || (psMin != sMin) || (pvMin != vMin) ||
					(phMax != hMax) || (psMax != sMax) || (pvMax != vMax)) {
					PRINT_DEBUG("(hMin = " << hMin << ", sMin = " << sMin << ", vMin = " << vMin << ")");
					PRINT_DEBUG("(hMax = " << hMax << ", sMax = " << sMax << ", vMax = " << vMax << ")");
					PRINT_DEBUG("");
					phMin = hMin;
					psMin = sMin;
					pvMin = vMin;
					phMax = hMax;
					psMax = sMax;
					pvMax = vMax;
				}

				// Display output image
				//imshow("myMask", myMask);
				//imshow(piano_mask_control_window, output);
				imshow(piano_mask_window, myMask);
			}

			mPianoDetector.apply(m_color_piano, m_color_piano, lowerThresh, upperThresh);
			mWhiteKeysLMOP = mPianoDetector.getWhiteKeysLMOP();
			mBlackKeysLMOP = mPianoDetector.getBlackKeysLMOP();
			if (!mWhiteKeysLMOP.empty()) {
				mPianoDetector.drawAllContours(m_color_piano, mWhiteKeysLMOP, Colors::mLineColorGreen, 1);
			}
			if (!mBlackKeysLMOP.empty()) {
				mPianoDetector.drawAllContours(m_color_piano, mBlackKeysLMOP, Colors::mLineColorYellow, 1);
			}
			imshow(piano_window, m_color_piano);
		}

		cv::destroyWindow(piano_window);
		cv::destroyWindow(piano_mask_window);
		cv::destroyWindow(piano_mask_control_window);

		if (mWhiteKeysLMOP.empty()) {
			cout << "Error: No white keys detected!" << endl;
			return -1;
		}
		if (mBlackKeysLMOP.empty()) {
			cout << "Error: No black keys detected!" << endl;
			return -1;
		}
		cout << "Piano detected!" << endl;
		cout << "\tNumber of white keys = " << mWhiteKeysLMOP.size() << endl;
		cout << "\tNumber of black keys = " << mBlackKeysLMOP.size() << endl;
		return EXIT_SUCCESS;
	}


	int GetNextKey() {

		int newkey = -1;


#if DEBUG_IMAGE == true
		const auto hand_window = "Play Next Note";
		namedWindow(hand_window, WINDOW_AUTOSIZE);
#endif


		//while (!kbhit() && waitKey(5) && getWindowProperty(hand_window, WND_PROP_AUTOSIZE) >= 0)
#if DEBUG_IMAGE == true
		while ((waitKey(5)) && newkey == -1
			&& getWindowProperty(hand_window, WND_PROP_AUTOSIZE) >= 0)
#else
		while ((waitKey(5)) && newkey == -1)
#endif
		{
			// Wait for next set of frames from the camera
			rs2::frameset data = pipe.wait_for_frames();

			// Apply filter
			//data.apply_filter(spat);

			// Align to a sensor (color/depth)
			data = align_to_color.process(data);

			// Get data from frame
			rs2::frame f_color_hand = data.get_color_frame();
			rs2::depth_frame f_depth_hand = data.get_depth_frame();
			scale = f_depth_hand.get_units();


			// Query frame size (width and height)
			const int w_c = f_color_hand.as<rs2::video_frame>().get_width();
			const int h_c = f_color_hand.as<rs2::video_frame>().get_height();

			// Create OpenCV matrix of size (w,h) from the RGB data
			m_color_hand = Mat(Size(w_c, h_c), CV_8UC3, (void*)f_color_hand.get_data(), Mat::AUTO_STEP);
			cvtColor(m_color_hand, m_color_hand, COLOR_BGR2RGB);

			// Find and draw red object (fingertip) on image
			mHandDetector.apply_red(m_color_hand, m_color_hand);

#if DEBUG_IMAGE == true
			// Show image
			imshow(hand_window, m_color_hand);
#endif

			// Check if key pressed and play sound
			newkey = checkKeyPressed(f_depth_piano, f_depth_hand, mHandDetector.getLowestPoint());
		}
#if DEBUG_IMAGE == true
		// Show image
		cv::destroyWindow(hand_window);
#endif
		return newkey;
	}

	void PlayNote(int current_note) {
		if (current_note >= 0 && current_note < NUM_WHITE_NOTES) {
			PlaySound(white_notes_buffers[current_note], HInstance, SND_MEMORY | SND_ASYNC);
			//Sleep(50);
			return;
		}
		int black_note = current_note - 1000;
		if (black_note >= 0 && black_note < NUM_BLACK_NOTES) {
			PlaySound(black_notes_buffers[black_note], HInstance, SND_MEMORY | SND_ASYNC);
			//Sleep(50);
		}

	}

};