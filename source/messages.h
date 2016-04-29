#ifndef MESSAGES_H
#define MESSAGES_H

#include <QString>

class Messages
{

private:

    static const QString ErrorVarCoeffText;
    static const QString ErrorVarCoeffInformativeText;
    static const QString FilesAreSavedText;
    static const QString FilesAreSavedWithFilenamesText;
    static const QString PlotWentWrongText;
    static const QString SomethingWrongWithProjectText;
    static const QString InvalidIntervalText;
    static const QString FileDoesNotExistText;
    static const QString CouldNotOpenFileText;
    static const QString CouldNotWriteFileText;
    static const QString RefFileMissingText;
    static const QString SavingWentWrongText;
    static const QString ReadingWentWrongText;
    static const QString NotEnoughDataText;
    static const QString SAMParsingNotYetProvidedText;
    static const QString SomethingWrongWithSampleData;
    static const QString ExitWarningText;

public:
    static int askForDeletingPreviousResults();
    static int errorCoefficientOfVariationWarning();
    static int exitApplicationWarning();
    static void filesAreSavedMessage();
    static void filesAreSavedMessage(const QString& files);

    static void SAMparsingNotYetProvidedInformation();

    static void couldNotOpenFileCritical(const QString file);
    static void couldNotWriteFileCritical();
    static void refFileMissingCritical();
    static void projectFileReadCritical();
    static void plotWentWrongCritical();
    static void invalidIntervalCritical();
    static void fileDoesNotExistCritical(const QString& file);
    static void sampleDataDoesNotExistCritical(int row);
    static void sampleDataMissingCritical(int row);
    static void savingWentWrongCritical();
    static void readingWentWrongCritical();
    static void unknowException(const QString& msg);
    static void notEnoughDataCritical();
    static void sampleDataIsWrongCritical();

    static const QString OpenParameterFileText;
    static const QString OpenReferenceFileText;
};

#endif // MESSAGES_H
