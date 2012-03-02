/**
  * D-LAN - A decentralized LAN file sharing software.
  * Copyright (C) 2010-2012 Greg Burri <greg.burri@gmail.com>
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  */
  
#ifndef GUI_WIDGETDOWNLOADS_H
#define GUI_WIDGETDOWNLOADS_H

#include <QWidget>
#include <QPoint>
#include <QStyledItemDelegate>

#include <Common/RemoteCoreController/ICoreConnection.h>

#include <CheckBoxList.h>
#include <CheckBoxModel.h>
#include <PeerList/PeerListModel.h>
#include <Downloads/DownloadFilterStatus.h>
#include <Downloads/DownloadsModel.h>

#include <Settings/DirListModel.h>

namespace Ui {
   class WidgetDownloads;
}

namespace GUI
{
   class DownloadsDelegate : public QStyledItemDelegate
   {
   public:
      void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
      QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
   };

   class WidgetDownloads : public QWidget
   {
      Q_OBJECT
   public:
      explicit WidgetDownloads(QSharedPointer<RCC::ICoreConnection> coreConnection, const PeerListModel& peerListModel, const DirListModel& sharedDirsModel, QWidget *parent = 0);
      ~WidgetDownloads();

   protected:
      void keyPressEvent(QKeyEvent* event);
      void changeEvent(QEvent* event);

   private slots:
      void displayContextMenuDownloads(const QPoint& point);
      void downloadDoubleClicked(const QModelIndex& index);
      void openLocationSelectedEntries();
      void removeCompletedFiles();
      void removeSelectedEntries();
      void filterChanged();

   private:
      void updateCheckBoxElements();

      Ui::WidgetDownloads *ui;
      CheckBoxList* filterStatusList;

      QSharedPointer<RCC::ICoreConnection> coreConnection;

      CheckBoxModel<DownloadFilterStatus> checkBoxModel;
      DownloadsModel downloadsModel;
      DownloadsDelegate downloadsDelegate;
   };
}

#endif
