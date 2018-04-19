/*
 * This class provides a set of functions that check if a file/directory
 * is good to read/write.
 *
 * The rational is recorded in this post:
 * https://github.com/NCAR/VAPOR/wiki/Robust-File-Operations-with-VAPOR-and-Qt
 *
 */

#ifndef FILEOPERATIONCHECKER_H
#define FILEOPERATIONCHECKER_H

#include <QString>

class FileOperationChecker {
public:
    static bool DirectoryGoodToRead(const QString &filename);
    static bool FileGoodToRead(const QString &filename);
    static bool FileGoodToWrite(const QString &filename);
};

#endif
