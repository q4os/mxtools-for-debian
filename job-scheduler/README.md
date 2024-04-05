# Job Scheduler
---------------

[![latest packaged version(s)](https://repology.org/badge/latest-versions/job-scheduler.svg)](https://repology.org/project/job-scheduler/versions)
[![build result](https://build.opensuse.org/projects/home:mx-packaging/packages/job-scheduler/badge.svg?type=default)](https://software.opensuse.org//download.html?project=home%3Amx-packaging&package=job-scheduler)

1) Documentation

 Job Scheduler is a scheduling utility which uses crontab as the backend
 Based upon qroneko 0.5.4, released in 2005 by korewaisai (korewaisai@yahoo.co.jp)
 More details : http://qroneko.sourceforge.net/

2) Compiling and Installation

 Set up Qt5 development environment.
 and type as follows :

 $ qmake -project -o job-schduler.pro
 $ qmake
 $ make

 You should be able to install .deb package from debs subfolder
