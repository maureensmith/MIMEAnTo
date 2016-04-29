#ifndef SAMPLEACTIVATIONCHECKBOX_H
#define SAMPLEACTIVATIONCHECKBOX_H

#include <QCheckBox>
#include "utils.hpp"

class SampleActivationCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    explicit SampleActivationCheckBox(utils::DataContainer&, QString, QWidget *parent = 0);

private:
    utils::DataContainer* data;

signals:

public slots:
    void on_sampleActivationCheckBox_toggled(bool);
};

#endif // SAMPLEACTIVATIONCHECKBOX_H
