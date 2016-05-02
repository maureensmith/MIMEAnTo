#include "mimemainwindow.h"
#include "ui_mimemainwindow.h"
#include "mimeexception.h"
#include "ioTools.hpp"
#include "utils.hpp"
#include "processing.hpp"
#include "plot.hpp"
#include <QFileDialog>
#include <string>
#include <QMessageBox>
#include <QSpinBox>
#include <QFormLayout>
#include <QPixmap>
#include <QGraphicsScene>
#include <QtSvg/QGraphicsSvgItem>
#include <QString>
#include <QCloseEvent>
#include <QDebug>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QCheckBox>
#include <QInputDialog>
#include <datatablelineedit.h>
#include <sampleactivationcheckbox.h>
#include <messages.h>
#include <boost/filesystem.hpp>
#include "boost/filesystem/fstream.hpp"
namespace fs = boost::filesystem;

int const MIMEMainWindow::RESTART_CODE = -123456789;


MIMEMainWindow::MIMEMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MIMEMainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    ui->backStepPushButton->setVisible(false);
    ui->dataTableWidget->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
    ui->dataTableWidget->horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);
    ui->dataTableWidget->horizontalHeader()->setResizeMode(3, QHeaderView::ResizeToContents);
    ui->dataTableWidget->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->dataTableWidget->verticalHeader()->setResizeMode(1, QHeaderView::Fixed);
    ui->dataTableWidget->verticalHeader()->setResizeMode(2, QHeaderView::Fixed);
    addNewRow();

    ui->cutFrontSpinBox->setValue(parameter.cutValueFwd);
    ui->cutBackSpinBox->setValue(parameter.cutValueBwd);

}


MIMEMainWindow::~MIMEMainWindow()
{
    delete ui;
}

void MIMEMainWindow::closeEvent(QCloseEvent *event) {
    int ret = Messages::exitApplicationWarning();
    if(ret == QMessageBox::Ok) {
        if(ioTools::dirExists(ui->resultDirLineEdit->text().toStdString()))
            ioTools::removeTmpFiles(ui->resultDirLineEdit->text().toStdString());
        event->accept();
    } else {
        event->ignore();
    }
}

void MIMEMainWindow::on_actionNew_Project_triggered()
{
    bool closed = this->close();
    if(closed)
        qApp->exit(MIMEMainWindow::RESTART_CODE);
}

/************ footer button handles *********/

void MIMEMainWindow::handleStackedWidgetAndFooter(int index)
{
    ui->stackedWidget->setCurrentIndex(index);
    ui->nextStepPushButton->setEnabled(false);
    ui->backStepPushButton->setEnabled(true);
    ui->nextStepPushButton->setVisible(index != 2);
    ui->backStepPushButton->setVisible(index != 0);
    //ui->backStepPushButton->setEnabled(index != 0);
    MIMEMainWindow::on_saveProjectPushButton_clicked();
}

void MIMEMainWindow::on_nextStepPushButton_clicked()
{
    int index = ui->stackedWidget->currentIndex();
    ++index;
    if(index == 1)
    {
        this->initErrorEstimationPage();
        handleStackedWidgetAndFooter(index);
    }
    else if(index == 2)
    {
        //check if errors vary too much and warn if the error should still be joined
        if(this->joinErrorWarning())
        {
            handleStackedWidgetAndFooter(index);
            this->initKDComputationPage();
        }
    }
}



void MIMEMainWindow::on_backStepPushButton_clicked()
{
    int index = ui->stackedWidget->currentIndex();
    ui->stackedWidget->setCurrentIndex(--index);
    ui->backStepPushButton->setEnabled(index != 0);
    ui->nextStepPushButton->setEnabled(true);
    ui->nextStepPushButton->setVisible(index != 2);
    ui->backStepPushButton->setVisible(index != 0);
}

void MIMEMainWindow::on_saveProjectPushButton_clicked()
{
    try
    {
        string resultDir = ui->resultDirLineEdit->text().toStdString();
        if(ioTools::dirExists(resultDir))
            ioTools::writeParameterFile(resultDir, parameter, data);
    }
    catch(...)
    {
        Messages::savingWentWrongCritical();
    }
}

/********** general methods ********************/

QWidget* MIMEMainWindow::createDataCell() {
    QWidget *cell = new QWidget;
    QFormLayout *layout = new QFormLayout;

    DataTableLineEdit *boundLineEdit = new DataTableLineEdit(utils::BOUND, ui->dataTableWidget);
    DataTableLineEdit *unboundLineEdit = new DataTableLineEdit(utils::UNBOUND, ui->dataTableWidget);
    //Connect the pushButton with the editLine next to it
    QPushButton *unboundButton = new QPushButton("non-selected", cell);
    QObject::connect(unboundButton, SIGNAL(clicked()), unboundLineEdit, SLOT(on_samplePushButton_clicked()));
    QPushButton *boundButton = new QPushButton("selected", cell);
    QObject::connect(boundButton, SIGNAL(clicked()), boundLineEdit, SLOT(on_samplePushButton_clicked()));
    boundButton->setGeometry(unboundButton->geometry());
    layout->addRow(unboundButton, unboundLineEdit);
    layout->addRow(boundButton, boundLineEdit);
    cell->setLayout(layout);
    return cell;
}

QWidget* MIMEMainWindow::createSelectionArea() {
    QWidget *cell = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    //note: no sampleactivation checkbox, because at this point the samples might not be created yet
    QCheckBox *rowCheckBox = new QCheckBox(ui->dataTableWidget);
    rowCheckBox->setChecked(true);
    QPushButton *rowDeleteButton = new QPushButton("delete", ui->dataTableWidget);
    QObject::connect(rowDeleteButton, SIGNAL(clicked()), this, SLOT(onDeleteRowPushButton_clicked()));
    layout->addWidget(rowCheckBox);
    layout->addWidget(rowDeleteButton);
    cell->setLayout(layout);
    return cell;
}


int MIMEMainWindow::addNewRow() {
    QTableWidget *table = ui->dataTableWidget;
    int rows = table->rowCount();
    table->insertRow(rows);

    // Editable Textlabel for the name of the samples
    QLineEdit *nameLineEdit = new QLineEdit(table);
    nameLineEdit->setPlaceholderText(tr("(enter name)"));

    table->setCellWidget(rows, 0, nameLineEdit);
    table->setCellWidget(rows, 1, createDataCell());
    table->setCellWidget(rows, 2, createDataCell());
    table->setCellWidget(rows, 3, createSelectionArea());

    table->hide();
    table->show();

    return rows;
}

void clearLayout(QLayout* layout, bool deleteWidgets = true) {

    while(QLayoutItem* item = layout->takeAt(0)) {
        if(deleteWidgets) {
            if(QWidget* widget = item->widget()) {
                delete widget;
            }
        }

        if(QLayout* childLayout = item->layout())
            clearLayout(childLayout, deleteWidgets);
        delete item;
    }
}

void clearGraphicsView(QGraphicsView* view) {
    if(view->items().size() > 0) {
        view->scene()->clear();
        view->items().clear();
    }
}

bool MIMEMainWindow::joinErrorWarning()
{
    bool ok = false;
    int ret = QMessageBox::Ok;
    //if the checkstate is changed there is a warning that priviously computed results are deleted, because of discordance
    if(this->parameter.joinErrors != ui->joinErrorsCheckBox->isChecked())
    {
        ret = Messages::askForDeletingPreviousResults();
        if(ret == QMessageBox::Ok) {
            ioTools::removeKDFiles(ui->resultDirLineEdit->text().toStdString());
        }
    }

    if(ret == QMessageBox::Ok && (data.maxCoVarErrorBound > parameter.coeffThreshold || data.maxCoVarErrorUnbound > parameter.coeffThreshold))
    {
        ui->errorTabWidget->setCurrentIndex(1);
        ret = Messages::errorCoefficientOfVariationWarning();
    }

    if(ret == QMessageBox::Ok)
    {
        this->parameter.joinErrors = ui->joinErrorsCheckBox->isChecked();
        ok = true;
    }

    return ok;
}

void MIMEMainWindow::initErrorEstimationPage() {
    //delete checkboxes from checkboxlayout if already filled.
    //clearLayout(ui->sampleCheckBoxLayout, true);
    clearGraphicsView(ui->errorEstimationGraphicsView);
    clearGraphicsView(ui->errorCoeffVarGraphicsView);
    //Show tab for error estimation
    ui->errorTabWidget->setCurrentIndex(0);
    //clear computed errors (e.g. from going back and forth)
    data.clearErrors();

    //enable button to load data if error file exist in reslut directory
    ui->loadErrorPushButton->setEnabled(ioTools::errorFilesExist(ui->resultDirLineEdit->text().toStdString()));

    //checkstate to join errors of the last computation
    ui->joinErrorsCheckBox->setChecked(parameter.joinErrors);
     QLayout *sampleCheckboxLayout = ui->sampleCheckBoxGroupBox->layout();
     clearLayout(sampleCheckboxLayout);
    for(utils::Sample bound : data.bound) {
        if(bound.library == 0) {
            SampleActivationCheckBox *checkBox = new SampleActivationCheckBox(data, QString::fromStdString(bound.name));
            QObject::connect(checkBox, SIGNAL(toggled(bool)), checkBox, SLOT(on_sampleActivationCheckBox_toggled(bool)));
            QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
            sizePolicy.setHorizontalStretch(0);
            sizePolicy.setVerticalStretch(0);
            sizePolicy.setHeightForWidth(checkBox->sizePolicy().hasHeightForWidth());
            checkBox->setChecked(bound.active);
            checkBox->setSizePolicy(sizePolicy);
            checkBox->show();

            sampleCheckboxLayout->addWidget(checkBox);
        }
    }
    ui->weightThrErrorSpinBox->setValue(parameter.weightThreshold*100);

    QSpacerItem *verticalSpacer;
    verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    sampleCheckboxLayout->addItem(verticalSpacer);
}

void MIMEMainWindow::initKDComputationPage() {
    ui->weightThrKDSpinBox->setValue(ui->weightThrErrorSpinBox->value());
    //clear already computed stuff
    clearGraphicsView(ui->KDGraphicsView);

    //disable buttons, only valid if raw KDs are computed
    MIMEMainWindow::enableParameter(false);
    ui->saveRawKDvaluesPushButton->setEnabled(false);
    enableKdSaveButtons(false);
    //use start and end of sequence as default values (maybe later the interval which is seen on screen?)
    ui->saveKDImageFromSpinBox->setValue(parameter.seqBegin);
    ui->saveKDImageFromSpinBox->setMinimum(parameter.seqBegin);
    ui->saveKDImageFromSpinBox->setMaximum(parameter.seqEnd);
    ui->saveKDImageToSpinBox->setValue(parameter.seqEnd);
    ui->saveKDImageToSpinBox->setMinimum(parameter.seqBegin);
    ui->saveKDImageToSpinBox->setMaximum(parameter.seqEnd);
    //enable button to load data if raw KD value file exist in reslut directory
    ui->loadRawKDValuesPushButton->setEnabled(ioTools::kDFileExists(ui->resultDirLineEdit->text().toStdString()));

    ui->alphaDoubleSpinBox->setValue(parameter.alpha);
    ui->weightThrKDSpinBox->setValue(parameter.weightThreshold*100);
    ui->s2nDoubleSpinBox->setValue(parameter.minSignal2NoiseStrength);
    ui->minCoverageSpinBox->setValue(parameter.minimumNrCalls);
    ui->minKDSpinBox->setValue(parameter.minNumberEstimatableKDs);

    ui->KDyAxisFromDoubleSpinBox->setValue(parameter.plotYAxisFrom);
    ui->KDyAxisToDoubleSpinBox->setValue(parameter.plotYAxisTo);
}




/************ init project button handles *********/

void MIMEMainWindow::enableSanitySaveButtons(bool isEnabled){
    ui->saveSanityPushButton->setEnabled(isEnabled);
    ui->epsSanityCheckBox->setEnabled(isEnabled);
    ui->pdfSanityCheckBox->setEnabled(isEnabled);
}

void MIMEMainWindow::on_resultDirPushButton_clicked()
{
    QString text = ui->resultDirLineEdit->text();

    if(text.isEmpty() && !ui->dataDirLineEdit->text().isEmpty())
        text = ui->dataDirLineEdit->text();
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Result Directory"),
                                                    text,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(!dir.isEmpty())
        ui->resultDirLineEdit->setText(dir);

    // if parameter file exists in result dir: offer to read them
//    ui->loadProjectPushButton->setEnabled(ioTools::fileExists(ui->resultDirLineEdit->text().toStdString(), "parameter.txt"));

        ui->loadProjectPushButton->setEnabled(ioTools::dirExists(dir.toStdString()));
}

void MIMEMainWindow::on_addRowPushButton_clicked()
{
    addNewRow();
}

void MIMEMainWindow::onDeleteRowPushButton_clicked() {
    QObject* obj = sender();
    bool found = false;
    int rowToDelete = -1;
    for(int row= 0; row<ui->dataTableWidget->rowCount() && !found; ++row) {
        for(int i = 0; i < ui->dataTableWidget->cellWidget(row, 3)->children().count() && !found; ++i) {
            QObject *cellObj = (ui->dataTableWidget->cellWidget(row, 3)->children())[i];
            if(obj == cellObj) {
                rowToDelete = row;
                found = true;
            }

        }
    }
    if(rowToDelete > -1)
        ui->dataTableWidget->removeRow(rowToDelete);
    this->update();
    //if a row is deleted, the data has to be loaded again. so next button inactive
    ui->nextStepPushButton->setEnabled(false);
}



void MIMEMainWindow::on_referencePushButton_clicked()
{
    QString text = ui->referenceLineEdit->text();
    if(text.isEmpty() && !ui->dataDirLineEdit->text().isEmpty())
        text = ui->dataDirLineEdit->text();
    QString fileName = QFileDialog::getOpenFileName(this, Messages::OpenReferenceFileText, text);

    try
    {
        if (!fileName.isEmpty())
        {
            QFile file(fileName);
            if (!file.open(QIODevice::ReadOnly)) {
                throw MIME_NoSuchFileException(fileName.toStdString());
            }

            file.close();

            ui->referenceLineEdit->setText(fileName);

            if(ui->dataDirLineEdit->text().isEmpty()) {
                 fs::path file(fileName.toStdString());
                 fs::path path = file.parent_path();
                 std::string nativePath = path.string();
                 ui->dataDirLineEdit->setText(QString::fromStdString(nativePath));
            }

        }
    }
    catch(MIME_NoSuchFileException& e)
    {
        Q_UNUSED(e)
        Messages::couldNotOpenFileCritical(fileName);
    }
}

void MIMEMainWindow::on_loadDataPushButton_clicked()
{
    //if da is changed and before errors or KD values have been computed, they will be deleted
    QObject* obj = sender();
    int ret = QMessageBox::Ok;
    //only ask of there is already something computed
    if(obj == ui->loadDataPushButton && (ioTools::errorFilesExist(ui->resultDirLineEdit->text().toStdString()) || ioTools::kDFileExists(ui->resultDirLineEdit->text().toStdString()))) {

        ret = Messages::askForDeletingPreviousResults();

        if(ret == QMessageBox::Ok) {
            ioTools::removeErrorFiles(ui->resultDirLineEdit->text().toStdString());
            ioTools::removeKDFiles(ui->resultDirLineEdit->text().toStdString());
        }
    }

    if(ret == QMessageBox::Ok)
    {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        MIMEMainWindow::enableSanitySaveButtons(false);

        ui->saveProjectPushButton->setEnabled(false);
        ui->nextStepPushButton->setEnabled(false);
        if(!ui->referenceLineEdit->text().isEmpty() && !ui->resultDirLineEdit->text().isEmpty())
        {
            //set result directory in window title (for later orientation)
           QString title("MIME - " + ui->resultDirLineEdit->text());
           this->setWindowTitle(title);
            bool error = false;

            //try to read reference file, if existing
            string refFileNative;
            if(ioTools::fileExists(ui->referenceLineEdit->text().toStdString()))
            {
                refFileNative = ioTools::readReference(ui->referenceLineEdit->text().toStdString(), data.ref);
            } else {
                error = true;
                Messages::fileDoesNotExistCritical(ui->referenceLineEdit->text());
            }
            parameter.refFile = refFileNative;
            parameter.dataDir = ui->dataDirLineEdit->text().toStdString();
            if(!error && (ui->cutFrontSpinBox->value() + ui->cutBackSpinBox->value()) >= (int)data.ref.size()) {
                error = true;
                Messages::invalidIntervalCritical();

            } else {
                parameter.cutValueFwd = ui->cutFrontSpinBox->value();
                parameter.cutValueBwd = ui->cutBackSpinBox->value();


                parameter.seqBegin = parameter.cutValueFwd+1;
                parameter.seqEnd = data.ref.size() - parameter.cutValueBwd;

                //delete possibly already saved data and widgets
                data.bound.clear();
                data.unbound.clear();
            }
            //at least one pair of data should be available
            bool dataExisting = false;

            try {
                for(int row= 0; row<ui->dataTableWidget->rowCount() && !error; ++row)
                {
                        QLineEdit *nameLine = qobject_cast<QLineEdit *>(ui->dataTableWidget->cellWidget(row, 0));
                        // name is mandatory
                        if(nameLine->text().toStdString().empty())
                            throw MIME_NoNameException();

                        string name = nameLine->text().toStdString();

                        //iterate through cell samples
                        for(int i = 0; i < ui->dataTableWidget->cellWidget(row, 1)->children().count()  && !error; ++i)
                        {
                            QObject *obj = (ui->dataTableWidget->cellWidget(row, 1)->children())[i];
                            if(obj->metaObject()->className() == DataTableLineEdit::staticMetaObject.className()) {
                                DataTableLineEdit *tableLineWt = qobject_cast<DataTableLineEdit *>(obj);
                                DataTableLineEdit *tableLineMut = qobject_cast<DataTableLineEdit *>((ui->dataTableWidget->cellWidget(row, 2)->children())[i]);
                                if(tableLineWt->text().isEmpty() || tableLineMut->text().isEmpty()) {
                                    Messages::sampleDataMissingCritical(row+1);
                                    error = true;
                                } else if(ui->dataDirLineEdit->text().isEmpty() && (!ioTools::fileExists(tableLineMut->text().toStdString()) || !ioTools::fileExists(tableLineWt->text().toStdString()))) {
                                    Messages::sampleDataDoesNotExistCritical(row+1);
                                    error = true;
                                } else if(ioTools::fileExists(tableLineMut->text().toStdString()) || ioTools::fileExists(tableLineWt->text().toStdString())) {
                                    //TODO: SAM parsing
                                    Messages::SAMparsingNotYetProvidedInformation();
                                    error = true;
                                } else {

                                    parameter.dataDir = ui->dataDirLineEdit->text().toStdString();
                                    utils::Sample sampleWt(name, tableLineWt->text().toInt(), tableLineWt->getType(), 0);
                                    utils::Sample sampleMut(name, tableLineMut->text().toInt(), tableLineMut->getType(),row+1);
                                    if(tableLineWt->getType() == utils::BOUND) {
                                        data.bound.insert(sampleWt);
                                        data.bound.insert(sampleMut);

                                    } else if(tableLineWt->getType() == utils::UNBOUND) {
                                        data.unbound.insert(sampleWt);
                                        data.unbound.insert(sampleMut);

                                    }
                                    dataExisting = true;
                                }

                            }
                        }
                        QCheckBox *sampleCheckBox = qobject_cast<QCheckBox *>(((ui->dataTableWidget->cellWidget(row, 3))->children())[1]);
                        if(!sampleCheckBox->isChecked())
                            data.activateSamples(name, false);

                } //for row

            } //try

            catch(MIME_NoNameException& e)
            {
                 QApplication::restoreOverrideCursor();
                Messages::unknowException(QString::fromStdString(e.message()));
                error = true;
            }
            catch(...)
            {
                 QApplication::restoreOverrideCursor();
                Messages::sampleDataIsWrongCritical();
                error = true;
            }



            if(!error) {
                if(dataExisting && data.bound.size() == data.unbound.size()) {                    

                    string resultDir = ui->resultDirLineEdit->text().toStdString();
                    string dataDir = ui->dataDirLineEdit->text().toStdString();
                    string mutRateSamplePlotFile;
                    try
                    {
                        mutRateSamplePlotFile = plot::plotMutationRatePerSampleBoxplot(resultDir, dataDir, parameter, data, plot::SVG);


//                    string mutRateSamplePlotFile ="/home/mi/msmith/Dokumente/MIME/MIME_Test/Data/1/mutRateBoxPlot.svg";
//                        std::cout << mutRateSamplePlotFile << std::endl;
                        if(!mutRateSamplePlotFile.empty()) {
                            QFile file(QString::fromStdString(mutRateSamplePlotFile));
                            if (!file.open(QIODevice::ReadOnly)) {
                                QApplication::restoreOverrideCursor();
                                QMessageBox::critical(this, tr("Error"), tr("Could not open plot file, something wrong with project data"));
                                return;
                            }

                            file.close();

            //                QImage image(QString::fromStdString(mutRateSamplePlotFile));
                            QGraphicsSvgItem *item = new QGraphicsSvgItem(QString::fromStdString(mutRateSamplePlotFile));
                            QGraphicsScene* scene = new QGraphicsScene();

            //                QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(image));
                             scene->addItem(item);
                             clearGraphicsView(ui->sanityGraphicsView);
                             ui->sanityGraphicsView->setScene(scene);
                              //scaling the svg graphic on the graphicsview size
                              ui->sanityGraphicsView->fitInView(item, Qt::KeepAspectRatio);

                              MIMEMainWindow::enableSanitySaveButtons(true);
                              ui->saveProjectPushButton->setEnabled(true);
                              MIMEMainWindow::on_saveProjectPushButton_clicked();
                              //all needed data is there: we can go to the next step
                              ui->nextStepPushButton->setEnabled(true);
                        } else {
                            QApplication::restoreOverrideCursor();
                            Messages::plotWentWrongCritical();
                        }
                    }
                    catch(MIME_NoSuchFileException& e)
                    {
                        QApplication::restoreOverrideCursor();
                        Messages::fileDoesNotExistCritical(QString::fromStdString(e.getFilename()));
                    }
                    catch(exception& e)
                    {
                        QApplication::restoreOverrideCursor();
                        Messages::unknowException(QString::fromStdString(e.what()));
                    }
                } else {
                    QApplication::restoreOverrideCursor();
                    Messages::notEnoughDataCritical();
                }
            }

        } else {
            QApplication::restoreOverrideCursor();
            Messages::refFileMissingCritical();
        }
        QApplication::restoreOverrideCursor();
    }
}

void MIMEMainWindow::on_dataDirPushButton_clicked()
{
    QString text = ui->dataDirLineEdit->text();

//    if(text.isEmpty())
//        text = ui->dirLineEdit->text();
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Data Directory"),
                                                    text,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(!dir.isEmpty())
        ui->dataDirLineEdit->setText(dir);
}




void MIMEMainWindow::on_loadProjectPushButton_clicked()
{
    std::cout << "Load Project..." << std::endl;

    QString fileName = QFileDialog::getOpenFileName(this, Messages::OpenParameterFileText, ui->resultDirLineEdit->text());


    if (!fileName.isEmpty()) {

        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }

        file.close();
        QApplication::setOverrideCursor(Qt::WaitCursor);
        MIMEMainWindow::enableSanitySaveButtons(false);
        //read parameter file (which also contains reference and sampleFiles)

        bool readSuccessful = false;
        try {
            readSuccessful = ioTools::readParameterFile(fileName.toStdString(), parameter, data);
        } catch(...)
        {
            QApplication::restoreOverrideCursor();
            Messages::projectFileReadCritical();
        }
        if(readSuccessful)
        {
            try {

                ui->referenceLineEdit->setText(QString::fromStdString(parameter.refFile));
                ui->dataDirLineEdit->setText(QString::fromStdString(parameter.dataDir));
                ui->cutFrontSpinBox->setValue(parameter.cutValueFwd);
                ui->cutBackSpinBox->setValue(parameter.cutValueBwd);
                if(!(data.bound.empty() || data.unbound.empty()))
                {
                    QTableWidget *table = ui->dataTableWidget;
                    table->clearContents();
                    table->setRowCount(0);
                    int rows = table->rowCount();
                    QString boundWtSample;
                    QString unboundWtSample;
                    for(auto boundIt=data.bound.begin(), unboundIt=data.unbound.begin(); boundIt != data.bound.end(); ++boundIt, ++unboundIt)
                    {
                        if(boundIt->library == 0) {
                           rows = this->addNewRow();
                           QLineEdit *line = qobject_cast<QLineEdit *>(table->cellWidget(rows, 0));
                           line->setText(QString::fromStdString(boundIt->name));
                           //should work, as the set of samples is ordered by library
                           boundWtSample = QString::number(boundIt->barcode);
                           unboundWtSample = QString::number(unboundIt->barcode);
                        } else {
                            for(int i = 0; i < ui->dataTableWidget->cellWidget(rows, 1)->children().count(); ++i) {
                                QObject *obj = (ui->dataTableWidget->cellWidget(rows, 1)->children())[i];
                                if(obj->metaObject()->className() == DataTableLineEdit::staticMetaObject.className()) {
                                    DataTableLineEdit *tableLineWt = qobject_cast<DataTableLineEdit *>(obj);
                                    DataTableLineEdit *tableLineMut = qobject_cast<DataTableLineEdit *>((ui->dataTableWidget->cellWidget(rows, 2)->children())[i]);
                                    if(tableLineWt->getType() == utils::BOUND) {
                                        tableLineWt->setText(boundWtSample);
                                        tableLineMut->setText(QString::number(boundIt->barcode));
                                    } else if(tableLineWt->getType() == utils::UNBOUND) {
                                        tableLineWt->setText(unboundWtSample);
                                        tableLineMut->setText(QString::number(unboundIt->barcode));
                                    }
                                }
                            }

                        }
                    }
                }
                this->on_loadDataPushButton_clicked();
            } catch(exception& e)
            {
                QApplication::restoreOverrideCursor();
                Q_UNUSED(e);
                Messages::sampleDataIsWrongCritical();
            }

        }
        QApplication::restoreOverrideCursor();
    }

}



void MIMEMainWindow::on_saveSanityPushButton_clicked()
{
    try {
        bool ok;
        QString text = QInputDialog::getText(this, tr("Save plot"), tr("Enter file name suffix for plot of mutation rate:"), QLineEdit::Normal, "", &ok);
        if(ok) {
            QApplication::setOverrideCursor(Qt::WaitCursor);
            string resultDir = ui->resultDirLineEdit->text().toStdString();
            string dataDir = ui->dataDirLineEdit->text().toStdString();

            MIMEMainWindow::on_saveProjectPushButton_clicked();
            string mutRateSamplePlotFile;
            if(ui->epsSanityCheckBox->isChecked())
                mutRateSamplePlotFile = plot::plotMutationRatePerSampleBoxplot(resultDir, dataDir, parameter, data, plot::EPS, text.toStdString());
            if(ui->pdfSanityCheckBox->isChecked())
                mutRateSamplePlotFile += "\n" + plot::plotMutationRatePerSampleBoxplot(resultDir, dataDir, parameter, data, plot::PDF, text.toStdString());
            QApplication::restoreOverrideCursor();
            if(!mutRateSamplePlotFile.empty()) {
                Messages::filesAreSavedMessage(QString::fromStdString(mutRateSamplePlotFile));
            }
        }
    }
    catch(...)
    {
        QApplication::restoreOverrideCursor();
        Messages::plotWentWrongCritical();
    }
}


/************ error button handles *********/

void MIMEMainWindow::enableErrorSaveButtons(bool isEnabled){
    ui->saveErrorPlotPushButton->setEnabled(isEnabled);
    ui->epsErrorCheckBox->setEnabled(isEnabled);
    ui->pdfErrorCheckBox->setEnabled(isEnabled);
    ui->saveErrorsPushButton->setEnabled(isEnabled);
    ui->joinErrorsCheckBox->setEnabled(isEnabled);
}

void MIMEMainWindow::on_estimateErrorPushButton_clicked()
{
    MIMEMainWindow::enableErrorSaveButtons(false);
    ui->nextStepPushButton->setEnabled(false);

    //if coming from loadError, everything is fine, if from estimate next results (Kds) will be deleted
    QObject* obj = sender();
    int ret = QMessageBox::Ok;
    //if(obj == ui->estimateErrorPushButton) {
    if(obj == ui->estimateErrorPushButton && (parameter.weightThreshold*100 != ui->weightThrErrorSpinBox->value() || !data.errorExist())) {
        if(ioTools::errorFilesExist(ui->resultDirLineEdit->text().toStdString()))
        {
            ret = Messages::askForDeletingPreviousResults();
        }
        if(ret == QMessageBox::Ok) {
            //error has just to be recalculated if threshold is changed. other changes doesn't affect the error results
//                if(parameter.weightThreshold != ui->weightThrErrorDoubleSpinBox->value() || !data.errorExist())
//                {
                QApplication::setOverrideCursor(Qt::WaitCursor);
                ioTools::removeErrorFiles(ui->resultDirLineEdit->text().toStdString());
                ioTools::removeKDFiles(ui->resultDirLineEdit->text().toStdString());
                parameter.weightThreshold = ui->weightThrErrorSpinBox->value()/100;
                try {
                    processing::estimateError(ui->dataDirLineEdit->text().toStdString(), parameter, data);
                }
                catch(MIME_NoSuchFileException& e) {
                    QApplication::restoreOverrideCursor();
                    Messages::fileDoesNotExistCritical(QString::fromStdString(e.getFilename()));
                    ret = QMessageBox::Cancel;
                }
                catch(exception& e)
                {
                    QApplication::restoreOverrideCursor();
                    Messages::unknowException(QString::fromStdString(e.what()));
                    ret = QMessageBox::Cancel;
                }

//                //TODO: hier oder bei save? -> Warnung, wenn noch nicht gesavt wurde und auf next gedrückt
//                try
//                {
//                    ioTools::writeErrorEstimates(ui->resultDirLineEdit->text().toStdString(), data);
//                }
//                catch(...)
//                {
//                    QApplication::restoreOverrideCursor();
//                    Messages::savingWentWrongCritical();
//                    ret = QMessageBox::Cancel;
//                }
        }
    }

    if(ret == QMessageBox::Ok) {
        try{
            //QApplication::setOverrideCursor(Qt::WaitCursor);
            //plot median error for bound and unbound
            string errorEstimationPlotFile = plot::plotMedianErrorPerSample(ui->resultDirLineEdit->text().toStdString(), parameter, data, plot::SVG);

//            std::cout << errorEstimationPlotFile << std::endl;
            if(!errorEstimationPlotFile.empty()) {
                QGraphicsSvgItem *item = new QGraphicsSvgItem(QString::fromStdString(errorEstimationPlotFile));
                QGraphicsScene* scene = new QGraphicsScene();
                scene->addItem(item);
                clearGraphicsView(ui->errorEstimationGraphicsView);
                ui->errorEstimationGraphicsView->setScene(scene);
                ui->errorEstimationGraphicsView->fitInView(item, Qt::KeepAspectRatio);
            } else {

                throw MIME_PlotFailedException();
//               QApplication::restoreOverrideCursor();
//               Messages::plotWentWrongCritical();
            }

            //plot coefficient of variation
            string errorCoeffVarPlotFile = plot::plotCoefficientOfVariation(ui->resultDirLineEdit->text().toStdString(), parameter, data, plot::SVG);
            //trick, to get a full sized plot on the second tab and not only a thumbnail
            int currIdx = ui->errorTabWidget->currentIndex();
            ui->errorTabWidget->setCurrentIndex(1);
            if(!errorCoeffVarPlotFile.empty()) {
                QGraphicsSvgItem *item = new QGraphicsSvgItem(QString::fromStdString(errorCoeffVarPlotFile));
                QGraphicsScene* scene = new QGraphicsScene();
                scene->addItem(item);
                clearGraphicsView(ui->errorCoeffVarGraphicsView);
                ui->errorCoeffVarGraphicsView->setScene(scene);
                ui->errorCoeffVarGraphicsView->fitInView(item, Qt::KeepAspectRatio);


            } else {
                QApplication::restoreOverrideCursor();
                Messages::plotWentWrongCritical();
            }
            ui->errorTabWidget->setCurrentIndex(currIdx);
            MIMEMainWindow::enableErrorSaveButtons(!errorCoeffVarPlotFile.empty() && !errorEstimationPlotFile.empty());
            ui->nextStepPushButton->setEnabled(!errorCoeffVarPlotFile.empty() && !errorEstimationPlotFile.empty());
        }
        catch(...)
        {
            QApplication::restoreOverrideCursor();
            Messages::plotWentWrongCritical();
        }
    }

    QApplication::restoreOverrideCursor();
}



void MIMEMainWindow::on_loadErrorPushButton_clicked()
{
    try {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        ioTools::readErrorEstimates(ui->resultDirLineEdit->text().toStdString(), data);
        QApplication::restoreOverrideCursor();
        this->on_estimateErrorPushButton_clicked();
    }
    catch(...)
    {
        QApplication::restoreOverrideCursor();
        Messages::readingWentWrongCritical();
    }
}


void MIMEMainWindow::on_saveErrorPlotPushButton_clicked()
{
    try{
        bool ok;
        QString text = QInputDialog::getText(this, tr("Save error values and plot"), tr("Enter file name prefix for plot:"), QLineEdit::Normal, "", &ok);
        if(ok) {
            QApplication::setOverrideCursor(Qt::WaitCursor);
            string resultDir = ui->resultDirLineEdit->text().toStdString();
            string errorRateSamplePlotFile;
            string errorCoeffVarPlotFile;
            if(ui->epsErrorCheckBox->isChecked()) {
                errorRateSamplePlotFile = plot::plotMedianErrorPerSample(resultDir, parameter, data, plot::EPS, text.toStdString());
                errorCoeffVarPlotFile = plot::plotCoefficientOfVariation(resultDir, parameter, data, plot::EPS, text.toStdString());
            }
            if(ui->pdfErrorCheckBox->isChecked()) {
                errorRateSamplePlotFile += "\n"+plot::plotMedianErrorPerSample(resultDir, parameter, data, plot::PDF, text.toStdString());
                errorCoeffVarPlotFile += "\n" + plot::plotCoefficientOfVariation(resultDir, parameter, data, plot::PDF, text.toStdString());
            }
            QApplication::restoreOverrideCursor();
            if(!errorRateSamplePlotFile.empty() && !errorCoeffVarPlotFile.empty())
                Messages::filesAreSavedMessage(QString::fromStdString(errorRateSamplePlotFile)+"\n"+QString::fromStdString(errorCoeffVarPlotFile));

        }
    }
    catch(...)
    {
        QApplication::restoreOverrideCursor();
        Messages::plotWentWrongCritical();
    }
}



void MIMEMainWindow::on_saveErrorsPushButton_clicked()
{
    try {
        string resultDir = ui->resultDirLineEdit->text().toStdString();
        if(ioTools::dirExists(resultDir)) {
            QApplication::setOverrideCursor(Qt::WaitCursor);
            MIMEMainWindow::on_saveProjectPushButton_clicked();
            ioTools::writeErrorEstimates(resultDir, data);
            QApplication::restoreOverrideCursor();
            Messages::filesAreSavedMessage();
        }
    }
    catch(...)
    {
        QApplication::restoreOverrideCursor();
        Messages::savingWentWrongCritical();
    }
}

/************ KD button handles *********/

void MIMEMainWindow::enableParameter(bool isEnabled)
{
     ui->qualityCriteriaPushButton->setEnabled(isEnabled);
     ui->weightThrKDSpinBox->setEnabled(isEnabled);
     ui->alphaDoubleSpinBox->setEnabled(isEnabled);
     ui->minCoverageSpinBox->setEnabled(isEnabled);
     ui->minKDSpinBox->setEnabled(isEnabled);
     ui->s2nDoubleSpinBox->setEnabled(isEnabled);
     ui->KDyAxisFromDoubleSpinBox->setEnabled(isEnabled);
     ui->KDyAxisToDoubleSpinBox->setEnabled(isEnabled);
}

void MIMEMainWindow::enableKdSaveButtons(bool isEnabled){
    ui->saveKDResultsPushButton->setEnabled(isEnabled);
    ui->saveKDImagewithCriteriaPushButton->setEnabled(isEnabled);
    ui->epsKdCheckBox->setEnabled(isEnabled);
    ui->pdfKdCheckBox->setEnabled(isEnabled);
    ui->saveKDImageFromSpinBox->setEnabled(isEnabled);
    ui->saveKDImageToSpinBox->setEnabled(isEnabled);
    ui->plotPValuesCheckBox->setEnabled(isEnabled);
}

void MIMEMainWindow::on_computeRawKDValuesPushButton_clicked()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    MIMEMainWindow::enableParameter(false);
    processing::computeKDvalues(ui->dataDirLineEdit->text().toStdString(), parameter, data);
    QApplication::restoreOverrideCursor();

    MIMEMainWindow::enableParameter(true);
    ui->saveRawKDvaluesPushButton->setEnabled(true);

}

void MIMEMainWindow::on_qualityCriteriaPushButton_clicked()
{
    parameter.alpha = ui->alphaDoubleSpinBox->value();
    parameter.minSignal2NoiseStrength = ui->s2nDoubleSpinBox->value();
    parameter.minimumNrCalls = ui->minCoverageSpinBox->value();
    parameter.minNumberEstimatableKDs = ui->minKDSpinBox->value();
    //TODO: wenn Wert von weight threshold bei error estimation abweicht: Warnung ausgeben
    parameter.weightThreshold = ui->weightThrKDSpinBox->value()/100;

    parameter.plotYAxisFrom = ui->KDyAxisFromDoubleSpinBox->value();
    parameter.plotYAxisTo = ui->KDyAxisToDoubleSpinBox->value();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    processing::applyQualityCriteria(parameter, data);

    string allEffectsPlotFile;
    if(ui->KDyAxisFromDoubleSpinBox->value() < ui->KDyAxisToDoubleSpinBox->value())
    {
        try {
            allEffectsPlotFile = plot::plotAllEffects(ui->resultDirLineEdit->text().toStdString(), parameter, data, plot::SVG);
        }
        catch(...)
        {
            QApplication::restoreOverrideCursor();
            Messages::plotWentWrongCritical();
        }
    } else {
       QApplication::restoreOverrideCursor();
       Messages::invalidIntervalCritical();
    }
    if(!allEffectsPlotFile.empty()) {
        QGraphicsSvgItem *item = new QGraphicsSvgItem(QString::fromStdString(allEffectsPlotFile));
        QGraphicsScene* scene = new QGraphicsScene();
        scene->addItem(item);
        clearGraphicsView(ui->KDGraphicsView);
        ui->KDGraphicsView->setScene(scene);
        ui->KDGraphicsView->fitInView(item, Qt::KeepAspectRatioByExpanding);
        enableKdSaveButtons(true);
    }else {
        QApplication::restoreOverrideCursor();
        Messages::plotWentWrongCritical();
    }
    QApplication::restoreOverrideCursor();


}

void MIMEMainWindow::on_loadRawKDValuesPushButton_clicked()
{
    try {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        MIMEMainWindow::enableParameter(false);
        ioTools::readRawKDCriteria(ui->resultDirLineEdit->text().toStdString(), data, "rawKdValues.csv");
        QApplication::restoreOverrideCursor();
        MIMEMainWindow::enableParameter(true);
        ui->saveRawKDvaluesPushButton->setEnabled(true);
    }
    catch(MIME_NoSuchFileException& e) {
        QApplication::restoreOverrideCursor();
        Messages::fileDoesNotExistCritical(QString::fromStdString(e.getFilename()));
    }

}


void MIMEMainWindow::on_saveRawKDvaluesPushButton_clicked()
{
    try
    {
        string resultDir = ui->resultDirLineEdit->text().toStdString();
        if(ioTools::dirExists(resultDir)) {
            QApplication::setOverrideCursor(Qt::WaitCursor);
            MIMEMainWindow::on_saveProjectPushButton_clicked();
            ioTools::writeRawKDCriteria(ui->resultDirLineEdit->text().toStdString(), data, "rawKdValues.csv");
            QApplication::restoreOverrideCursor();
            Messages::filesAreSavedMessage();

        }
    }
    catch(...)
    {
        QApplication::restoreOverrideCursor();
        Messages::savingWentWrongCritical();
    }
}


void MIMEMainWindow::on_saveKDResultsPushButton_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Save Kd results"), tr("Enter file name suffix for the result table:"), QLineEdit::Normal, "", &ok);
    if(ok) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        std::string resultDir = ui->resultDirLineEdit->text().toStdString();
        MIMEMainWindow::on_saveProjectPushButton_clicked();
        std::string filename;
        try
        {
            filename = ioTools::writePositionWiseKDEstimates(resultDir, data, text.toStdString());
            if(!filename.empty())
                Messages::filesAreSavedMessage(QString::fromStdString(filename));
        }
        catch(...)
        {
            QApplication::restoreOverrideCursor();
            Messages::savingWentWrongCritical();
        }

        QApplication::restoreOverrideCursor();
    }
}

void MIMEMainWindow::on_saveKDImagewithCriteriaPushButton_clicked()
{
    if(ui->saveKDImageFromSpinBox->value() <= ui->saveKDImageToSpinBox->value() && ui->KDyAxisFromDoubleSpinBox->value() < ui->KDyAxisToDoubleSpinBox->value()) {
        parameter.plotStartRegion = ui->saveKDImageFromSpinBox->value();
        parameter.plotEndRegion = ui->saveKDImageToSpinBox->value();
        parameter.plotPValues = ui->plotPValuesCheckBox->isChecked();
        parameter.plotYAxisFrom = ui->KDyAxisFromDoubleSpinBox->value();
        parameter.plotYAxisTo = ui->KDyAxisToDoubleSpinBox->value();

        bool ok;
        QString text = QInputDialog::getText(this, tr("Save Kd plots"), tr("Enter file name suffix for the plot files:"), QLineEdit::Normal,"", &ok);
        if(ok) {
            QApplication::setOverrideCursor(Qt::WaitCursor);
            try{
                if(ui->epsKdCheckBox->isChecked())
                    plot::plotAllEffects(ui->resultDirLineEdit->text().toStdString(), parameter, data, plot::EPS, text.toStdString());
                if(ui->pdfKdCheckBox->isChecked())
                    plot::plotAllEffects(ui->resultDirLineEdit->text().toStdString(), parameter, data, plot::PDF, text.toStdString());
            }
            catch(...)
            {
                QApplication::restoreOverrideCursor();
                Messages::plotWentWrongCritical();
            }

            QApplication::restoreOverrideCursor();
            Messages::filesAreSavedMessage();
        }

    } else {
        Messages::invalidIntervalCritical();
    }
}
