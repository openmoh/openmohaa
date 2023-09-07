/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

This file is part of OpenMoHAA source code.

OpenMoHAA source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMoHAA source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMoHAA source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// con_arrayset.h: con_set with an index table

#pragma once

#include "mem_blockalloc.h"

template<typename key, typename value>
class con_arrayset_enum;

template<typename k, typename v>
class con_arrayset
{
    friend class con_arrayset_enum<k, v>;

public:
    class Entry
    {
    public:
        k            key;
        v            value;
        unsigned int index;

        Entry *next;

    public:
        void *operator new(size_t size);
        void  operator delete(void *ptr);

        Entry();

#ifdef ARCHIVE_SUPPORTED
        void Archive(Archiver& arc);
#endif
    };

public:
    static MEM_BlockAlloc<Entry> Entry_allocator;

private:
    Entry            **table; // hashtable
    unsigned int       tableLength;
    unsigned int       threshold;
    unsigned int       count; // num of entries
    short unsigned int tableLengthIndex;
    Entry             *defaultEntry;
    Entry            **reverseTable; // the index table

protected:
    Entry *findKeyEntry(const k& key) const;
    Entry *addKeyEntry(const k& key);
    Entry *addNewKeyEntry(const k& key);

public:
    con_arrayset();
    ~con_arrayset();

#ifdef ARCHIVE_SUPPORTED
    void Archive(Archiver& arc);
#endif

    void         clear();
    void         resize(int count = 0);
    unsigned int size() const;

    unsigned int findKeyIndex(const k& key);
    unsigned int addKeyIndex(const k& key);
    unsigned int addNewKeyIndex(const k& key);
    bool         remove(const k        &key);

    v& operator[](unsigned int index);
};

template<typename k, typename v>
MEM_BlockAlloc<typename con_arrayset<k, v>::Entry> con_arrayset<k, v>::Entry_allocator;

template<typename k, typename v>
void *con_arrayset<k, v>::Entry::operator new(size_t size)
{
    return Entry_allocator.Alloc();
}

template<typename k, typename v>
void con_arrayset<k, v>::Entry::operator delete(void *ptr)
{
    Entry_allocator.Free(ptr);
}

template<typename k, typename v>
con_arrayset<k, v>::Entry::Entry()
{
    this->key   = k();
    this->value = v();

    index = 0;
    next  = NULL;
}

template<typename key, typename value>
con_arrayset<key, value>::con_arrayset()
{
    tableLength = 1;
    table       = &defaultEntry;

    threshold        = 1;
    count            = 0;
    tableLengthIndex = 0;

    defaultEntry = NULL;
    reverseTable = (&this->defaultEntry) - 1;
}

template<typename key, typename value>
con_arrayset<key, value>::~con_arrayset()
{
    clear();
}

template<typename key, typename value>
void con_arrayset<key, value>::resize(int count)
{
    Entry      **oldReverseTable = reverseTable;
    Entry      **oldTable        = table;
    Entry       *e, *old;
    unsigned int index;
    unsigned int oldTableLength = tableLength;
    unsigned int i;

    if (count > 0) {
        tableLength += count;
        threshold = tableLength;
    } else {
        //threshold = ( unsigned int )( ( float )tableLength * 0.75f );
        threshold = (unsigned int)((float)tableLength * 0.75);
        if (threshold < 1) {
            threshold = 1;
        }

        tableLength += threshold;
    }

    // allocate a new table
    table = new Entry *[tableLength]();
    memset(table, 0, tableLength * sizeof(Entry *));

    // rehash the table
    for (i = oldTableLength; i > 0; i--) {
        // rehash all entries from the old table
        for (e = oldTable[i - 1]; e != NULL; e = old) {
            old = e->next;

            // insert the old entry to the table hashindex
            index = HashCode<key>(e->key) % tableLength;

            e->next      = table[index];
            table[index] = e;
        }
    }

    if (oldTableLength > 1) {
        // delete the previous table
        delete[] oldTable;
    }

    // allocate a bigger reverse table
    reverseTable = (new Entry *[this->tableLength]()) - 1;

    for (i = 1; i <= oldTableLength; i++) {
        reverseTable[i] = oldReverseTable[i];
    }

    if (oldTableLength > 1) {
        ++oldReverseTable;
        delete[] oldReverseTable;
    }
}

template<typename k, typename v>
unsigned int con_arrayset<k, v>::size() const
{
    return count;
}

template<typename key, typename value>
void con_arrayset<key, value>::clear()
{
    Entry       *entry = NULL;
    Entry       *next  = NULL;
    unsigned int i;

    if (this->tableLength > 1) {
        reverseTable++;
        delete[] reverseTable;
        reverseTable = (&this->defaultEntry) - 1;
    }

    for (i = 0; i < tableLength; i++) {
        for (entry = table[i]; entry != NULL; entry = next) {
            next = entry->next;
            delete entry;
        }
    }

    if (tableLength > 1) {
        delete[] table;
    }

    tableLength = 1;
    table       = &defaultEntry;

    threshold        = 1;
    count            = 0;
    tableLengthIndex = 0;

    defaultEntry = NULL;
}

template<typename k, typename v>
typename con_arrayset<k, v>::Entry *con_arrayset<k, v>::findKeyEntry(const k& key) const
{
    Entry *entry;

    entry = table[HashCode<k>(key) % tableLength];

    for (; entry != NULL; entry = entry->next) {
        if (entry->key == key) {
            return entry;
        }
    }

    return NULL;
}

template<typename k, typename v>
typename con_arrayset<k, v>::Entry *con_arrayset<k, v>::addKeyEntry(const k& key)
{
    Entry *entry;

    entry = findKeyEntry(key);

    if (entry != NULL) {
        return entry;
    } else {
        return addNewKeyEntry(key);
    }
}

template<typename k, typename v>
typename con_arrayset<k, v>::Entry *con_arrayset<k, v>::addNewKeyEntry(const k& key)
{
    Entry *entry;
    int    index;

    if (count >= threshold) {
        resize();
    }

    index = HashCode<k>(key) % tableLength;

    count++;

    entry = new Entry;

    if (defaultEntry == NULL) {
        defaultEntry = entry;
        entry->next  = NULL;
    } else {
        entry->next = table[index];
    }

    entry->key   = key;
    entry->index = count;

    table[index]        = entry;
    reverseTable[count] = entry;

    return entry;
}

template<typename k, typename v>
unsigned int con_arrayset<k, v>::addKeyIndex(const k& key)
{
    Entry *entry = this->addKeyEntry(key);

    return entry->index;
}

template<typename k, typename v>
unsigned int con_arrayset<k, v>::addNewKeyIndex(const k& key)
{
    Entry *entry = this->addNewKeyEntry(key);

    return entry->index;
}

template<typename k, typename v>
unsigned int con_arrayset<k, v>::findKeyIndex(const k& key)
{
    Entry *entry = this->findKeyEntry(key);

    if (entry != NULL) {
        return entry->index;
    } else {
        return 0;
    }
}

template<typename k, typename v>
bool con_arrayset<k, v>::remove(const k& key)
{
    int i;

    for (i = 1; i <= this->tableLength; i++) {
        if (reverseTable[i] && reverseTable[i]->key == key) {
            reverseTable[i] = NULL;
        }
    }

    return con_set<k, v>::remove(key);
}

template<typename key, typename value>
value& con_arrayset<key, value>::operator[](unsigned int index)
{
    return reverseTable[index]->key;
}

template<typename key, typename value>
class con_arrayset_enum
{
    friend class con_map_enum<key, value>;

public:
    using Entry = typename con_arrayset<key, value>::Entry;

protected:
    con_arrayset<key, value> *m_Set;
    unsigned int              m_Index;
    Entry                    *m_CurrentEntry;
    Entry                    *m_NextEntry;

public:
    con_arrayset_enum();
    con_arrayset_enum(con_arrayset<key, value>& set);

    bool operator=(con_arrayset<key, value>& set);

    Entry *NextElement(void);
    Entry *CurrentElement(void);
};

template<typename key, typename value>
con_arrayset_enum<key, value>::con_arrayset_enum()
{
    m_Set          = NULL;
    m_Index        = 0;
    m_CurrentEntry = NULL;
    m_NextEntry    = NULL;
}

template<typename key, typename value>
con_arrayset_enum<key, value>::con_arrayset_enum(con_arrayset<key, value>& set)
{
    *this = set;
}

template<typename key, typename value>
bool con_arrayset_enum<key, value>::operator=(con_arrayset<key, value>& set)
{
    m_Set          = &set;
    m_Index        = m_Set->tableLength;
    m_CurrentEntry = NULL;
    m_NextEntry    = NULL;

    return true;
}

template<typename key, typename value>
typename con_arrayset_enum<key, value>::Entry *con_arrayset_enum<key, value>::CurrentElement(void)
{
    return m_CurrentEntry;
}

template<typename key, typename value>
typename con_arrayset_enum<key, value>::Entry *con_arrayset_enum<key, value>::NextElement(void)
{
    if (!m_NextEntry) {
        while (1) {
            if (!m_Index) {
                break;
            }

            m_Index--;
            m_NextEntry = m_Set->table[m_Index];

            if (m_NextEntry) {
                break;
            }
        }

        if (!m_NextEntry) {
            m_CurrentEntry = NULL;
            return NULL;
        }
    }

    m_CurrentEntry = m_NextEntry;
    m_NextEntry    = m_NextEntry->next;

    return m_CurrentEntry;
}
