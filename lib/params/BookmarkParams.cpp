//************************************************************************
//									*
//		     Copyright (C)  2016				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		BookmarkParams.cpp
//
//	Author:		Stas Jaroszynski
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado

#include <vapor/BookmarkParams.h>

using namespace VAPoR;

const string BookmarkParams::NameTag = "NameTag";
const string BookmarkParams::DataTag = "DataTag";
const string BookmarkParams::IconDataTag = "IconDataTag";
const string BookmarkParams::IconSizeTag = "IconSizeTag";

static ParamsRegistrar<BookmarkParams> registrar(BookmarkParams::GetClassType());

void BookmarkParams::_init() {}

BookmarkParams::BookmarkParams(ParamsBase::StateSave *ssave) : ParamsBase(ssave, GetClassType()) { _init(); }

BookmarkParams::BookmarkParams(ParamsBase::StateSave *ssave, XmlNode *node) : ParamsBase(ssave, node)
{
    if (node->GetTag() != BookmarkParams::GetClassType()) {
        node->SetTag(BookmarkParams::GetClassType());
        _init();
    }
}

BookmarkParams::BookmarkParams(const BookmarkParams &rhs) : ParamsBase(rhs) {}
