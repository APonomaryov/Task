#include "FileHandler.h"

#include <fstream>
#include <algorithm>

FileHandler::FileHandler(size_t numThreads)
    : m_numThreads(numThreads)
    , m_logFilename(L"Log.log")
{ 
    std::locale::global(std::locale(""));
}

void FileHandler::AddFileName(const wchar_t* filename)
{
    char chFilename[MAX_FILEPATH];
    wcstombs(chFilename, filename, MAX_FILEPATH);
    struct stat fileStat;
    stat(chFilename, &fileStat);
    if (fileStat.st_mode & S_IFREG)
    {
        std::wstring strFilename(filename);
        // Defining a position of the last separator.
        size_t pos = strFilename.find_last_of('\\');
        m_fileInfos.push_back(fileInfoType(strFilename.substr(pos + 1), fileStat));
        if (m_dirPath.empty())
        {
            m_dirPath = strFilename.substr(0, pos + 1);
        }
    }
}

size_t FileHandler::GetNumFiles() const
{
    return m_fileInfos.size();
}

std::wstring FileHandler::GetLogFullPath() const
{
    return m_dirPath + m_logFilename;
}

void FileHandler::SetDirPath(const std::wstring& dirPath)
{
    m_dirPath = dirPath;
}

bool FileHandler::CompareFilenames(const fileInfoType& file1,
                                   const fileInfoType& file2)
{
    std::wstring str1 = file1.first;
    std::wstring str2 = file2.first;
    std::transform(str1.begin(), str1.end(), str1.begin(), towlower);
    std::transform(str2.begin(), str2.end(), str2.begin(), towlower);
    return (str1.compare(str2) < 0);
}

void FileHandler::SortFiles()
{
    std::sort(m_fileInfos.begin(), m_fileInfos.end(), FileHandler::CompareFilenames);
}

void FileHandler::WriteLogFile()
{
    if (m_fileInfos.size() > 0)
    {
        SortFiles();
        std::wofstream logfile;
        std::wstring logPath = m_dirPath + m_logFilename;
        logfile.open(logPath.c_str(), std::fstream::out | std::fstream::app);
        if (logfile.is_open())
        {
            CalculateChecksumForAllFiles();
            logfile << "Total number of files: " << m_fileInfos.size() << "\n";
            logfile << "----------------------------------------------------------------\n";
            // Line format: index_number short_filename size_in_KB modification_time checksum.
            for (size_t i = 0; i < m_fileInfos.size(); ++i)
            {
                time_t rawtime = m_fileInfos[i].second.st_mtime;
                char timeBuffer[100];
                strftime(timeBuffer, 100, "%Y-%m-%d %H:%M:%S", localtime(&rawtime));
                // Rounds upward.
                unsigned long long numKBytes = (m_fileInfos[i].second.st_size + 1023) / 1024;
                logfile << (i + 1) << "\t" 
                        << m_fileInfos[i].first << "\t" 
                        << numKBytes << "KB\t" 
                        << timeBuffer
                        << "\tChecksum: " << m_checkSums[i] << "\n";
            }
            logfile << "----------------------------------------------------------------\n";
            logfile.close();
        }
    }
}

DWORD FileHandler::CalculateChecksum(const std::wstring& path)
{
    DWORD checksum = 0;
    std::wstring fullpath = m_dirPath + path;
    std::ifstream file(fullpath.c_str(), std::ifstream::in | std::ifstream::binary);
    if (!file.is_open()) return 0;
    while (file.good())
    {
        DWORD value = static_cast<DWORD>(file.get());
        if (!file.eof())
        {
            checksum += value;
        }
    }
    file.close();
    return checksum;
}

void FileHandler::CalculateChecksumForFileIndexRange(size_t beginIndex, size_t endIndex)
{
    for (size_t i = beginIndex; i < endIndex; ++i)
    {
        m_checkSums[i] = CalculateChecksum(m_fileInfos[i].first);
    }
}

void FileHandler::CalculateChecksumForAllFiles()
{
    m_threadPool.clear();
    m_checkSums.resize(m_fileInfos.size());
    // Splitting an index range between threads.
    size_t calcPart = m_fileInfos.size() / m_numThreads;
    size_t remainderPart = m_fileInfos.size() % m_numThreads;
    size_t endIndex = 0;
    for (size_t i = 0; i < m_numThreads; ++i)
    {
        size_t beginIndex = endIndex;
        endIndex = beginIndex + calcPart;
        if (i == m_numThreads - 1)
        {
            endIndex += remainderPart;
        }
        m_threadPool.push_back(std::thread(&FileHandler::CalculateChecksumForFileIndexRange, this, beginIndex, endIndex));
    }
    for (size_t i = 0; i < m_numThreads; ++i)
    {
        m_threadPool[i].join();
    }
}