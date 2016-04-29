#ifndef MIMEEXCEPTION_H
#define MIMEEXCEPTION_H

#include <exception>
#include <string>

class MIME_NoSuchFileException : public std::exception
{
    public:
    //const char* what() const noexcept override;
//    const char* what() const throw();
    //MIME_NoSuchFileException();
    MIME_NoSuchFileException(const std::string &filename);
    std::string message();
    std::string getFilename();
    virtual ~MIME_NoSuchFileException() = default;

private:
    std::string filename;
};

class MIME_PlotFailedException : public std::exception
{
public:
     std::string message();
     virtual ~MIME_PlotFailedException() = default;
};

class MIME_NoNameException : public std::exception
{
public:
     std::string message();
     virtual ~MIME_NoNameException() = default;
};

#endif // MIMEEXCEPTION_H
