#include "messages.hpp"
#include <QMessageBox>

const QString Messages::ErrorVarCoeffText = "Coefficient of variation is exceeding the threshold";
const QString Messages::ErrorVarCoeffInformativeText  = "The coefficient of variation of the error for the bound and/or unbound samples is exceeding  high. You are about to join the errors. Considering the errors respectively for each sample is recommended.\nPush OK if you still want to join the errors and proceed with your computation. Push Cancel if you want to change it.";
const QString Messages::FilesAreSavedText = "Files are saved.";
const QString Messages::FilesAreSavedWithFilenamesText = "Files are saved:\n";
const QString Messages::PlotWentWrongText = "Something went wrong with plot. The error is logged in the file tmp/gnuplotError.log";
const QString Messages::RefFileMissingText = "Reference file or result directory are missing.";
const QString Messages::InvalidIntervalText = "Invalid interval.";
const QString Messages::CouldNotOpenFileText = "Couldn't open file";
const QString Messages::CouldNotWriteFileText = "Couldn't write file";
const QString Messages::FileDoesNotExistText = "File does not exist:\n";
const QString Messages::OpenParameterFileText = "Open Parameter File";
const QString Messages::OpenReferenceFileText = "Open Reference Sequence File";
const QString Messages::SomethingWrongWithProjectText = "Problems with reading the project file";
const QString Messages::SavingWentWrongText = "Something went wrong with saving.";
const QString Messages::ReadingWentWrongText = "Something went wrong with reading.";
const QString Messages::SomethingWrongWithSampleData = "Something is wrong with the given samples.";
const QString Messages::NotEnoughDataText = "Not enough data.";
const QString Messages::SAMParsingNotYetProvidedText = "SAM parsing is not yet implemented.\nPlease use the provided python scripts to parse your data and give the data directory and the sample barcodes.";
const QString Messages::ExitWarningText = "You are about to exit the program. All unsaved information will be lost.\nQuit anyway?";
const QString Messages::BackButtonWarningText = "You are about to go one step back. All unsaved data computed here will be lost.\nGo back anyway?";
const QString Messages::ErrorDuringQualityCriteriaText = "An error occured during the application of the quality criteria.";
const QString Messages::ErrorDuringRawKdValuesText = "An error occured during the computation of the raw Kd values.";
const QString Messages::ResultDirDoesNotExistText = "The given result directory does not exist.";

int Messages::askForDeletingPreviousResults()
{
    int ret = QMessageBox::Ok;
    QMessageBox msgBox;
    msgBox.setText("The data has been modified.");
    msgBox.setInformativeText("All previously computed results will be deleted if you continue.");
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Warning);
    ret = msgBox.exec();
    return ret;
}

int Messages::errorCoefficientOfVariationWarning()
{
    int ret = QMessageBox::Ok;
    QMessageBox msgBox;
    msgBox.setText(Messages::ErrorVarCoeffText);
    msgBox.setInformativeText(Messages::ErrorVarCoeffInformativeText);
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Warning);
    ret = msgBox.exec();
    return ret;
}

int Messages::exitApplicationWarning()
{
    int ret = QMessageBox::Ok;
    QMessageBox msgBox;
    msgBox.setText(Messages::ExitWarningText);
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Warning);
    ret = msgBox.exec();
    return ret;
}

int Messages::backButtonWarning()
{
    int ret = QMessageBox::Ok;
    QMessageBox msgBox;
    msgBox.setText(Messages::BackButtonWarningText);
    msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Warning);
    ret = msgBox.exec();
    return ret;
}

void Messages::SAMparsingNotYetProvidedInformation()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(Messages::SAMParsingNotYetProvidedText);
    msgBox.exec();
}


void Messages::filesAreSavedMessage()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(Messages::FilesAreSavedText);
    msgBox.exec();
}

void Messages::filesAreSavedMessage(const QString &files)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(Messages::FilesAreSavedWithFilenamesText+files);
    msgBox.exec();
}

std::string Messages::plotWentWrongCritical()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::PlotWentWrongText);
    msgBox.exec();
    return Messages::PlotWentWrongText.toStdString();
}


std::string Messages::couldNotOpenFileCritical(const QString file)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::CouldNotOpenFileText+file);
    msgBox.exec();
    return (Messages::CouldNotOpenFileText+file).toStdString();
}

std::string Messages::couldNotWriteFileCritical()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::CouldNotWriteFileText);
    msgBox.exec();
    return Messages::CouldNotWriteFileText.toStdString();
}

std::string Messages::refFileMissingCritical()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::RefFileMissingText);
    msgBox.exec();
    return Messages::RefFileMissingText.toStdString();
}

std::string Messages::projectFileReadCritical()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::SomethingWrongWithProjectText);
    msgBox.exec();
    return Messages::SomethingWrongWithProjectText.toStdString();
}

std::string Messages::invalidIntervalCritical()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::InvalidIntervalText);
    msgBox.exec();
    return Messages::InvalidIntervalText.toStdString();
}

std::string Messages::fileDoesNotExistCritical(const QString& file)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::FileDoesNotExistText+file);
    msgBox.exec();
    return (Messages::FileDoesNotExistText+file).toStdString();
}

std::string Messages::sampleDataDoesNotExistCritical(int row)
{
    QString text = "Sample data in row " +QString::number(row)+ " does not exist. Maybe forgot the data directory?";
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(text);
    msgBox.exec();
    return text.toStdString();
}

std::string Messages::sampleDataMissingCritical(int row)
{
    QString text = "Sample data in row " +QString::number(row)+ " is missing.";
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(text);
    msgBox.exec();
    return text.toStdString();
}

std::string Messages::savingWentWrongCritical()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::SavingWentWrongText);
    msgBox.exec();
    return Messages::SavingWentWrongText.toStdString();
}

std::string Messages::readingWentWrongCritical()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::ReadingWentWrongText);
    msgBox.exec();
    return Messages::ReadingWentWrongText.toStdString();
}

void Messages::unknowException(const QString &msg)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(msg);
    msgBox.exec();
}

std::string Messages::notEnoughDataCritical()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::NotEnoughDataText);
    msgBox.exec();
    return Messages::NotEnoughDataText.toStdString();
}

std::string Messages::sampleDataIsWrongCritical()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::SomethingWrongWithSampleData);
    msgBox.exec();
    return Messages::SomethingWrongWithSampleData.toStdString();
}

std::string Messages::errorDuringQualityCriteriaApplication()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::ErrorDuringQualityCriteriaText);
    msgBox.exec();
    return Messages::ErrorDuringQualityCriteriaText.toStdString();
}

std::string Messages::errorDuringRawKdValuesCritical()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::ErrorDuringRawKdValuesText);
    msgBox.exec();
    return Messages::ErrorDuringRawKdValuesText.toStdString();
}

std::string Messages::resultDirDoesNotExistCritical()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::ResultDirDoesNotExistText);
    msgBox.exec();
    return Messages::ResultDirDoesNotExistText.toStdString();
}


