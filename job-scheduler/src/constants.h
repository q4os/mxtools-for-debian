/*
   Copyright (C) 2005 korewaisai
   korewaisai@yahoo.co.jp

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
*/

#pragma once

namespace JobScheduler {

// Time constants
constexpr int MINUTES_PER_HOUR = 60;
constexpr int HOURS_PER_DAY = 24;
constexpr int DAYS_PER_MONTH = 31;
constexpr int DAYS_PER_WEEK = 7;
constexpr int MONTHS_PER_YEAR = 12;

// UI constants
constexpr int MONTH_BUTTON_COLUMN_WIDTH = 120;
constexpr int ABOUT_DIALOG_WIDTH = 600;
constexpr int ABOUT_DIALOG_HEIGHT = 500;
constexpr int EXECUTE_LIST_MIN = 1;
constexpr int EXECUTE_LIST_MAX = 999;
constexpr int EXECUTE_LIST_STEP = 10;
constexpr int SAVE_DIALOG_WIDTH = 410;
constexpr int SAVE_DIALOG_HEIGHT = 500;
constexpr int MAINWINDOW_DEFAULT_WIDTH = 670;
constexpr int MAINWINDOW_DEFAULT_HEIGHT = 480;
constexpr int MAINWINDOW_DEFAULT_VIEW_WIDTH = 200;
constexpr int MAINWINDOW_DEFAULT_VIEW_HEIGHT = 460;
constexpr int MAINWINDOW_DEFAULT_MAX_LIST_NUM = 100;
constexpr int MAINWINDOW_DEFAULT_MAX_LIST_DATE = 1;
constexpr int COMMAND_TIME_BUTTON_MIN_WIDTH = 150;
constexpr int TIME_BUTTON_SIZE = 27;

} // namespace JobScheduler
