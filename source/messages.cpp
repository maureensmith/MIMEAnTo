#include "messages.h"
#include <QMessageBox>

const QString Messages::ErrorVarCoeffText = "Coefficient of variation is exceeding the threshold";
const QString Messages::ErrorVarCoeffInformativeText  = "The coefficient of variation of the error for the bound and/or unbound samples is exceeding  high. You are about to join the errors. Considering the errors respectively for each sample is recommended.\nPush OK if you still want to join the errors and proceed with your computation. Push Cancel if you want to change it.";
const QString Messages::FilesAreSavedText = "Files are saved.";
const QString Messages::FilesAreSavedWithFilenamesText = "Files are saved:\n";
const QString Messages::PlotWentWrongText = "Something went wrong with plot.";
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

void Messages::plotWentWrongCritical()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::PlotWentWrongText);
    msgBox.exec();
}


void Messages::couldNotOpenFileCritical(const QString file)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::CouldNotOpenFileText+file);
    msgBox.exec();
}

void Messages::couldNotWriteFileCritical()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::CouldNotWriteFileText);
    msgBox.exec();
}

void Messages::refFileMissingCritical()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::RefFileMissingText);
    msgBox.exec();
}

void Messages::projectFileReadCritical()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::SomethingWrongWithProjectText);
    msgBox.exec();
}

void Messages::invalidIntervalCritical()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::InvalidIntervalText);
    msgBox.exec();
}

void Messages::fileDoesNotExistCritical(const QString& file)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::FileDoesNotExistText+file);
    msgBox.exec();
}

void Messages::sampleDataDoesNotExistCritical(int row)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText("Sample data in row " +QString::number(row)+ " does not exist. Maybe forgot the data directory?");
    msgBox.exec();
}

void Messages::sampleDataMissingCritical(int row)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText("Sample data in row " +QString::number(row)+ " is missing.");
    msgBox.exec();
}

void Messages::savingWentWrongCritical()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::SavingWentWrongText);
    msgBox.exec();
}

void Messages::readingWentWrongCritical()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::ReadingWentWrongText);
    msgBox.exec();
}

void Messages::unknowException(const QString &msg)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(msg);
    msgBox.exec();
}

void Messages::notEnoughDataCritical()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::NotEnoughDataText);
    msgBox.exec();
}

void Messages::sampleDataIsWrongCritical()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(Messages::SomethingWrongWithSampleData);
    msgBox.exec();
}



