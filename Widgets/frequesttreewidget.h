/*
 *
Copyright (C) 2017-2019  Fábio Bento (fabiobento512)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#ifndef FREQUESTTREEWIDGET_H
#define FREQUESTTREEWIDGET_H

#include <CustomTreeWidget/customtreewidget.h>

class FRequestTreeWidget : public CustomTreeWidget
{
	Q_OBJECT
		
public:
    FRequestTreeWidget(QWidget* parent);
private slots:
	void keyPressEvent(QKeyEvent *event);
signals:
    void deleteKeyPressed();
};

#endif // FREQUESTTREEWIDGET_H
