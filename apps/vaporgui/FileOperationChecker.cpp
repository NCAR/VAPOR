#include <QFileInfo>
#include "FileOperationChecker.h"

QString FileOperationChecker::_message("");

bool FileOperationChecker::DirectoryGoodToRead(const QString &filename)
{
    QFileInfo fileInfo(filename);

    // Test if this filename exists
    if (!fileInfo.exists()) {
        QString msg(" The following input does not exist! \n");
        msg += filename;
        _message = msg;
        return false;
    }

    // Test if this is a directory
    if (!fileInfo.isDir()) {
        QString msg(" The following input is NOT a directory! \n");
        msg += filename;
        _message = msg;
        return false;
    }

    // Test if this directory is readable
    if (!fileInfo.isReadable()) {
        QString msg(" The following input is NOT readable! \n");
        msg += filename;
        _message = msg;
        return false;
    }

    // Test if this directory is executable
    if (!fileInfo.isExecutable()) {
        QString msg(" The following input is NOT executable! \n");
        msg += filename;
        _message = msg;
        return false;
    }

    return true;
}

bool FileOperationChecker::FileGoodToRead(const QString &filename)
{
    QFileInfo fileInfo(filename);

    // Test if this filename exists
    if (!fileInfo.exists()) {
        QString msg(" The following input does not exist! \n");
        msg += filename;
        _message = msg;
        return false;
    }

    // Test if this is a file
    if (!fileInfo.isFile()) {
        QString msg(" The following input is NOT a file! \n");
        msg += filename;
        _message = msg;
        return false;
    }

    // Test if this file is readable
    if (!fileInfo.isReadable()) {
        QString msg(" The following input is NOT readable! \n");
        msg += filename;
        _message = msg;
        return false;
    }

    return true;
}

bool FileOperationChecker::FileGoodToWrite(const QString &filename)
{
    QFileInfo fileInfo(filename);

    // In case this file does not exist
    if (!fileInfo.exists()) {
        std::FILE *f = std::fopen(filename.toAscii(), "w");
        if (f)    // able to write
        {
            std::fclose(f);
            std::remove(filename.toAscii());
            return true;
        } else {
            QString msg(" The following file cannot be created! \n");
            msg += filename;
            _message = msg;
            return false;
        }
    }

    // The input exists when the program reaches here

    // Test if this is a file
    if (!fileInfo.isFile()) {
        QString msg(" The following is NOT a file! \n");
        msg += filename;
        _message = msg;
        return false;
    }

    // Test if this file is readable
    if (!fileInfo.isReadable()) {
        QString msg(" The following file is NOT readable! \n");
        msg += filename;
        _message = msg;
        return false;
    }

    // Test if this file is writable
    if (!fileInfo.isWritable()) {
        QString msg(" The following file is NOT writable! \n");
        msg += filename;
        _message = msg;
        return false;
    }

    return true;
}

bool FileOperationChecker::FileHasCorrectSuffix(const QString &filename, const QString &expectedSuffix)
{
    QFileInfo fileInfo(filename);
    if (fileInfo.suffix().compare(expectedSuffix) == 0)
        return true;
    else {
        QString msg(" The following input does NOT have the expected suffix: ");
        msg += expectedSuffix;
        msg += "\n";
        msg += filename;
        _message = msg;

        return false;
    }
}

QString FileOperationChecker::GetLastErrorMessage() { return _message; }
