/****************************** Module Header ******************************\
Module Name:  FileHandler.h
Project:      CppShellExtContextMenuHandler
Author:		  Ponomaryov Aleksandr 17.11.2013

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma once

#include <vector>
#include <string>
#include <sys/stat.h>
#include <thread>

class FileHandler
{
public:
	// Constructor of FileHandler.
	// Desirable number of threads can be indicated as an input parameter.
	FileHandler(size_t numThreads = 8);

	// Adds filename to FileHandler.
	void AddFileName(const wchar_t* filename);

	// Sets path to a current directory.
	void SetDirPath(const std::wstring& dirPath);

	// Gets a number of files in FileHandler.
	size_t GetNumFiles() const;

	// Writes log file inside a current directory.
	void WriteLogFile();

private:
	typedef std::pair<std::wstring, struct stat> fileInfoType;

	// Compares filenames according to a lexicographical order.
	// Used by SortFiles()
	static bool CompareFilenames(const fileInfoType& file1,
								 const fileInfoType& file2);

	// Max file path length
	static const size_t MAX_FILEPATH = 260;

	// Sorts files according to a lexicographical order.
	// Uses CompareFilenames()
	void SortFiles();

	// Calculates checksums for all files using a specified number of threads.
	// Uses CalculateChecksumForFileIndexRange()
	void CalculateChecksumForAllFiles();

	// Calculates checksums for files in a specified index range in one thread.
	// Uses CalculateChecksum()
	void CalculateChecksumForFileIndexRange(size_t beginIndex, size_t endIndex);

	// Calculates a checksum for a specified file.
	DWORD CalculateChecksum(const std::wstring& path);

	// Current directory path.
	std::wstring m_dirPath;

	// A name of a log file (short name).
	std::wstring m_logFilename;

	// File infos of all files (filename, size, etc.)
	std::vector<fileInfoType> m_fileInfos;

	// Number of threads that are used for a checksum calculation.
	size_t m_numThreads;

	// All threads for a checksum calculation.
	std::vector<std::thread> m_threadPool;

	// Calculated checksums of files.
	std::vector<DWORD> m_checkSums;
};

