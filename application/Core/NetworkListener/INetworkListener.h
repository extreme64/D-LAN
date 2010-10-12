#ifndef NETWORKMANAGER_INETWORKLISTENER_H
#define NETWORKMANAGER_INETWORKLISTENER_H

#include <QSharedPointer>

namespace NL
{
   class IChat;
   class ISearch;

   class INetworkListener
   {
   public:
      virtual ~INetworkListener() {}

      virtual IChat& getChat() = 0;
      virtual QSharedPointer<ISearch> newSearch() = 0;
   };
}
#endif
