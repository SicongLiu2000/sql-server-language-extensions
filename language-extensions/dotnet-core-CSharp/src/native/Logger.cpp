//*********************************************************************
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// @File: Logger.cpp
//
// Purpose:
//  Wrapper class around logging to standardize logging messages and errors.
//
//*********************************************************************
#include <string>
#include <iostream>
#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>  // Windows-specific header
#else
#include <unistd.h>   // Linux-specific includes (if needed)
#endif

#include "Logger.h"

using namespace std;

//--------------------------------------------------------------------------------------------------
// Name: Logger::Log
//
// Description:
//  Logs a message to stdout.
//
void Logger::Log(const string &msg)
{
#ifdef DEBUG
    cout << msg << endl;  // Output to standard output in debug mode
#endif
}

//--------------------------------------------------------------------------------------------------
// Name: Logger::LogError
//
// Description:
//  Logs an error to stderr
//
void Logger::LogError(const string &errorMsg)
{
    cerr << "Error: " << errorMsg << endl;  // Output error to standard error
}
