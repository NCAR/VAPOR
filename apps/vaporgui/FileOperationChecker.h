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
    static bool DirectoryGoodToRead(const std::string &filename) { return (DirectoryGoodToRead(QString(filename.c_str()))); }

    static bool FileGoodToRead(const QString &filename);
    static bool FileGoodToRead(const std::string &filename) { return (FileGoodToRead(QString(filename.c_str()))); }

    static bool FileGoodToWrite(const QString &filename);
    static bool FileGoodToWrite(const std::string &filename) { return (FileGoodToWrite(QString(filename.c_str()))); }

    static bool FileHasCorrectSuffix(const QString &filename, const QString &expectedSuffix);
    static bool FileHasCorrectSuffix(const std::string &filename, const std::string &expectedSuffix) { return (FileHasCorrectSuffix(QString(filename.c_str()), QString(expectedSuffix.c_str()))); }

    static QString GetLastErrorMessage();

private:
    static QString _message;
};

#endif
