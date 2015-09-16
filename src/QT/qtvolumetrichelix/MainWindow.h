// This file is part of CNCSVision, a computer vision related library
// This software is developed under the grant of Italian Institute of Technology
//
// Copyright (C) 2010-2014 Carlo Nicolini <carlo.nicolini@iit.it>
//
//
// CNCSVision is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// CNCSVision is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// CNCSVision. If not, see <http://www.gnu.org/licenses/>.

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QSlider>
#include <QWidget>
#include "ui_MainWindow.h"
#include <stdexcept>

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{  Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:
    void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *, QEvent *);

private slots:
    void onPushbuttonsetoutputdirectoryClicked();

    void onPushButtonStartFramesGenerationClicked();
    void onDoubleSpinboxObjectSizeChanged(double value);
    void onDoubleSpinboxRotationAngleChanged(double value);
    void onDoubleSpinboxOffsetXChanged(double value);
    void onDoubleSpinboxOffsetYChanged(double value);
    void onDoubleSpinboxOffsetZChanged(double value);
    void onSpinboxNSlicesChanged(int value);

    void onPushbuttonRandomizeSpheresPressed();

    void on_spinBoxSpheresRadiusMin_valueChanged(int arg1);

    void on_spinBoxSpheresRadiusMax_valueChanged(int arg1);

    void on_checkBoxCameraViewMode_clicked(bool checked);

    void on_pushButtonLoad2Dpoints_clicked();

    void on_pushButtonLoad3Dpoints_clicked();

    void on_checkBoxUseCalibratedView_clicked(bool checked);

private:
    Ui::MainWindow *ui;
};

#endif