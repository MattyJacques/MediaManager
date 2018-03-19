// Description: List all the duplicate episodes in a directory
// Date: 04/03/2018
// Author: Matthew Jacques

#include <string>
#include <iostream>
#include <corecrt_io.h>
#include <vector>
#include <fstream>
#include <windows.h>
#include <atlstr.h>
#include <limits>

#undef max // undef to use numeric_limits::max()

const std::string OUTFILENAME = "Duplicates.txt";

void GetDirectoryFromUser(std::string* a_Directory)
{ // Prompt the user for the directory

  std::string l_Path;           // Path of the directory to search
  bool l_Success = false;  // Whether the path is a valid directory

  do
  {
    std::cout << "Enter directory to search: ";
    std::getline(std::cin, l_Path);

    if (_access(l_Path.c_str(), 0) == 0)
    { // Check access to the directory specified

      struct stat status;
      stat(l_Path.c_str(), &status);

      if (status.st_mode & S_IFDIR)
      { // Access successful, continue
        l_Success = true;
        *a_Directory = l_Path;
      }
      else
      { // Path was a file, display error and get new path
        std::cout << "The path you entered is a file." << std::endl;
      }
    }
    else
    { // Path did not exist, output error and get new path
      std::cout << "Path doesn't exist." << std::endl;
    }
  } while (!l_Success);
} // GetDirectoryFromUser()


void OutputNamesToFile(const std::vector<std::string>& a_Filenames)
{ // Output all filenames to the file

  std::fstream l_File;
  l_File.open(OUTFILENAME, std::ios::out);

  if (l_File.is_open())
  { // Save filenames, each on their own line
    for (int i = 0; i < (int)a_Filenames.size(); i++)
    {
      l_File << a_Filenames[i] << std::endl;
    }
  }
  else
  { // Failed to open file, inform user
    std::cout << "Failed to open: " << OUTFILENAME << std::endl;
  }
} // OutputNamesToFile()


void OutputResults(const std::vector<std::string>& a_Filenames)
{ // Output the number of files found and wait for user input

  OutputNamesToFile(a_Filenames); // Output all filenames to file

  // Inform user of number of files
  std::cout << std::endl << a_Filenames.size() << " files found, ";
  std::cout << "output saved to " << OUTFILENAME << std::endl;

  // Prompt user to press enter to quit
  std::cout << "Press Enter to quit" << std::endl;

  // Read input, ignoring all chars apart from enter
  std::cin.clear();
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
} // OutputResults()


void ScanDirectory(const std::string& l_Directory, 
                   std::vector<std::string>* l_Filenames)
{ // Scan the directory for files that are not encoded with x265

  WIN32_FIND_DATA l_FileData;               // Holds file data
  std::string l_FullDir = l_Directory;
  std::vector<std::string> l_FileList;
  
  // Make sure we have a full path with wildcard
  if (l_FullDir.back() != '\\' && l_FullDir.back() != '/')
  {
    l_FullDir += '\\';
  }

  l_FullDir += '*';

  // Create new handle and find the first file in the directory
  HANDLE l_FindHandle = FindFirstFile(l_FullDir.c_str(), &l_FileData);

  if (l_FindHandle != INVALID_HANDLE_VALUE)
  { // If there is a file in the directory loop through and push filenames on to
    // the vector of filenames

    do
    { // Add first filename to vector before trying to find another filename

      std::string l_Filename = l_FileData.cFileName;
      if (l_Filename != "." && l_Filename != "..")
      {
        // Add folder name to the path
        std::string l_FullPath = l_Directory + "/" + l_Filename;

        if (l_FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
          std::cout << std::endl << "Searching: " << l_FullPath << std::endl;
          ScanDirectory(l_FullPath, l_Filenames);
        }
        else
        { // If the found object is not a directory, add to filename vector
          if (l_Filename.substr(l_Filename.length() - 4) == ".mkv"
            || l_Filename.substr(l_Filename.length() - 4) == ".mp4"
            || l_Filename.substr(l_Filename.length() - 4) == ".avi")
          l_FileList.push_back(l_Filename);
        }
      }

      // If another file found, do again
    } while (FindNextFile(l_FindHandle, &l_FileData));

    for (int i = 0; i < (int)l_FileList.size(); i++)
    {
      std::size_t l_DashPos = l_FileList[i].find(" - ");

      if (l_DashPos != std::string::npos
        && l_FileList[i].substr(l_FileList[i].length() - 4) == ".mkv"
        || l_FileList[i].substr(l_FileList[i].length() - 4) == ".mp4"
        || l_FileList[i].substr(l_FileList[i].length() - 4) == ".avi")
      {
        std::string l_CheckString = l_FileList[i].substr(0, l_DashPos + 9);

        for (int j = 0; j < (int)l_FileList.size(); j++)
        {
          
          std::string l_OtherCheckString = l_FileList[j].substr(0, l_DashPos + 9);

          if (i != j && l_CheckString == l_OtherCheckString)
          {
            std::cout << "Adding: " << l_FileList[i] << std::endl;
            l_Filenames->push_back(l_FileList[i]);
          }
        }
      }
    }
  } // if (findHandle != INVALID_HANDLE_VALUE)
} // ScanDirectory()


int main()
{ // Process the application

  std::string l_Directory = "";
  std::vector<std::string> l_Filenames;

  //GetDirectoryFromUser(&l_Directory);
  ScanDirectory("\\\\RASPBERRYPI\\PiShare\\TV Shows", &l_Filenames);
  OutputResults(l_Filenames);

  return 0;
} // main()