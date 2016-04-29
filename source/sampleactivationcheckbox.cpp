#include "sampleactivationcheckbox.h"

SampleActivationCheckBox::SampleActivationCheckBox(utils::DataContainer& data, QString name, QWidget *parent) :
    QCheckBox(name, parent)
{
    this->data = &data;
}

void SampleActivationCheckBox::on_sampleActivationCheckBox_toggled(bool active) {
    data->activateSamples(this->text().toStdString(), active);
}

