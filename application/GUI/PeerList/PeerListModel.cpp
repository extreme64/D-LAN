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
  
#include <PeerList/PeerListModel.h>
using namespace GUI;

#include <QtAlgorithms>
#include <QStringBuilder>
#include <QSet>

#include <Common/ProtoHelper.h>
#include <Common/Global.h>

PeerListModel::PeerListModel(QSharedPointer<RCC::ICoreConnection> coreConnection) :
   coreConnection(coreConnection),
   currentSortType(Protos::GUI::Settings::BY_SHARING_AMOUNT)
{
   connect(this->coreConnection.data(), SIGNAL(newState(Protos::GUI::State)), this, SLOT(newState(Protos::GUI::State)));
}

PeerListModel::~PeerListModel()
{
   foreach (Peer* p, this->peers)
      delete p;
}

/**
  * Return 'defaultNick' if the peer isn't found.
  */
QString PeerListModel::getNick(const Common::Hash& peerID, const QString& defaultNick) const
{
   Peer* peer = this->indexedPeers.value(peerID, 0);
   if (!peer)
      return defaultNick;
   return peer->nick;
}

bool PeerListModel::isOurself(int rowNum) const
{
   if (rowNum >= this->peers.size())
      return false;
   return this->peers[rowNum]->peerID == this->coreConnection->getRemoteID();
}

Common::Hash PeerListModel::getPeerID(int rowNum) const
{
   if (rowNum >= this->peers.size())
      return Common::Hash();
   return this->peers[rowNum]->peerID;
}

QHostAddress PeerListModel::getPeerIP(int rowNum) const
{
   if (rowNum >= this->peers.size())
      return QHostAddress();
   return this->peers[rowNum]->ip;
}

void PeerListModel::clear()
{
   google::protobuf::RepeatedPtrField<Protos::GUI::State_Peer> peers;
   this->updatePeers(peers, QSet<Common::Hash>());
}

void PeerListModel::setSortType(Protos::GUI::Settings::PeerSortType sortType)
{
   if (this->currentSortType == sortType)
      return;

   this->currentSortType = sortType;
   this->sort();
}

Protos::GUI::Settings::PeerSortType PeerListModel::getSortType() const
{
   return this->currentSortType;
}

int PeerListModel::rowCount(const QModelIndex& parent) const
{
   return this->peers.size();
}

int PeerListModel::columnCount(const QModelIndex& parent) const
{
   return 3;
}

QVariant PeerListModel::data(const QModelIndex& index, int role) const
{
   if (!index.isValid() || index.row() >= this->peers.size())
      return QVariant();

   switch (role)
   {
   case Qt::DisplayRole:
      switch (index.column())
      {
      case 0: return QVariant::fromValue(this->peers[index.row()]->transfertInformation);
      case 1: return this->peers[index.row()]->nick;
      case 2: return Common::Global::formatByteSize(this->peers[index.row()]->sharingAmount);
      default: return QVariant();
      }

   case Qt::BackgroundRole:
      if (this->peersToColorize.contains(this->peers[index.row()]->peerID))
         return this->peersToColorize[this->peers[index.row()]->peerID];
      if (this->isOurself(index.row()))
         return QColor(192, 255, 192);
      return QVariant();

   case Qt::ForegroundRole:
      if (this->peersToColorize.contains(this->peers[index.row()]->peerID))
         return QColor(255, 255, 255);
      if (this->isOurself(index.row()))
         return QColor(0, 0, 0);
      return QVariant();

   case Qt::TextAlignmentRole:
      return index.column() == 2 ? Qt::AlignRight : Qt::AlignLeft;

   case Qt::ToolTipRole:
      {
         const QString coreVersion = this->peers[index.row()]->coreVersion;
         QString toolTip;
         if (!coreVersion.isEmpty())
            toolTip += tr("Version %1\n").arg(coreVersion);
         toolTip +=
            tr("Download rate: ") % Common::Global::formatByteSize(this->peers[index.row()]->transfertInformation.downloadRate) % "/s\n" %
            tr("Upload rate: ") % Common::Global::formatByteSize(this->peers[index.row()]->transfertInformation.uploadRate) % "/s";
         return toolTip;
      }

   default:
      return QVariant();
   }
}

void PeerListModel::colorize(const Common::Hash& peerID, const QColor& color)
{
   if (Peer* peer = this->indexedPeers.value(peerID, 0))
   {
      emit layoutAboutToBeChanged();
      this->peersToColorize[peer->peerID] = color;
      emit layoutChanged();
   }
   else
      this->peersToColorize.insert(peerID, color);
}

void PeerListModel::colorize(const QModelIndex& index, const QColor& color)
{
   if (!index.isValid() || index.row() >= this->peers.size())
      return;

   this->peersToColorize[this->peers[index.row()]->peerID] = color;

   emit dataChanged(this->createIndex(index.row(), 0), this->createIndex(index.row(), this->columnCount() - 1));
}

void PeerListModel::uncolorize(const QModelIndex& index)
{
   if (!index.isValid() || index.row() >= this->peers.size())
      return;

   if (this->peersToColorize.remove(this->peers[index.row()]->peerID))
      emit dataChanged(this->createIndex(index.row(), 0), this->createIndex(index.row(), this->columnCount() - 1));
}

void PeerListModel::newState(const Protos::GUI::State& state)
{
   QSet<Common::Hash> peersDownloadingOurData;
   for (int i = 0; i < state.upload_size(); i++)
      peersDownloadingOurData << Common::Hash(state.upload(i).peer_id().hash());

   this->updatePeers(state.peer(), peersDownloadingOurData);
}

void PeerListModel::updatePeers(const google::protobuf::RepeatedPtrField<Protos::GUI::State::Peer>& peers, const QSet<Common::Hash>& peersDownloadingOurData)
{
   bool sortNeeded = false;

   QList<Peer*> peersToRemove = this->peers;
   QList<Peer*> peersToAdd;

   for (int i = 0; i < peers.size(); i++)
   {
      const Common::Hash peerID(peers.Get(i).peer_id().hash());
      const QString& nick = ProtoHelper::getStr(peers.Get(i), &Protos::GUI::State::Peer::nick);
      const QString& coreVersion =ProtoHelper::getStr(peers.Get(i), &Protos::GUI::State::Peer::core_version);
      const quint64 sharingAmount(peers.Get(i).sharing_amount());
      TransfertInformation transfertInformation { peers.Get(i).download_rate(), peers.Get(i).upload_rate(),  peersDownloadingOurData.contains(peerID) };

      const QHostAddress ip =
         peers.Get(i).has_ip() ?
            Common::ProtoHelper::getIP(peers.Get(i).ip()) :
            QHostAddress();

      int j = this->peers.indexOf(this->indexedPeers[peerID]);
      if (j != -1)
      {
         peersToRemove.removeOne(this->indexedPeers[peerID]);
         if (this->peers[j]->nick != nick)
         {
            this->peers[j]->nick = nick;
            sortNeeded = true;
            emit dataChanged(this->createIndex(j, 1), this->createIndex(j, 1));
         }
         if (this->peers[j]->sharingAmount != sharingAmount)
         {
            this->peers[j]->sharingAmount = sharingAmount;
            sortNeeded = true;
         }
         if (this->peers[j]->transfertInformation != transfertInformation)
         {
            this->peers[j]->transfertInformation = transfertInformation;
            emit dataChanged(this->createIndex(j, 0), this->createIndex(j, 0));
         }

         this->peers[j]->ip = ip;
         this->peers[j]->coreVersion = coreVersion;
      }
      else
      {
         peersToAdd << new Peer(peerID, nick, coreVersion, sharingAmount, ip, transfertInformation);
      }
   }

   if(!peersToAdd.isEmpty())
      sortNeeded = true;

   QList<Common::Hash> peerIDsRemoved;
   for (QListIterator<Peer*> i(peersToRemove); i.hasNext();)
   {
      Peer* const peer = i.next();
      peerIDsRemoved << peer->peerID;
      int j = this->peers.indexOf(peer);
      if (j != -1)
      {
         this->beginRemoveRows(QModelIndex(), j, j);
         this->indexedPeers.remove(peer->peerID);
         this->peers.removeAt(j);
         delete peer;
         this->endRemoveRows();
      }
   }

   if (!peerIDsRemoved.isEmpty())
      emit peersRemoved(peerIDsRemoved);

   if (!peersToAdd.isEmpty())
   {
      this->beginInsertRows(QModelIndex(), this->peers.size(), this->peers.size() + peersToAdd.size() - 1);
      for (QListIterator<Peer*> i(peersToAdd); i.hasNext();)
      {
         Peer* const peer = i.next();
         this->indexedPeers.insert(peer->peerID, peer);
      }
      this->peers.append(peersToAdd);
      this->endInsertRows();
   }

   if (sortNeeded)
      this->sort();
}

void PeerListModel::sort()
{
   emit layoutAboutToBeChanged();
   qSort(this->peers.begin(), this->peers.end(), this->currentSortType == Protos::GUI::Settings::BY_NICK ? Peer::sortCompByNick : Peer::sortCompBySharingAmount);
   emit layoutChanged();
}

//////

bool PeerListModel::Peer::sortCompByNick(const Peer* p1, const Peer* p2)
{
   return Common::Global::toLowerAndRemoveAccents(p1->nick) < Common::Global::toLowerAndRemoveAccents(p2->nick);
}

bool PeerListModel::Peer::sortCompBySharingAmount(const PeerListModel::Peer* p1, const PeerListModel::Peer* p2)
{
   if (p1->sharingAmount == p2->sharingAmount)
      return Common::Global::toLowerAndRemoveAccents(p1->nick) < Common::Global::toLowerAndRemoveAccents(p2->nick);
   return p1->sharingAmount > p2->sharingAmount;
}
