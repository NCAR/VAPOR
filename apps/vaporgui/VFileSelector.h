#pragma once

#include <string>
#include <QWidget>
#include <QFileDialog>

#include "VContainer.h"

class VPushButton;
class VLineEdit;

class VFileSelector : public VContainer {
    Q_OBJECT

public:
    std::string GetValue() const;
    bool SetValue( const std::string& file );

     void HideLineEdit( bool hide );

protected:
    VFileSelector(
        const std::string& buttonText,
        const std::string& defaultPath,
        const std::string& filter
    );

    VLineEdit*           _lineEdit;
    VPushButton*         _pushButton;

    std::string           _filePath;
    std::string           _filter;

private:
    virtual std::string _launchFileDialog() = 0;
    virtual bool _isFileOperable( const std::string& file ) const = 0;

public slots:
    void OpenFileDialog();
    void SetPathFromLineEdit( const std::string& value );

signals:
    void ValueChanged( const std::string& path );
};


//
//
//
class VFileReader : public VFileSelector {
    Q_OBJECT

public:
    VFileReader(
        const std::string& buttonText = "Select",
        const std::string& defaultPath = QDir::homePath().toStdString(),
        const std::string& filter = ""
    );

private:
    virtual std::string _launchFileDialog();
    virtual bool _isFileOperable( const std::string& file ) const;
};

//
//
//
class VFileWriter : public VFileSelector {
    Q_OBJECT

public:
    VFileWriter(
        const std::string& buttonText = "Select",
        const std::string& defaultPath = QDir::homePath().toStdString(),
        const std::string& filter = ""
    );

private:
    virtual std::string _launchFileDialog();
    virtual bool _isFileOperable( const std::string& file ) const;
};

//
//
//
class VDirSelector : public VFileSelector {
    Q_OBJECT

public:
    VDirSelector (
        const std::string& buttonText = "Select",
        const std::string& defaultPath = QDir::homePath().toStdString()
    );

private:
    virtual std::string _launchFileDialog();
    virtual bool _isFileOperable( const std::string& file ) const;
};
