#ifndef PEERMANAGER_GET_CHUNK_RESULT_H
#define PEERMANAGER_GET_CHUNK_RESULT_H

#include <QObject>
#include <QTimer>

#include <google/protobuf/message.h>

#include <Protos/common.pb.h>
#include <Protos/core_protocol.pb.h>

#include <IGetChunkResult.h>
#include <priv/Socket.h>

namespace PM
{
   class GetChunkResult : public IGetChunkResult
   {
      Q_OBJECT
   public:
      GetChunkResult(const Protos::Core::GetChunk& chunk, QSharedPointer<Socket> socket);
      ~GetChunkResult();
      void start();

   private slots:
      void newMessage(quint32 type, const google::protobuf::Message& message);
      void timeoutError();

   private:
      const Protos::Core::GetChunk chunk;
      QSharedPointer<Socket> socket;
      QTimer timerTimeout;
      bool error;
   };
}

#endif
