#ifndef MIMEMAINWINDOW_H
#define MIMEMAINWINDOW_H

#include <QMainWindow>
//#include <QtGui/QWidget>
#include <QLineEdit>
#include <QGridLayout>
#include "datatablelineedit.h"
#include "utils.hpp"

namespace Ui {
class MIMEMainWindow;
}

class MIMEMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static const int RESTART_CODE;
    explicit MIMEMainWindow(QWidget *parent = 0);
    ~MIMEMainWindow();

protected:
     void closeEvent(QCloseEvent *event);

private:

    QLineEdit* createLineEdit(const QString& text);

    void enableParameter(bool isEnabled);

    void enableSanitySaveButtons(bool isEnabled);

    void enableErrorSaveButtons(bool isEnabled);

    void enableKdSaveButtons(bool isEnabled);

    void initErrorEstimationPage();

    void initKDComputationPage();

    int addNewRow();



private slots:
    void on_resultDirPushButton_clicked();

    void on_addRowPushButton_clicked();

    void onDeleteRowPushButton_clicked();

    void on_referencePushButton_clicked();

    void on_loadDataPushButton_clicked();

    void on_dataDirPushButton_clicked();

    void on_estimateErrorPushButton_clicked();

    void on_qualityCriteriaPushButton_clicked();

    void on_loadRawKDValuesPushButton_clicked();

    void on_loadErrorPushButton_clicked();

    void on_loadProjectPushButton_clicked();

    void on_saveKDImagewithCriteriaPushButton_clicked();

    void on_saveSanityPushButton_clicked();

     void on_computeRawKDValuesPushButton_clicked();

     void on_nextStepPushButton_clicked();

     void on_backStepPushButton_clicked();

     void on_saveProjectPushButton_clicked();

     void on_saveRawKDvaluesPushButton_clicked();

     void on_saveErrorPlotPushButton_clicked();

     void on_saveErrorsPushButton_clicked();

     void on_saveKDResultsPushButton_clicked();

     void on_actionNew_Project_triggered();


private:
    Ui::MIMEMainWindow *ui;
    QGridLayout *mainLayout;

    utils::DataContainer data;
    utils::Parameter parameter;

    QWidget* createDataCell();

    QWidget* createSelectionArea();

    bool joinErrorWarning();

    void handleStackedWidgetAndFooter(int index);

};

#endif // MIMEMAINWINDOW_H
