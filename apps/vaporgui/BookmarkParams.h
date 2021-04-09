//************************************************************************
//									*
//		     Copyright (C)  2016				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		BookmarkParams
//
//	Author:		Stas Jaroszynski
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado

#pragma once

#include <vapor/ParamsBase.h>

class MouseModeParams;

class BookmarkParams : public VAPoR::ParamsBase {
public:
    BookmarkParams(VAPoR::ParamsBase::StateSave *ssave);
    BookmarkParams(VAPoR::ParamsBase::StateSave *ssave, VAPoR::XmlNode *node);
    BookmarkParams(const BookmarkParams &rhs);

    static string GetClassType() { return ("BookmarkParams"); }

    static const string NameTag;
    static const string DataTag;
    static const string IconDataTag;
    static const string IconSizeTag;
    
    void SetName(const string &name) { SetValueString(NameTag, "", name); }
    string GetName() const { return GetValueString(NameTag, ""); }
    
    void SetData(const string &data) { SetValueString(DataTag, "", data); }
    string GetData() const { return GetValueString(DataTag, ""); }
    
    string GetIconData() const { return GetValueString(IconDataTag, ""); }
    int GetIconSize() const { return GetValueLong(IconSizeTag, 0); }
    
    size_t GetIconDataSize() const
    {
        size_t s = GetIconSize();
        return s*s*3;
    }
    
    void SetIcon(int size, const string &data)
    {
        SetIconData(data);
        SetIconSize(size);
    }
    
private:

    void SetIconData(const string &data) { SetValueString(IconDataTag, "", data); }
    void SetIconSize(int size) { SetValueLong(IconSizeTag, "", size); }
    void _init();
};
