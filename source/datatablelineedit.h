#ifndef DATATABLELINEEDIT_H
#define DATATABLELINEEDIT_H

#include <QLineEdit>
#include "utils.hpp"

class DataTableLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit DataTableLineEdit(utils::SampleType, QWidget *parent = 0);

    void mouseDoubleClickEvent(QMouseEvent *event);

    utils::SampleType getType();

private:
    utils::SampleType type;

signals:

public slots:

    void on_samplePushButton_clicked();

};

#endif // DATATABLELINEEDIT_H
