#ifndef FILEMANAGER_FILE_H
#define FILEMANAGER_FILE_H

#include <exception>

#include <QString>
#include <QMutex>
#include <QFile>
#include <QList>

#include <Protos/common.pb.h>
#include <IFile.h>
#include <priv/Cache/Entry.h>

namespace FileManager
{
   class IChunk;
   class Chunk;
   class Directory;

   class FileNotFoundException : public std::exception {};

   class File : public Entry, public IFile
   {
   public:
      static const int BUFFER_SIZE = 65536; // (64kB) Buffer used when reading a file.
      static const int CHUNK_SIZE = 33554432; // (32 MB).

      File(Directory* dir, const QString& name, qint64 size);
      virtual ~File() {};

      QString getPath();
      QString getFullPath();
      Directory* getRoot();

      void newDataWriterCreated();
      void newDataReaderCreated();

      void dataWriterDeleted();
      void dataReaderDeleted();

      /**
        * Write some bytes to the file at the given offset.
        * If the buffer exceed the file size then only the begining of the buffer is
        * used, the file is not resizing.
        * @param buffer The buffer.
        * @param offset An offset.
        * @return true if end of file reached.
        */
      bool write(const QByteArray& buffer, qint64 offset);

      /**
        * Fill the buffer with the read bytes from the given offset.
        * If the end of file is reached the buffer will be partialy filled.
        * @param buffer The buffer.
        * @param offset An offset.
        * @return the number of bytes read.
        */
      qint64 read(QByteArray& buffer, qint64 offset);

      /**
        * It will open the file, read it and calculate all theirs chunk hashes.
        * If it already owns some chunks, there are destroyed first.
        * This method can be called from an another thread than the main one. For example,
        * from 'FileUpdated' thread.
        */
      void computeHashes();

      QList<IChunk*> getChunks();
      const QList<Chunk*>& getChunksRef();

      void populateFileEntry(Protos::Common::FileEntry* entry);

   private:
      Directory* dir;
      QList<Chunk*> chunks;

      int numDataWriter;
      int numDataReader;
      QFile* fileInWriteMode;
      QFile* fileInReadMode;
      QMutex* writeLock;
      QMutex* readLock;
   };
}
#endif