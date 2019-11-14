#pragma once

#include <string>
#include <QWidget>
#include <QFileDialog>

#include "VContainer.h"

class VPushButton2;
class VLineEdit2;

class VFileSelector2 : public VContainer {
    Q_OBJECT

public:
    std::string GetValue() const;
    bool SetValue( const std::string& file );

protected:
    VFileSelector2(
        const std::string& buttonText,
        const std::string& defaultPath,
        const std::string& filter
    );

    VLineEdit2*           _lineEdit;
    VPushButton2*         _pushButton;

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
class VFileReader2 : public VFileSelector2 {
    Q_OBJECT

public:
    VFileReader2(
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
class VFileWriter2 : public VFileSelector2 {
    Q_OBJECT

public:
    VFileWriter2(
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
class VDirSelector : public VFileSelector2 {
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
