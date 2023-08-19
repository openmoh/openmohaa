#include "con_timer.h"

#if defined(ARCHIVE_SUPPORTED)
#    include "../fgame/archive.h"
#endif

con_timer::con_timer(void)
{
    m_inttime = 0;
    m_bDirty  = false;
}

void con_timer::AddElement(Class *e, int inttime)
{
    Element element;

    element.obj     = e;
    element.inttime = inttime;

    m_Elements.AddObject(element);

    if (inttime <= m_inttime) {
        SetDirty();
    }
}

void con_timer::RemoveElement(Class *e)
{
    for (int i = m_Elements.NumObjects(); i > 0; i--) {
        Element *index = &m_Elements.ObjectAt(i);

        if (index->obj == e) {
            m_Elements.RemoveObjectAt(i);
            return;
        }
    }
}

Class *con_timer::GetNextElement(int& foundtime)
{
    int    best_inttime;
    int    i;
    int    foundIndex;
    Class *result;

    foundIndex   = 0;
    best_inttime = m_inttime;

    for (i = m_Elements.NumObjects(); i > 0; i--) {
        if (m_Elements.ObjectAt(i).inttime <= best_inttime) {
            best_inttime = m_Elements.ObjectAt(i).inttime;
            foundIndex   = i;
        }
    }

    if (foundIndex) {
        result = m_Elements.ObjectAt(foundIndex).obj;
        m_Elements.RemoveObjectAt(foundIndex);
        foundtime = best_inttime;
    } else {
        result   = NULL;
        m_bDirty = false;
    }

    return result;
}

#if defined(ARCHIVE_SUPPORTED)

void con_timer::ArchiveElement(Archiver& arc, Element *e)
{
    arc.ArchiveObjectPointer(&e->obj);
    arc.ArchiveInteger(&e->inttime);
}

void con_timer::Archive(Archiver& arc)
{
    arc.ArchiveBool(&m_bDirty);
    arc.ArchiveInteger(&m_inttime);

    m_Elements.Archive(arc, con_timer::ArchiveElement);
}
#endif
