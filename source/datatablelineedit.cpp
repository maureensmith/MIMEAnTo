#include "datatablelineedit.hpp"
#include <iostream>
#include <QFileDialog>

DataTableLineEdit::DataTableLineEdit(utils::SampleType type, QWidget *parent) :
    QLineEdit(parent)
{
    this->type = type;
    if(type == utils::BOUND)
        this->setPlaceholderText(tr("(type barcode for selected sample)"));
        //this->setPlaceholderText(tr("(choose file for selected sample)"));
    else if(this->type == utils::UNBOUND)
        this->setPlaceholderText(tr("(type barcode for non-selected sample)"));
        //this->setPlaceholderText(tr("(choose file for non-selected sample)"));
}

void DataTableLineEdit::mouseDoubleClickEvent(QMouseEvent *event) {
    Q_UNUSED(event);
    this->on_samplePushButton_clicked();
}

utils::SampleType DataTableLineEdit::getType() {
    return this->type;
}

void DataTableLineEdit::on_samplePushButton_clicked() {
    QString sample;
    if(this->type == utils::BOUND)
        sample = "selected";
    else if(this->type == utils::UNBOUND)
        sample = "unselected";
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open %1 sample file").arg(sample), QDir::homePath());

    this->setText(fileName);
}


