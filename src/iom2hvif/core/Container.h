/*
 * Copyright 2006-2025, Haiku.
 * Distributed under the terms of the MIT License.
 *
 * Based on Haiku Icon-O-Matic implementation by:
 *   Stephan Aßmus <superstippi@gmx.de>
 *   Zardshard
 *
 * Adapted as standalone converter without GUI dependencies.
 */

#ifndef CONTAINER_H
#define CONTAINER_H

#include <List.h>
#include "IconBuild.h"

_BEGIN_ICON_NAMESPACE

template<class Type>
class Container {
public:
    Container(bool ownsItems)
        : fItems(16)
        , fOwnsItems(ownsItems)
    {
    }

    virtual ~Container()
    {
        MakeEmpty();
    }

    bool AddItem(Type* item)
    {
        return AddItem(item, CountItems());
    }

    bool AddItem(Type* item, int32 index)
    {
        if (!item)
            return false;
        if (HasItem(item))
            return false;
        return fItems.AddItem((void*)item, index);
    }

    bool RemoveItem(Type* item)
    {
        return fItems.RemoveItem((void*)item);
    }

    Type* RemoveItem(int32 index)
    {
        return (Type*)fItems.RemoveItem(index);
    }

    void MakeEmpty()
    {
        int32 count = CountItems();
        if (fOwnsItems) {
            for (int32 i = 0; i < count; i++) {
                delete ItemAtFast(i);
            }
        }
        fItems.MakeEmpty();
    }

    int32 CountItems() const
    {
        return fItems.CountItems();
    }

    bool HasItem(Type* item) const
    {
        return fItems.HasItem((void*)item);
    }

    int32 IndexOf(Type* item) const
    {
        return fItems.IndexOf((void*)item);
    }

    Type* ItemAt(int32 index) const
    {
        return (Type*)fItems.ItemAt(index);
    }

    Type* ItemAtFast(int32 index) const
    {
        return (Type*)fItems.ItemAtFast(index);
    }

private:
    BList fItems;
    bool fOwnsItems;
};

_END_ICON_NAMESPACE

#endif
