#include "class.h"

class con_timer : public Class
{
public:
    class Element
    {
    public:
        Class *obj;
        int    inttime;
    };

private:
    Container<con_timer::Element> m_Elements;
    bool                          m_bDirty;
    int                           m_inttime;

public:
    con_timer();

    void AddElement(Class *e, int inttime);
    void RemoveElement(Class *e);

    Class *GetNextElement(int& foundTime);

    void SetDirty(void);
    bool IsDirty(void);
    void SetTime(int inttime);

#if defined(ARCHIVE_SUPPORTED)
    static void ArchiveElement(class Archiver& arc, Element *e);
    void        Archive(class Archiver       &arc) override;
#endif
};

inline void con_timer::SetDirty(void)
{
    m_bDirty = true;
};

inline bool con_timer::IsDirty(void)
{
    return m_bDirty;
};

inline void con_timer::SetTime(int inttime)
{
    m_inttime = inttime;
    m_bDirty  = true;
}
