/*
 * This class provides a set of functions that check if a file/directory
 * is good to read/write.
 */

#include <QString>

class FileOperationChecker {
public:
    static bool DirectoryGoodToRead(const QString &filename);
    static bool FileGoodToRead(const QString &filename);
    static bool FileGoodToWrite(const QString &filename);
};
