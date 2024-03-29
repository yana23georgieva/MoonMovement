﻿// MoonMovement.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "MoonMovement.h"
#include <opencv2/core/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>
#include <windows.h>
#include <commdlg.h>

#include <gdiplus.h>
#include <gdiplusinit.h>

#define MAX_LOADSTRING 100
using namespace cv;
using namespace std;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

//Mat frame;
//// Mat object is a basic image container. frame is an object of Mat.

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MOONMOVEMENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MOONMOVEMENT));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MOONMOVEMENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MOONMOVEMENT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case ID_SHOWTRAECTORY:
            {
                char FileName[220];
                OPENFILENAME ofn;

                wchar_t filePath[MAX_PATH] = L"";
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.lpstrFile = (LPWSTR)filePath;
                ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER;

                if (GetOpenFileName(&ofn) != 0)
                {
                    wcstombs(FileName, filePath, 220);

                    cv::VideoCapture cap(FileName);

                    if (!cap.isOpened())
                    {
                        std::cerr << "Failed to open video file" << std::endl;
                        return 1;
                    }

                    cv::Mat frame, gray, thresh;
                    std::vector<std::vector<cv::Point>> contours;
                    std::vector<cv::Vec3f> circles;

                    while (cap.read(frame))
                    {
                        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
                        cv::threshold(gray, thresh, 127, 255, cv::THRESH_BINARY);

                        cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

                        for (const auto& contour : contours)
                        {
                            std::vector<cv::Point> approx;
                            cv::approxPolyDP(contour, approx, cv::arcLength(contour, true) * 0.02, true);

                            if (approx.size() >= 6 && approx.size() <= 20 && cv::isContourConvex(approx))
                            {
                                double area = cv::contourArea(contour);
                                cv::Rect rect = cv::boundingRect(contour);
                                double radius = rect.width / 2.0;

                                if (radius >= 5 && std::abs(1 - ((double)rect.width / (double)rect.height)) <= 0.2 &&
                                    std::abs(1 - (area / (CV_PI * std::pow(radius, 2)))) <= 0.2)
                                {
                                    circles.push_back(cv::Vec3f(rect.x + radius, rect.y + radius, radius));
                                }
                            }
                        }

                        for (const auto& circle : circles)
                        {
                            cv::circle(frame, cv::Point(circle[0], circle[1]), circle[2], cv::Scalar(0, 0, 255), 2);
                        }

                        for (int i = 1; i < circles.size(); i++)
                        {
                            cv::line(frame, cv::Point(circles[i - 1][0], circles[i - 1][1]),
                                cv::Point(circles[i][0], circles[i][1]), cv::Scalar(0, 255, 0), 10);
                        }

                        cv::imshow("circles", frame);

                        if (cv::waitKey(1) == 27)
                        {
                            break;
                        }
                    }
                }
            }
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

