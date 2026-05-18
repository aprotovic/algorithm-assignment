/**
 * DEBRE BERHAN UNIVERSITY
 * COLLEGE OF COMPUTING SCIENCE
 * DEPARTMENT OF SOFTWARE ENGINEERING
 * 
 * Course Name: Design and Analysis of Data Structure and Algorithm
 * Course Code: SEng2022
 * Title: Online Broker Management System
 * 
 * Group Members:
 * 1. Ezra Behailu (DBU17001273)
 * 2. Fiker Mamo (DBU1700671)
 * 3. Firdos Abdu (DBU1700677)
 * 4. Hosaena Anteneh (DBU1700766)
 * 
 * Submitted to: Mr. Bekele
 */

#include <windows.h>
#include "GUI.h"
#include "FileStorage.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    FileStorage storage("database.txt");
    GUI app(hInstance, &storage);
    return app.run();
}
