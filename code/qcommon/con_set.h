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

// con_set.h: C++ map/set classes. Contains an hash table to improve the speed of finding a key

#pragma once

#include "mem_blockalloc.h"

class Class;
class Archiver;

template<typename key, typename value>
class con_map;

template<typename key, typename value>
class con_map_enum;

template<typename key, typename value>
class con_set_enum;

template<typename T>
struct con_set_is_pointer {
    static const bool con_value = false;
};

template<typename T>
struct con_set_is_pointer<T *> {
    static const bool con_value = true;
};

template<typename k, typename v>
class con_set
{
    friend class con_set_enum<k, v>;

public:
    class Entry
    {
        friend con_set<k, v>;
        friend con_set_enum<k, v>;

    private:
        Entry* next;
        k key;

    public:
        v value;

    public:
        void *operator new(size_t size);
        void  operator delete(void *ptr);

        Entry();

#ifdef ARCHIVE_SUPPORTED
        void Archive(Archiver& arc);
#endif
        k& GetKey();
        void SetKey(const k& newKey);
    };

public:
    static MEM_BlockAlloc<Entry> Entry_allocator;

protected:
    Entry            **table; // hashtable
    unsigned int       tableLength;
    unsigned int       threshold;
    unsigned int       count; // num of entries
    short unsigned int tableLengthIndex;
    Entry             *defaultEntry;

protected:
    Entry *findKeyEntry(const k& key) const;
    Entry *addKeyEntry(const k& key);
    Entry *addNewKeyEntry(const k& key);

public:
    con_set();
    ~con_set();

#ifdef ARCHIVE_SUPPORTED
    void Archive(Archiver& arc);
#endif

    void clear();
    void resize(int count = 0);

    v *findKeyValue(const k& key) const;
    k *firstKeyValue();

    v& addKeyValue(const k& key);
    v& addNewKeyValue(const k& key);

    bool keyExists(const k& key);
    bool isEmpty();
    bool remove(const k& key);

    unsigned int size();
};

template<typename k, typename v>
MEM_BlockAlloc<typename con_set<k, v>::Entry> con_set<k, v>::Entry_allocator;

template<typename k>
int HashCode(const k& key);

template<typename k, typename v>
void *con_set<k, v>::Entry::operator new(size_t size)
{
    return Entry_allocator.Alloc();
}

template<typename k, typename v>
void con_set<k, v>::Entry::operator delete(void *ptr)
{
    Entry_allocator.Free(ptr);
}

template<typename k, typename v>
con_set<k, v>::Entry::Entry()
{
    this->key   = k();
    this->value = v();

    next = NULL;
}

template<typename k, typename v>
void con_set<k, v>::Entry::SetKey(const k& newKey)
{
    key = newKey;
}

template<typename k, typename v>
k& con_set<k, v>::Entry::GetKey()
{
    return key;
}

template<typename key, typename value>
con_set<key, value>::con_set()
{
    tableLength = 1;
    table       = &defaultEntry;

    threshold        = 1;
    count            = 0;
    tableLengthIndex = 0;

    defaultEntry = NULL;
}

template<typename key, typename value>
con_set<key, value>::~con_set()
{
    clear();
}

template<typename key, typename value>
void con_set<key, value>::clear()
{
    Entry       *entry = NULL;
    Entry       *next  = NULL;
    unsigned int i;

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

template<typename key, typename value>
void con_set<key, value>::resize(int count)
{
    Entry      **oldTable = table;
    Entry       *e, *old;
    unsigned int oldTableLength = tableLength;
    unsigned int i;
    unsigned int index;

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
            index = HashCode<key>(e->GetKey()) % tableLength;

            e->next      = table[index];
            table[index] = e;
        }
    }

    if (oldTableLength > 1) {
        // delete the previous table
        delete[] oldTable;
    }
}

template<typename k, typename v>
typename con_set<k, v>::Entry *con_set<k, v>::findKeyEntry(const k& key) const
{
    Entry *entry;

    entry = table[HashCode<k>(key) % tableLength];

    for (; entry != NULL; entry = entry->next) {
        if (entry->GetKey() == key) {
            return entry;
        }
    }

    return NULL;
}

template<typename k, typename v>
typename con_set<k, v>::Entry *con_set<k, v>::addKeyEntry(const k& key)
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
typename con_set<k, v>::Entry *con_set<k, v>::addNewKeyEntry(const k& key)
{
    Entry *entry;
    int    hash;

    if (count >= threshold) {
        resize();
    }

    count++;

    entry = new Entry;
    entry->SetKey(key);
    hash = HashCode<k>(entry->GetKey()) % tableLength;
   
    if (defaultEntry == NULL) {
        defaultEntry = entry;
        entry->next  = NULL;
    } else {
        entry->next = table[hash];
    }
    table[hash] = entry;

    return entry;
}

template<typename key, typename value>
bool con_set<key, value>::isEmpty(void)
{
    return count == 0;
}

template<typename k, typename v>
bool con_set<k, v>::remove(const k& key)
{
    int    hash;
    int    i;
    Entry *prev  = NULL;
    Entry *entry, *e;

    hash = HashCode<k>(key) % tableLength;

    for (entry = table[hash]; entry != NULL; entry = entry->next) {
        // just to make sure we're using the correct overloaded operator for the key
        if (!(entry->GetKey() == key)) {
            prev = entry;
            continue;
        }

        if (defaultEntry == entry) {
            defaultEntry = prev ? prev : table[hash];
            // find a default entry
            for (i = 0; i < tableLength && !defaultEntry; i++) {
                for (e = table[i]; e; e = e->next) {
                    if (e == entry) { continue; }
                    defaultEntry = e;
                    break;
                }
            }
        }

        if (prev) {
            prev->next = entry->next;
        } else {
            table[hash] = entry->next;
        }

        count--;
        delete entry;

        return true;
    }

    return false;
}

template<typename k, typename v>
v *con_set<k, v>::findKeyValue(const k& key) const
{
    Entry *entry = findKeyEntry(key);

    if (entry != NULL) {
        return &entry->value;
    } else {
        return NULL;
    }
}

template<typename key, typename value>
key *con_set<key, value>::firstKeyValue(void)
{
    if (defaultEntry) {
        return &defaultEntry->GetKey();
    } else {
        return NULL;
    }
}

template<typename k, typename v>
v& con_set<k, v>::addKeyValue(const k& key)
{
    Entry *entry = addKeyEntry(key);

    return entry->value;
}

template<typename k, typename v>
v& con_set<k, v>::addNewKeyValue(const k& key)
{
    Entry *entry = addNewKeyEntry(key);

    return entry->value;
}

template<typename k, typename v>
bool con_set<k, v>::keyExists(const k& key)
{
    Entry *entry;

    for (entry = table; entry != NULL; entry = entry->next) {
        if (entry->GetKey() == key) {
            return true;
        }
    }

    return false;
}

template<typename key, typename value>
unsigned int con_set<key, value>::size()
{
    return count;
}

template<typename key, typename value>
class con_set_enum
{
    friend class con_map_enum<key, value>;

public:
    using Entry = typename con_set<key, value>::Entry;

protected:
    con_set<key, value> *m_Set;
    unsigned int         m_Index;
    Entry               *m_CurrentEntry;
    Entry               *m_NextEntry;

public:
    con_set_enum();
    con_set_enum(con_set<key, value>& set);

    bool operator=(con_set<key, value>& set);

    Entry *NextElement(void);
    Entry *CurrentElement(void);
};

template<typename key, typename value>
con_set_enum<key, value>::con_set_enum()
{
    m_Set          = NULL;
    m_Index        = 0;
    m_CurrentEntry = NULL;
    m_NextEntry    = NULL;
}

template<typename key, typename value>
con_set_enum<key, value>::con_set_enum(con_set<key, value>& set)
{
    *this = set;
}

template<typename key, typename value>
bool con_set_enum<key, value>::operator=(con_set<key, value>& set)
{
    m_Set          = &set;
    m_Index        = m_Set->tableLength;
    m_CurrentEntry = NULL;
    m_NextEntry    = NULL;

    return true;
}

template<typename key, typename value>
typename con_set_enum<key, value>::Entry *con_set_enum<key, value>::CurrentElement(void)
{
    return m_CurrentEntry;
}

template<typename key, typename value>
typename con_set_enum<key, value>::Entry *con_set_enum<key, value>::NextElement(void)
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

template<typename key, typename value>
class con_map
{
    friend class con_map_enum<key, value>;

private:
    con_set<key, value> m_con_set;

public:
#ifdef ARCHIVE_SUPPORTED
    void Archive(Archiver& arc);
#endif

    void         clear();
    virtual void resize(int count = 0);

    value& operator[](const key& index);

    value *find(const key& index);
    bool   remove(const key  &index);

    unsigned int size();
};

template<typename key, typename value>
void con_map<key, value>::clear()
{
    m_con_set.clear();
}

template<typename key, typename value>
void con_map<key, value>::resize(int count)
{
    m_con_set.resize(count);
}

template<typename key, typename value>
value& con_map<key, value>::operator[](const key& index)
{
    return m_con_set.addKeyValue(index);
}

template<typename key, typename value>
value *con_map<key, value>::find(const key& index)
{
    return m_con_set.findKeyValue(index);
}

template<typename key, typename value>
bool con_map<key, value>::remove(const key& index)
{
    return m_con_set.remove(index);
}

template<typename key, typename value>
unsigned int con_map<key, value>::size(void)
{
    return m_con_set.size();
}

template<typename key, typename value>
class con_map_enum
{
public:
    using Entry = typename con_set_enum<key, value>::Entry;

private:
    con_set_enum<key, value> m_Set_Enum;

public:
    con_map_enum();
    con_map_enum(con_map<key, value>& map);

    bool operator=(con_map<key, value>& map);

    key   *NextKey(void);
    value *NextValue(void);
    key   *CurrentKey(void);
    value *CurrentValue(void);
};

template<typename key, typename value>
con_map_enum<key, value>::con_map_enum()
{
    m_Set_Enum.m_Set          = NULL;
    m_Set_Enum.m_Index        = 0;
    m_Set_Enum.m_CurrentEntry = NULL;
    m_Set_Enum.m_NextEntry    = NULL;
}

template<typename key, typename value>
con_map_enum<key, value>::con_map_enum(con_map<key, value>& map)
{
    *this = map;
}

template<typename key, typename value>
bool con_map_enum<key, value>::operator=(con_map<key, value>& map)
{
    m_Set_Enum = map.m_con_set;

    return true;
}

template<typename key, typename value>
key *con_map_enum<key, value>::CurrentKey(void)
{
    Entry *entry = m_Set_Enum.CurrentElement();

    if (entry) {
        return &entry->GetKey();
    } else {
        return NULL;
    }
}

template<typename key, typename value>
value *con_map_enum<key, value>::CurrentValue(void)
{
    Entry *entry = m_Set_Enum.CurrentElement();

    if (entry) {
        return &entry->value;
    } else {
        return NULL;
    }
}

template<typename key, typename value>
key *con_map_enum<key, value>::NextKey(void)
{
    Entry *entry = m_Set_Enum.NextElement();

    if (entry) {
        return &entry->GetKey();
    } else {
        return NULL;
    }
}

template<typename key, typename value>
value *con_map_enum<key, value>::NextValue(void)
{
    Entry *entry = m_Set_Enum.NextElement();

    if (entry) {
        return &entry->value;
    } else {
        return NULL;
    }
}
