#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h> // Include time.h
#include "LeapC.h"
#include "ExampleConnection.h"` 
#include "Client.h" // Include Client.c directly

#define STACK_SIZE 3
#define DATA_SIZE 8

int64_t lastFrameID = 0; //The last frame received
float Data_Stack[STACK_SIZE + 1][DATA_SIZE]; // Stack to store data
int stackIndex = 0; // Index to keep track of data stack

void handleData(float data[DATA_SIZE]) {
    char dataString[256];
    sprintf_s(dataString, sizeof(dataString), "[%f, %f, %f, %f, %f, %f, %f, %f]", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]); // Use sprintf_s
    handle(dataString); // Send data to server
    //printf("msg to be sent: %s\n", dataString);
}

void processData(LEAP_HAND* hand) {
    float data[DATA_SIZE];
    data[0] = hand->palm.position.x;
    data[1] = hand->palm.position.y;
    data[2] = hand->palm.position.z;

    // Calculate roll, pitch, and yaw for the robot's coordinate system
    LEAP_QUATERNION rotation = hand->palm.orientation;
    //float roll_rad = atan2(2.0f * (rotation.w * rotation.x + rotation.z * -rotation.y), 1.0f - 2.0f * (rotation.x * rotation.x + rotation.z * rotation.z));
    float roll_rad = rotation.z * -3;
    data[3] = (roll_rad); // Roll
    //data[3] = 1.4;
    data[4] = atan2(hand->palm.normal.z, sqrt(hand->palm.normal.x * hand->palm.normal.x + hand->palm.normal.y * hand->palm.normal.y)); // Pitch
    data[5] = atan2(hand->palm.normal.x, hand->palm.normal.y); // Yaw
    data[6] = hand->grab_strength;
    data[7] = (float)time(NULL);
    float roll = data[3];
    //printf("pitch: %f \n", data[5]);
    memcpy(Data_Stack[stackIndex], data, sizeof(data)); // Add data to stack

    if (stackIndex == STACK_SIZE) { // If stack is full
        for (int i = 0; i <= STACK_SIZE; i++) {
            handleData(Data_Stack[i]); // Process each data in stack
        }
        stackIndex = 0; // Reset stack index
    }
    else {
        stackIndex++;
    }
}

 //quaternion roll function
//void processData(LEAP_HAND* hand) {
//    float data[DATA_SIZE];
//    data[0] = hand->palm.position.x;
//    data[1] = hand->palm.position.y;
//    data[2] = hand->palm.position.z;
//
//    // Get the rotation quaternion
//    LEAP_QUATERNION rotation = hand->palm.orientation;
//    printf("Quaternion: w=%f, x=%f, y=%f, z=%f\n", rotation.w, rotation.x, rotation.y, rotation.z);
//
//    // Calculate roll from quaternion and store it in data[3]
//    // Calculate roll from quaternion
//    float roll_rad = atan2(2.0f * (rotation.w * rotation.x + rotation.y * rotation.z), 1.0f - 2.0f * (rotation.x * rotation.x + rotation.y * rotation.y));
//
//    // Convert roll to degrees
//    float roll_deg = roll_rad * (180.0f / 3.141593);
//
//    // Print the roll value in degrees
//    printf("Roll: %f degrees\n", roll_deg);
//
//    data[3] = roll_rad;
//    data[4] = hand->grab_strength;
//    data[5] = (float)time(NULL);
//
//    memcpy(Data_Stack[stackIndex], data, sizeof(data)); // Add data to stack
//
//    if (stackIndex == STACK_SIZE) { // If stack is full
//        for (int i = 0; i <= STACK_SIZE; i++) {
//            handleData(Data_Stack[i]); // Process each data in stack
//        }
//        stackIndex = 0; // Reset stack index
//    }
//    else {
//        stackIndex++;
//    }
//}
//void processData(LEAP_HAND* hand) {
//    float data[DATA_SIZE];
//    data[0] = hand->palm.position.x;
//    data[1] = hand->palm.position.y;
//    data[2] = hand->palm.position.z;
//    data[3] = atan2(hand->palm.normal.x, hand->palm.normal.z);
//    data[4] = hand->grab_strength;
//    data[5] = (float)time(NULL);
//
//    memcpy(Data_Stack[stackIndex], data, sizeof(data)); // Add data to stack
//
//    if (stackIndex == STACK_SIZE) { // If stack is full
//        for (int i = 0; i <= STACK_SIZE; i++) {
//            handleData(Data_Stack[i]); // Process each data in stack
//        }
//        stackIndex = 0; // Reset stack index
//    }
//    else {
//        stackIndex++;
//    }
//}

int main(int argc, char** argv) {
    OpenConnection();
    while (!IsConnected)
        millisleep(100); //wait a bit to let the connection complete

    //printf("Connected.");
    LEAP_DEVICE_INFO* deviceProps = GetDeviceProperties();
    if (deviceProps) {
       // printf("Using device %s.\n", deviceProps->serial);
    }
    //else {
    //    int x=1;
    //    while (1) {
    //        x++;
    //        // No Leap Motion controller detected, send a sample message
    //        float sampleData[DATA_SIZE] = { x, x, x, 0.0, 1.0, (float)time(NULL) };
    //        handleData(sampleData);
    //    }
    //    return 0;  // Exit the program after sending the sample message
    //}

    for (;;) {
        LEAP_TRACKING_EVENT* frame = GetFrame();
        if (frame && (frame->tracking_frame_id > lastFrameID)) {
            lastFrameID = frame->tracking_frame_id;
            //printf("Frame %lli with %i hands.\n", (long long int)frame->tracking_frame_id, frame->nHands);
            for (uint32_t h = 0; h < frame->nHands; h++) {
                LEAP_HAND* hand = &frame->pHands[h];
                //printf("    Hand id %i is a %s hand with position (%f, %f, %f).\n",
                    hand->id,
                    (hand->type == eLeapHandType_Left ? "left" : "right"),
                    hand->palm.position.x,
                    hand->palm.position.y,
                    hand->palm.position.z;
                processData(hand); // Process hand data
            }
        }
    } //ctrl-c to exit
    return 0;
}

//int main(int argc, char** argv) {
//    OpenConnection();
//    while (!IsConnected)
//        millisleep(100); //wait a bit to let the connection complete
//
//
//    printf("Connected.");
//    LEAP_DEVICE_INFO* deviceProps = GetDeviceProperties();
//    if (deviceProps)
//        printf("Using device %s.\n", deviceProps->serial);
//
//    for (;;) {
//        LEAP_TRACKING_EVENT* frame = GetFrame();
//        if (frame && (frame->tracking_frame_id > lastFrameID)) {
//            lastFrameID = frame->tracking_frame_id;
//            printf("Frame %lli with %i hands.\n", (long long int)frame->tracking_frame_id, frame->nHands);
//            for (uint32_t h = 0; h < frame->nHands; h++) {
//                LEAP_HAND* hand = &frame->pHands[h];
//                printf("    Hand id %i is a %s hand with position (%f, %f, %f).\n",
//                    hand->id,
//                    (hand->type == eLeapHandType_Left ? "left" : "right"),
//                    hand->palm.position.x,
//                    hand->palm.position.y,
//                    hand->palm.position.z);
//                processData(hand); // Process hand data
//            }
//        }
//    } //ctrl-c to exit
//    return 0;
//}




