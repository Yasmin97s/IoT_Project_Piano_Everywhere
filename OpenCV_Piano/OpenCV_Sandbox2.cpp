// OpenCV_Sandbox2.cpp : Defines the entry point for the application.
//


//set to true/false
#define DEBUG_IMAGE true
#define HIGH_QUALITY true
#define PIANO_PRINT true


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
#define MAX_DELTA_LOWER_THRESH 0.010f
//#define MAX_DELTA_UPPER_THRESH 0.006f
#define MAX_DELTA_UPPER_THRESH 0.020f
//#define DEPTH_MIN_Z 0.200f
#define DEPTH_MIN_Z 0.100f

#define NUM_WHITE_NOTES 8
#define NUM_BLACK_NOTES 5

//cv::RNG rng(12345);

float scale = 0.0f;

int prev_note = -1;


vector<vector<Point>> mWhiteKeysLMOP;
vector<vector<Point>> mBlackKeysLMOP;

char* notes_buffers[NUM_WHITE_NOTES];
HINSTANCE HInstance = GetModuleHandle(0);
char* notes_paths[NUM_WHITE_NOTES] = {
	"../../../assets/note1.wav", "../../../assets/note2.wav",
	"../../../assets/note3.wav", "../../../assets/note4.wav",
	"../../../assets/note5.wav", "../../../assets/note6.wav",
	"../../../assets/note7.wav", "../../../assets/note8.wav" };

void PreloadNotes() {
	for (int i = 0; i <= 6; i++) {
		notes_buffers[i] = 0;

		std::ifstream infile(notes_paths[i], std::ios::binary);

		if (!infile)
		{
			std::cout << "Wave::file error: " << notes_paths[i] << std::endl;
			return;
		}

		infile.seekg(0, std::ios::end);   // get length of file
		int length = infile.tellg();
		notes_buffers[i] = new char[length];    // allocate memory
		infile.seekg(0, std::ios::beg);   // position to start of file
		infile.read(notes_buffers[i], length);  // read entire file

		infile.close();
	}
}

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

void checkKeyPressed1(const rs2::depth_frame& piano, rs2::frameset& data, rs2::align& align_to_color, Point& point)
{
	/*if (point.x == -1 || point.y == -1) {
		return;
	}

	int previous_note = prevPoint_note;
	prevPoint_note = -1;

	if (prevPoint.x == -1 && prevPoint.y == -1) {
		prevPoint = Point(0, 0);
		prevPoint_depth = 0.0f;
		prevPoint_close = HAND_IS_CLOSE::NO;
	}

	currPoint = point;

	int xDiffPoints = currPoint.x- prevPoint.x;
	int yDiffPoints = currPoint.y - prevPoint.y;

	prevPoint = currPoint;

	data = align_to_color.process(data); // Align frame to color image
	rs2::depth_frame hand = data.get_depth_frame();
	float handDepth = hand.get_distance(currPoint.x, currPoint.y);
	float pianoDepth = piano.get_distance(currPoint.x, currPoint.y);

	//if (yDiffPoints < 5) {

	currPoint_depth = handDepth;

	bool isClose = false;
	if (prevPoint_close == HAND_IS_CLOSE::YES){ //next point still considered close unless very far
		//todo: maybe - if any(x,y,depth) changed a lot, isClose = false
		//if hand not far enough (<=DIFF_P_H_UPPER) && Y didn't change a lot
		bool depth_changed = (prevPoint_depth - currPoint_depth) >= DIFF_P_H_UPPER;
		bool x_changed;
		bool y_changed;
		isClose = (!depth_changed);
		isClose = ((pianoDepth - handDepth) <= DIFF_P_H_UPPER) && (yDiffPoints <= DIFF_Y_UPPER);
	}
	else { //next point still considered far unless very close
		//if last point was not close, then: (only a big change makes it close)
		//
		isClose = ((pianoDepth - handDepth) <= DIFF_P_H_LOWER) && (yDiffPoints >= DIFF_Y_LOWER);
	}
	currPoint_close = isClose ? HAND_IS_CLOSE::YES : HAND_IS_CLOSE::NO;


	float depthDiffPoints = currPoint_depth - prevPoint_depth;
	HAND_IS_CLOSE prevClose = prevPoint_close;

	prevPoint_depth = currPoint_depth;
	prevPoint_close = currPoint_close;

	//if (handDepth <= 0.100f) {
	//	return;
	//}

	if (yDiffPoints < DIFF_Y_LOWER) {
		//cout << "y diff points: FAIL" << endl;
		//cout << endl << endl;
		return;
	}
	//if (yDiffPoints >= 0)
		//cout << "y diff points: OK" << endl;

	if (currPoint_close != HAND_IS_CLOSE::YES) {
		//cout << "HAND_IS_CLOSE: FAIL" << endl;
		//cout << endl << endl;
		return;
	}
	//if (yDiffPoints >= 0)
		//cout << "HAND_IS_CLOSE: OK" << endl;


	//  since:
	//      depthDiffPoints = currPoint_depth - prevPoint_depth
	//  depth of current has to be bigger than depth of previous
	if (prevClose == HAND_IS_CLOSE::YES && depthDiffPoints < DIFF_H_H &&
		yDiffPoints < DIFF_Y_LOWER && xDiffPoints < DIFF_X) {
			//yDiffPoints < DIFF_Y && xDiffPoints < DIFF_X) {
		//cout << "hand-hand depth diff: FAIL - " << endl;
		//cout << "\tprevClose=" << ((prevClose == HAND_IS_CLOSE::YES) ? "YES" : "NO") << endl;
		//cout << "\tdepthDiffPoints=" << depthDiffPoints << endl;
		//cout << endl << endl;
		return;
	}
	//if (yDiffPoints >= 0)
		//cout << "hand-hand depth diff: OK" << endl;

	//waitKey(0);

	bool played = false;
	for (int i = 0; i < mWhiteKeysLMOP.size(); i++) {
		//MatOfPoint2f p = new MatOfPoint2f();
		//p.fromArray(mWhiteKeysLMOP.get(i).toArray());

		if (pointPolygonTest(mWhiteKeysLMOP.at(i), point, false) == 0
			|| pointPolygonTest(mWhiteKeysLMOP.at(i), point, false) == 1) {
			prevPoint_note = i;
			if (previous_note == prevPoint_note) {
				continue;
			}
			cout << "!!FOUND KEY!! W" << i + 1 << endl;
			if (i >= 0 && i < NUM_WHITE_NOTES) {
				played = PlaySound(notes_buffers[i], HInstance, SND_MEMORY | SND_ASYNC);
				//PlaySound(TEXT(notes_paths[i]), NULL, SND_FILENAME | SND_ASYNC);
				played = true;
			}
			else
				cout << "!!FOUND KEY!! W" << i + 1 << endl;
			Sleep(50);
		}
	}


	for (int i = 0; i < mBlackKeysLMOP.size(); i++) {
		if (pointPolygonTest(mBlackKeysLMOP.at(i), point, false) == 0
			|| pointPolygonTest(mBlackKeysLMOP.at(i), point, false) == 1) {
			prevPoint_note = 1000 + i;
			if (previous_note == prevPoint_note) {
				continue;
			}
			cout << "!!FOUND KEY!! B" << i + 1 << endl;
			played = true;
			//if (i >= 0 && i <= 6)
				//bool played = PlaySound(TEXT("mywavsound.wav"), NULL, SND_FILENAME | SND_ASYNC);
			//Sleep(500);
			Sleep(50);
		}
	}
	if (!played) {
		cout << "NO KEY FOUND!" << endl;
	} else {
		//cout << "piano depth=" << pianoDepth << endl;
		//cout << "piano-hand depth diff=" << pianoDepth - currPoint_depth << endl;
		//cout << "HAND_IS_CLOSE=" << ((currPoint_close == HAND_IS_CLOSE::YES) ? "YES" : "NO") << endl;
		//cout << "hand-hand depth diff=" << currPoint_depth - prevPoint_depth << endl;
		cout << "yDiffPoints=" << yDiffPoints << endl;
		cout << endl;
		cout << "hand depth=" << currPoint_depth << endl;
		cout << endl;
		cout << "prevClose=" << ((prevClose == HAND_IS_CLOSE::YES) ? "YES" : "NO") << endl;
		cout << "isClose=" << ((isClose) ? "TRUE" : "FALSE") << endl;
		cout << "HAND_IS_CLOSE=" << ((currPoint_close == HAND_IS_CLOSE::YES) ? "YES" : "NO") << endl;
		cout << endl;
		cout << "HAND_IS_CLOSE=" << ((currPoint_close == HAND_IS_CLOSE::YES) ? "YES" : "NO") << endl;
		cout << "depthDiffPoints=" << depthDiffPoints << endl;
		cout << "yDiffPoints=" << yDiffPoints << endl;
		cout << "xDiffPoints=" << xDiffPoints << endl;
		cout << endl;
		cout << "prevPoint_note(current)=" << prevPoint_note << endl;
		cout << "previous_note=" << previous_note << endl;
	}
	cout << endl << endl << endl;*/
}


void checkKeyPressed2(const rs2::depth_frame& piano, rs2::depth_frame& hand, Point& point)
{
	if (point.x == -1 || point.y == -1) {
		return;
	}

	//todo: check if point outside piano mask, then return.

#if HIGH_QUALITY == true
	const int w_d = hand.as<rs2::video_frame>().get_width();
	const int h_d = hand.as<rs2::video_frame>().get_height();
	Mat mask = Mat::zeros(h_d, w_d, CV_8U);
	circle(mask, point, 5, 255, -1);
	//imshow("mymask", mask);

	Mat handDepthMat(Size(w_d, h_d), CV_16UC1, (void*)hand.get_data(), Mat::AUTO_STEP);
	handDepthMat.convertTo(handDepthMat, CV_64F);
	handDepthMat = handDepthMat * scale;
	// Calculate mean depth inside the detection region
	// This is a very naive way to estimate objects depth
	// but it is intended to demonstrate how one might 
	// use depth data in general
	Mat nonZeroHand = (handDepthMat != 0);
	nonZeroHand = mask & nonZeroHand;
	Scalar handDepthMean = mean(handDepthMat, nonZeroHand);
	//cout << std::setprecision(3) << handDepthMean[0] << " meters away";

	Mat pianoDepthMat(Size(w_d, h_d), CV_16UC1, (void*)piano.get_data(), Mat::AUTO_STEP);
	pianoDepthMat.convertTo(pianoDepthMat, CV_64F);
	pianoDepthMat = pianoDepthMat * scale;
	Mat nonZeroPiano = (pianoDepthMat != 0);
	nonZeroPiano = mask & nonZeroPiano;
	Scalar pianoDepthMean = mean(pianoDepthMat, nonZeroPiano);

	//float handDepth = hand.get_distance(point.x, point.y);
	float handDepth = handDepthMean[0];
	float pianoDepth = pianoDepthMean[0];
#else
	float handDepth = hand.get_distance(point.x, point.y);
	float pianoDepth = piano.get_distance(point.x, point.y);
#endif

	PRINT_DEBUG(endl << endl);
	//cout << "pianoDepth=" << pianoDepth << ", handDepth=" << handDepth << endl;

	if (handDepth <= DEPTH_MIN_Z) {
		//todo: remove printing and exit
		cout << "ERROR: handDepth=" << handDepth << endl;
		PRINT_DEBUG("ERROR: prev_note=" << prev_note);
		//exit(-10);
		return;
	}
	if (pianoDepth <= DEPTH_MIN_Z) {
		//todo: remove printing and exit
		cout << "ERROR: pianoDepth=" << pianoDepth << endl;
		PRINT_DEBUG("ERROR: prev_note=" << prev_note);
		//exit(-10);
		return;
	}

	float currPoint_depth = pianoDepth - handDepth;

	//if depth > upper threshold, return (with prev_note=-1)
	if (currPoint_depth > MAX_DELTA_UPPER_THRESH) {
		PRINT_DEBUG("H.R. currPoint_depth=" << currPoint_depth);
		PRINT_DEBUG("H.R. curr_note=-1");
		prev_note = -1;
		return;
	}
	//else continue..

	//if different note, play note and return
	int current_note = -1;
	for (int i = 0; i < mWhiteKeysLMOP.size(); i++) {
		if (pointPolygonTest(mWhiteKeysLMOP.at(i), point, false) == 0
			|| pointPolygonTest(mWhiteKeysLMOP.at(i), point, false) == 1) {
			current_note = i;
			//if different note && depth is good, play new note and return
			if (prev_note != current_note && currPoint_depth <= MAX_DELTA_LOWER_THRESH) {
				PRINT_DEBUG("------------------------------------------------------");
				PRINT_DEBUG("!!FOUND KEY!! W" << i + 1);
				PRINT_DEBUG("P.W. currPoint_depth=" << currPoint_depth);
				PRINT_DEBUG("P.W. current_note=" << current_note);
				if (i >= 0 && i < NUM_WHITE_NOTES) {
					PlaySound(notes_buffers[i], HInstance, SND_MEMORY | SND_ASYNC);
					//PlaySound(TEXT(notes_paths[i]), NULL, SND_FILENAME | SND_ASYNC);
					//Sleep(50);
				}
				prev_note = current_note;
				return;
			}
		}
	}

	for (int i = 0; i < mBlackKeysLMOP.size(); i++) {
		if (pointPolygonTest(mBlackKeysLMOP.at(i), point, false) == 0
			|| pointPolygonTest(mBlackKeysLMOP.at(i), point, false) == 1) {
			current_note = 1000 + i;
			//if different note && depth is good, play new note and return
			if (prev_note != current_note && currPoint_depth <= MAX_DELTA_LOWER_THRESH) {
				PRINT_DEBUG("------------------------------------------------------");
				PRINT_DEBUG("!!FOUND KEY!! B" << i + 1);
				PRINT_DEBUG("P.B. currPoint_depth=" << currPoint_depth);
				PRINT_DEBUG("P.B. current_note=" << current_note);
				if (i >= 0 && i < NUM_WHITE_NOTES) {
					PlaySound(notes_buffers[NUM_WHITE_NOTES - 1 - i], HInstance, SND_MEMORY | SND_ASYNC);
					//PlaySound(TEXT(notes_paths[i]), NULL, SND_FILENAME | SND_ASYNC);
					//Sleep(50);
				}
				prev_note = current_note;
				return;
			}
		}
	}
	//if reached here, either found same note, or didn't find note, or different note but depth not good.

	//if didn't find note, return (with prev_note=-1)
	if (current_note == -1) {
		PRINT_DEBUG("N.F. pianoDepth=" << pianoDepth << ", handDepth=" << handDepth);
		PRINT_DEBUG("N.F. currPoint_depth=" << currPoint_depth);
		PRINT_DEBUG("N.F. current_note=" << current_note);
		prev_note = -1;
		return;
	}
	//else found note.

	//if different but not depth, then hand removed.
	if (current_note != prev_note && currPoint_depth > MAX_DELTA_LOWER_THRESH) {
		prev_note = -1;
		PRINT_DEBUG("H.R.#2 currPoint_depth=" << currPoint_depth);
		PRINT_DEBUG("H.R.#2 current_note=-1");
		return;
	}
	//else found same note.

	//if same note, prev_note = current_note (doesn't change)
	//assert(prev_note == current_note);
	prev_note = current_note;
	PRINT_DEBUG("S.N. currPoint_depth=" << currPoint_depth);
	PRINT_DEBUG("S.N. current_note=" << current_note << ", prev_note=" << prev_note);
}

// This function assumes point is INSIDE fingertip.
// Should be faster than checkKeyPressed2(?).
void checkKeyPressed3(const rs2::depth_frame& piano, rs2::depth_frame& hand, Point& point)
{
	if (point.x == -1 || point.y == -1) {
		return;
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
		return;
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
		return;
	}
	if (pianoDepth <= DEPTH_MIN_Z) {
		cout << "ERROR: pianoDepth=" << pianoDepth << endl;
		PRINT_DEBUG("ERROR: prev_note=" << prev_note);
		return;
	}

	float currPoint_depth = pianoDepth - handDepth;

	//if depth > upper threshold, return (with prev_note=-1)
	if (currPoint_depth > MAX_DELTA_UPPER_THRESH) {
		PRINT_DEBUG("H.R. currPoint_depth=" << currPoint_depth);
		PRINT_DEBUG("H.R. curr_note=-1");
		prev_note = -1;
		return;
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
			if (current_note >= 0 && current_note < NUM_WHITE_NOTES) {
				PlaySound(notes_buffers[current_note], HInstance, SND_MEMORY | SND_ASYNC);
				//Sleep(50);
			}
			prev_note = current_note;
			return;
		}
		//else black key
		PRINT_DEBUG("------------------------------------------------------");
		PRINT_DEBUG("!!FOUND KEY!! B" << (current_note - 1000) + 1);
		PRINT_DEBUG("P.W. currPoint_depth=" << currPoint_depth);
		PRINT_DEBUG("P.W. current_note=" << current_note);
		if ((current_note - 1000) >= 0 && (current_note - 1000) < NUM_WHITE_NOTES) {
			PlaySound(notes_buffers[NUM_WHITE_NOTES - 1 - (current_note - 1000)], HInstance, SND_MEMORY | SND_ASYNC);
			//Sleep(50);
		}
		prev_note = current_note;
		return;

	}
	//else either same note, or different but depth > lower threshold

	//if same note, return.
	if (current_note == prev_note) {
		PRINT_DEBUG("S.N. currPoint_depth=" << currPoint_depth);
		PRINT_DEBUG("S.N. current_note=" << current_note << ", prev_note=" << prev_note);
		return;
	}
	//else different but depth > lower threshold => hand removed

	prev_note = -1;
	PRINT_DEBUG("H.R.#2 currPoint_depth=" << currPoint_depth);
	PRINT_DEBUG("H.R.#2 current_note=-1");
	return;
}

void checkKeyPressed(const rs2::depth_frame& piano, rs2::depth_frame& hand, Point& point)
{

	//todo: New class "NoteDetector"
	//	* has variables:
	//		- currPoint, PrevPoint
	//		- mWhiteKeysLMOP, mBlackKeysLMOP
	//	* has private function "checkKeyPressed"
	//	* has public function "detect_note" that returns integer:
	//		- White keys: number of key [0 - number of white keys-1]
	//		- Black keys: number of key [0 - number of black keys-1]
	//		- returns -1 if not found
	//	* handles upper/lower thresholds for detection
	//-----------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------

	//return checkKeyPressed1(piano, data, align_to_color, point);
	//return checkKeyPressed2(piano, hand, point);
	return checkKeyPressed3(piano, hand, point);


}

//credit: https://stackoverflow.com/questions/22588146/tracking-white-color-using-python-opencv
void calibrateWhite(rs2::pipeline& pipe, rs2::align align_to_color) {

	// Create a window
	auto window_image = "image";
	namedWindow(window_image, WINDOW_AUTOSIZE);

	// create trackbars for color change
	createTrackbar("HMin", window_image, 0, 179); // Hue is from 0 - 179 for Opencv
	createTrackbar("SMin", window_image, 0, 255);
	createTrackbar("VMin", window_image, 0, 255);
	createTrackbar("HMax", window_image, 0, 179);
	createTrackbar("SMax", window_image, 0, 255);
	createTrackbar("VMax", window_image, 0, 255);

	// Expected range: (H,S,V)
	// sensitivity = 15 (change this)
	// lower_white = (0, 0, 255 - sensitivity)
	// upper_white = (179, sensitivity, 255)
	const int sensitivity = 15;
	// Set default value for MIN HSV trackbars.
	setTrackbarPos("HMin", window_image, 0);
	setTrackbarPos("SMin", window_image, 0);
	setTrackbarPos("VMin", window_image, 255 - sensitivity);
	// Set default value for MAX HSV trackbars.
	setTrackbarPos("HMax", window_image, 179);
	setTrackbarPos("SMax", window_image, sensitivity);
	setTrackbarPos("VMax", window_image, 255);

	// Initialize to check if HSV min / max value changes
	int hMin = 0, sMin = 0, vMin = 0, hMax = 0, sMax = 0, vMax = 0;
	int phMin = 0, psMin = 0, pvMin = 0, phMax = 0, psMax = 0, pvMax = 0;


	int wait_time = 33;

	//char out_char = 0;

	while (1) {
		rs2::frameset frames = pipe.wait_for_frames();

		frames = align_to_color.process(frames);
		rs2::frame f_color = frames.get_color_frame();
		// Query frame size (width and height)
		const int w_c = f_color.as<rs2::video_frame>().get_width();
		const int h_c = f_color.as<rs2::video_frame>().get_height();


		// Create OpenCV matrix of size (w,h) from the RGB data
		Mat m_color = Mat(Size(w_c, h_c), CV_8UC3, (void*)f_color.get_data(), Mat::AUTO_STEP);
		cvtColor(m_color, m_color, COLOR_BGR2RGB);

		Mat output = m_color;

		// get current positions of all trackbars
		hMin = getTrackbarPos("HMin", window_image);
		sMin = getTrackbarPos("SMin", window_image);
		vMin = getTrackbarPos("VMin", window_image);

		hMax = getTrackbarPos("HMax", window_image);
		sMax = getTrackbarPos("SMax", window_image);
		vMax = getTrackbarPos("VMax", window_image);

		// Set minimum and max HSV values to display
		cv::Scalar lowerThresh(hMin, sMin, vMin);
		cv::Scalar upperThresh(hMax, sMax, vMax);

		// Create HSV Imageand threshold into a range.
		Mat hsvMat;
		cvtColor(m_color, hsvMat, COLOR_RGB2HSV);
		Mat myMask;
		inRange(hsvMat, lowerThresh, upperThresh, myMask);
		bitwise_and(m_color, m_color, output, myMask);

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
		imshow("myMask", myMask);
		imshow(window_image, output);

		// Wait longer to prevent freeze for videos.
		/*out_char = (waitKey(wait_time) & 0xFF);
		if (out_char == 'q' || out_char == 's') {
			break;
		}*/
		if (waitKey(wait_time) != -1) {
			//todo:	set threshold to selected values and make \
					PianoDetector::apply (or consturctor) take thresholds as params
			break;
		}
	}
	destroyAllWindows();
	//return (out_char == 's');
}


int main() try
{

	
	#include "PianoHandler.h"

	PianoHandler mPianoHandler = PianoHandler::GetInstance();
	mPianoHandler.ConfigurePiano();

	for (int i = 0; /*i < 3*/; i++) {
		int curr_key = mPianoHandler.GetNextKey();
		cout << endl << endl << endl;
		cout << "playing note: " << curr_key << endl;
		mPianoHandler.PlayNote(curr_key);
	}
 
	return EXIT_SUCCESS;
}
catch (const rs2::error& e)
{
	std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::exception& e)
{
	std::cerr << e.what() << std::endl;
	return EXIT_FAILURE;
}


int main_original() try
{

	//todo: Create new singleton class "PianoHandler"
	//	* checks if camera connected
	//	* starts rs2::pipeline
	//	* detects piano (aligned to color)
	//	* has function play_note(int note)
	// 	* has a private class "PianoNotes" in it, which:
	//		- Preloads notes on init
	// 	    - Frees notes on end
	// 	    - has function play_note(int note) 
	//-----------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------


	// Declare depth colorizer for pretty visualization of depth data
	//rs2::colorizer color_map;

	// Declare RealSense pipeline, encapsulating the actual device and sensors
	rs2::pipeline pipe;
	// Start streaming with default recommended configuration
	pipe.start();
	//profile = pipe.start();
	// Or set your own configuration
	//rs2::config cfg;
	//cfg.enable_stream(RS2_STREAM_COLOR,640,480, RS2_FORMAT_RGB8, 30);
	//cfg.enable_stream(RS2_STREAM_DEPTH,848,480, RS2_FORMAT_Z16, 30);
	//pipe.start(cfg);


	// Enable hole-filling
	// Hole filling is an agressive heuristic and it gets the depth wrong many times
	// However, this demo is not built to handle holes
	// (the shortest-path will always prefer to "cut" through the holes since they have zero 3D distance)
	//rs2::spatial_filter spat;
	//spat.set_option(RS2_OPTION_HOLES_FILL, 5); // 5 = fill all the zero pixels


	//todo: for better depth results, instead of align_to_color, try align_to_depth \
				from the beginning (piano detection), and use raw data of depth
	rs2::align align_to_color(RS2_STREAM_COLOR);
	//rs2::align align_to_depth(RS2_STREAM_DEPTH);

	// Piano Detector
	PianoDetector mPianoDetector;
	Mat m_color_piano;
	rs2::frame f_depth_piano;
	// Hand Detector
	HandDetector mHandDetector;
	Mat m_color_hand;
	Mat m_depth_hand_clr;


	const auto piano_window = "Piano Configuration";
	namedWindow(piano_window);
	
	// ------------------------------------------------------------------
	// Calibrate White
	char* piano_mask_window = "Piano Mask";
	namedWindow(piano_mask_window, WINDOW_AUTOSIZE);
	int desiredWidth = 640, desiredheight = 480;
	resizeWindow(piano_mask_window, desiredWidth, desiredheight);
	//resizeWindow(piano_mask_window, Size(srcRGB.cols, srcRGB.rows));
	
	createTrackbar("HMin", piano_mask_window, 0, 179); // Hue is from 0 - 179 for Opencv
	createTrackbar("SMin", piano_mask_window, 0, 255);
	createTrackbar("VMin", piano_mask_window, 0, 255);
	createTrackbar("HMax", piano_mask_window, 0, 179);
	createTrackbar("SMax", piano_mask_window, 0, 255);
	createTrackbar("VMax", piano_mask_window, 0, 255);

	// Expected range: (H,S,V)
	// sensitivity = 15 (change this)
	// lower_white = (0, 0, 255 - sensitivity)
	// upper_white = (179, sensitivity, 255)
	const int sensitivity = 70;
	// Set default value for MIN HSV trackbars.
	setTrackbarPos("HMin", piano_mask_window, 0);
	setTrackbarPos("SMin", piano_mask_window, 0);
	setTrackbarPos("VMin", piano_mask_window, 255 - sensitivity);
	// Set default value for MAX HSV trackbars.
	setTrackbarPos("HMax", piano_mask_window, 179);
	setTrackbarPos("SMax", piano_mask_window, sensitivity);
	setTrackbarPos("VMax", piano_mask_window, 255);

	// Initialize to check if HSV min / max value changes
	int hMin = 0, sMin = 0, vMin = 0, hMax = 0, sMax = 0, vMax = 0;
	int phMin = 0, psMin = 0, pvMin = 0, phMax = 0, psMax = 0, pvMax = 0;
	// ------------------------------------------------------------------

	cout << "Configure piano: press any key to set.." << endl;
	//while (waitKey(1) < 0 && getWindowProperty(window_name1, WND_PROP_AUTOSIZE) >= 0 && getWindowProperty(window_name2, WND_PROP_AUTOSIZE) >= 0)
	//while (!kbhit() && waitKey(100) && getWindowProperty(piano_window, WND_PROP_AUTOSIZE) >= 0)
	while ((waitKey(100) == -1) && getWindowProperty(piano_window, WND_PROP_AUTOSIZE) >= 0
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
		cv::Scalar lowerThresh;;
		cv::Scalar upperThresh;
		{
			// get current positions of all trackbars
			hMin = getTrackbarPos("HMin", piano_mask_window);
			sMin = getTrackbarPos("SMin", piano_mask_window);
			vMin = getTrackbarPos("VMin", piano_mask_window);

			hMax = getTrackbarPos("HMax", piano_mask_window);
			sMax = getTrackbarPos("SMax", piano_mask_window);
			vMax = getTrackbarPos("VMax", piano_mask_window);

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
			//imshow(piano_mask_window, output);
			imshow("try", myMask);
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
	//while (kbhit()) getch();	//clear buffer
	//fflush(stdin);				//clear stdin's buffer

	cv::destroyWindow(piano_window);
	cv::destroyWindow(piano_mask_window);
	//cv::destroyAllWindows();

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

	//todo: change true/false to activate
	//while (false && !kbhit() && waitKey(5)) {
	while (false && (waitKey(5) == -1)) {
		rs2::frameset data = pipe.wait_for_frames(); // Wait for next set of frames from the camera
		data = align_to_color.process(data);

		rs2::depth_frame mydepth = data.get_depth_frame();
		rs2::frame mycolor = data.get_color_frame();
		scale = mydepth.get_units();

		// Query frame size (width and height)
		const int w_c = mycolor.as<rs2::video_frame>().get_width();
		const int h_c = mycolor.as<rs2::video_frame>().get_height();

		Mat myTest(Size(w_c, h_c), CV_8UC3, (void*)mycolor.get_data(), Mat::AUTO_STEP);
		cvtColor(myTest, myTest, COLOR_BGR2RGB);

		circle(myTest, Point(438, 220), 5, Colors::mLineColorRed, -1);
		circle(myTest, Point(440, 275), 5, Colors::mLineColorRed, -1);

		{
			// Define ROI (region of interest) of point1, for hand
			Mat mask1 = Mat::zeros(h_c, w_c, CV_8U);
			circle(mask1, Point(438, 220), 10, 255, -1);
			Mat handDepthMat1(Size(w_c, h_c), CV_16UC1, (void*)mydepth.get_data(), Mat::AUTO_STEP);
			handDepthMat1.convertTo(handDepthMat1, CV_64F);
			handDepthMat1 = handDepthMat1 * scale;
			Mat nonZeroHand1 = (handDepthMat1 != 0);
			nonZeroHand1 = mask1 & nonZeroHand1;
			// Calculate mean depth inside the ROI
			// This is a very naive way to estimate objects depth
			Scalar handDepthMean1 = mean(handDepthMat1, nonZeroHand1);

			// Define ROI (region of interest) of point1, for piano
			Mat pianoDepthMat1(Size(w_c, h_c), CV_16UC1, (void*)f_depth_piano.get_data(), Mat::AUTO_STEP);
			pianoDepthMat1.convertTo(pianoDepthMat1, CV_64F);
			pianoDepthMat1 = pianoDepthMat1 * scale;
			Mat nonZeroPiano1 = (pianoDepthMat1 != 0);
			nonZeroPiano1 = mask1 & nonZeroPiano1;
			Scalar pianoDepthMean1 = mean(pianoDepthMat1, nonZeroPiano1);

			//float handDepth = hand.get_distance(point.x, point.y);
			float handDepth1 = handDepthMean1[0];
			float pianoDepth1 = pianoDepthMean1[0];
			cout << "(438,220)=" << mydepth.get_distance(438, 220) << " (should be 0.500/1)" << endl;
			cout << "avg (438,220) hand=" << handDepth1 << " (should be 0.500/1)" << endl;
			cout << "avg (438,220) piano=" << pianoDepth1 << endl;
		}



		{
			// Define ROI (region of interest) of point2, for hand
			Mat mask2 = Mat::zeros(h_c, w_c, CV_8U);
			circle(mask2, Point(440, 275), 10, 255, -1);
			Mat handDepthMat2(Size(w_c, h_c), CV_16UC1, (void*)mydepth.get_data(), Mat::AUTO_STEP);
			handDepthMat2.convertTo(handDepthMat2, CV_64F);
			handDepthMat2 = handDepthMat2 * scale;
			// Calculate mean depth inside the detection region
			// This is a very naive way to estimate objects depth
			// but it is intended to demonstrate how one might 
			// use depth data in general
			Mat nonZeroHand2 = (handDepthMat2 != 0);
			nonZeroHand2 = mask2 & nonZeroHand2;
			Scalar handDepthMean2 = mean(handDepthMat2, nonZeroHand2);

			// Define ROI (region of interest) of point2, for piano
			Mat pianoDepthMat2(Size(w_c, h_c), CV_16UC1, (void*)f_depth_piano.get_data(), Mat::AUTO_STEP);
			pianoDepthMat2.convertTo(pianoDepthMat2, CV_64F);
			pianoDepthMat2 = pianoDepthMat2 * scale;
			Mat nonZeroPiano2 = (pianoDepthMat2 != 0);
			nonZeroPiano2 = mask2 & nonZeroPiano2;
			Scalar pianoDepthMean2 = mean(pianoDepthMat2, nonZeroPiano2);

			//float handDepth = hand.get_distance(point.x, point.y);
			float handDepth2 = handDepthMean2[0];
			float pianoDepth2 = pianoDepthMean2[0];
			cout << "(440,275)=" << mydepth.get_distance(440, 275) << " (should be 0.506-8)" << endl;
			cout << "avg (440, 275) hand=" << handDepth2 << " (should be 0.500/1)" << endl;
			cout << "avg (440, 275) piano=" << pianoDepth2 << endl;
		}
		cout << endl << endl;


		imshow("myTest", myTest);
	}
	//while (kbhit()) getch();	//clear buffer
	//fflush(stdin);
	cv::destroyAllWindows();


	//cout << "m_rgb_piano (rows,cols)=(" << m_rgb_piano.rows << "," << m_rgb_piano.cols << ")" << endl;

	PreloadNotes();

#if DEBUG_IMAGE == true
	const auto hand_window = "Play Songs";
	namedWindow(hand_window, WINDOW_AUTOSIZE);
#endif


	//while (!kbhit() && waitKey(5) && getWindowProperty(hand_window, WND_PROP_AUTOSIZE) >= 0)
#if DEBUG_IMAGE == true
	while ((waitKey(5) == -1) && getWindowProperty(hand_window, WND_PROP_AUTOSIZE) >= 0)
#else
	while ((waitKey(5) == -1))
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
		checkKeyPressed(f_depth_piano, f_depth_hand, mHandDetector.getLowestPoint());

	}
	//while (kbhit()) getch();         //clear buffer
	//fflush(stdin);         // clear stdin's buffer

	for (int i = 0; i < 7; i++) {
		delete[] notes_buffers[i];
	}

	return EXIT_SUCCESS;
}
catch (const rs2::error& e)
{
	std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::exception& e)
{
	std::cerr << e.what() << std::endl;
	return EXIT_FAILURE;
}

//int main() try
//{
//	//return main_original();
//	return main_new();
//}
//catch (const rs2::error& e)
//{
//	std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
//	return EXIT_FAILURE;
//}
//catch (const std::exception& e)
//{
//	std::cerr << e.what() << std::endl;
//	return EXIT_FAILURE;
//}