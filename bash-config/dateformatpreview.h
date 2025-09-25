#pragma once

#include <QLabel>
#include "global.h"

class DateFormatPreview : public QLabel
{
    Q_OBJECT
public:
    explicit DateFormatPreview(QWidget *parent = nullptr);

    QString dateFormat() const;
signals:

public slots:
    void setDateFormat(const QString& dateString);
private slots:
    void updateDisplayText();
private:
    QString m_dateFormat;
};
