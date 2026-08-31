/**********************************************************************
 *  MX Date/Time about dialog.
 **********************************************************************
 *   Copyright (C) 2022-2026 by Adrian
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 * This file is part of mx-datetime.
 **********************************************************************/

#ifndef ABOUT_H
#define ABOUT_H

class QString;

void displayDoc(const QString &url, const QString &title, bool largeWindow = false);
void displayHelpDoc(const QString &path, const QString &title);
void displayAboutMsgBox(const QString &title, const QString &message, const QString &licence_url,
                        const QString &license_title);

#endif // ABOUT_H
