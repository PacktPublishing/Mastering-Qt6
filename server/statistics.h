#pragma once

#ifndef STATISTICS_H
#define STATISTICS_H

#include <QObject>

class Statistics : public QObject
{
Q_OBJECT
public:
  Statistics(QObject *parent=nullptr) : QObject(parent)
  {}
  ~Statistics() {}
};

#endif