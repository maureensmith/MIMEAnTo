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
    static const QString BackButtonWarningText;
    static const QString ErrorDuringQualityCriteriaText;
    static const QString ErrorDuringRawKdValuesText;
    static const QString ResultDirDoesNotExistText;

public:
    static int askForDeletingPreviousResults();
    static int errorCoefficientOfVariationWarning();
    static int exitApplicationWarning();
    static int backButtonWarning();
    static void filesAreSavedMessage();
    static void filesAreSavedMessage(const QString& files);

    static void SAMparsingNotYetProvidedInformation();

    static std::string couldNotOpenFileCritical(const QString file);
    static std::string couldNotWriteFileCritical();
    static std::string refFileMissingCritical();
    static std::string projectFileReadCritical();
    static std::string plotWentWrongCritical();
    static std::string invalidIntervalCritical();
    static std::string fileDoesNotExistCritical(const QString& file);
    static std::string sampleDataDoesNotExistCritical(int row);
    static std::string sampleDataMissingCritical(int row);
    static std::string savingWentWrongCritical();
    static std::string readingWentWrongCritical();
    static void unknowException(const QString& msg);
    static std::string notEnoughDataCritical();
    static std::string sampleDataIsWrongCritical();
    static std::string errorDuringQualityCriteriaApplication();
    static std::string errorDuringRawKdValuesCritical();
    static std::string resultDirDoesNotExistCritical();

    static const QString OpenParameterFileText;
    static const QString OpenReferenceFileText;
};

#endif // MESSAGES_H
