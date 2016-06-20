#include "mimeexception.hpp"

// using standard exceptions
#include <iostream>
#include <exception>
using namespace std;

MIME_NoSuchFileException::MIME_NoSuchFileException(const std::string& filename) : std::exception()
{
    this->filename = filename;
}

std::string MIME_NoSuchFileException::message()
{
    std::string message = "File does not exist or is not readable.";
          if(!this->filename.empty())
              message += "\n"+this->filename;
    return message;
}

std::string MIME_NoSuchFileException::getFilename()
{
    return this->filename;
}


std::string MIME_PlotFailedException::message()
{
    std::string message = "Plot failed.";
    return message;
}

std::string MIME_NoNameException::message()
{
    std::string message = "No name given for sample set.";
    return message;
}

std::string MIME_PathToExecutableNotFoundException::message()
{
    std::string message = "The path to the MIMEAnTo executable and with this the relative path to gnuplot could not be found.";
    return message;
}

std::string MIME_GnuplotNotFoundException::message()
{
    std::string message = "Gnuplot is not installed or can not be found. Please install gnuplot or add the location to the environment variable PATH.";
    return message;
}




