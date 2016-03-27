/*
===========================================================================
Copyright (C) 2008 the OpenMoHAA team

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

// safeptr.h: Safe Pointers

#ifndef __SAFEPTR_H__
#define __SAFEPTR_H__

class SafePtrBase
{
private:
	void	AddReference( Class *ptr );
	void	RemoveReference( Class *ptr );

protected:
	SafePtrBase	*prev;
	SafePtrBase	*next;
	Class		*ptr;

public:
	SafePtrBase();
	virtual		~SafePtrBase();
	void		InitSafePtr( Class *newptr );
	Class		*Pointer( void );
	void		Clear( void );
};

inline void SafePtrBase::AddReference( Class *ptr )
{
	if( !ptr->SafePtrList )
	{
		ptr->SafePtrList = this;
		LL_Reset( this, next, prev );
	}
	else
	{
		LL_Add( ptr->SafePtrList, this, next, prev );
	}
}

inline void SafePtrBase::RemoveReference( Class *ptr )
{
	if( ptr->SafePtrList == this )
	{
		if( ptr->SafePtrList->next == this )
		{
			ptr->SafePtrList = NULL;
		}
		else
		{
			ptr->SafePtrList = next;
			LL_Remove( this, next, prev );
		}
	}
	else
	{
		LL_Remove( this, next, prev );
	}
}

inline void SafePtrBase::Clear( void )
{
	if( ptr )
	{
		RemoveReference( ptr );
		ptr = NULL;
	}
}

inline SafePtrBase::SafePtrBase()
{
	prev = NULL;
	next = NULL;
	ptr = NULL;
}

inline SafePtrBase::~SafePtrBase()
{
	Clear();
}

inline Class * SafePtrBase::Pointer( void )
{
	return ptr;
}

inline void SafePtrBase::InitSafePtr( Class *newptr )
{
	if( ptr != newptr )
	{
		if( ptr )
		{
			RemoveReference( ptr );
		}

		ptr = newptr;
		if( ptr == NULL )
		{
			return;
		}

		AddReference( ptr );
	}
}

template<class T>
class SafePtr : public SafePtrBase
{
public:
	SafePtr( T* objptr = 0 );
	SafePtr( const SafePtr& obj );

	SafePtr& operator=( const SafePtr& obj );
	SafePtr& operator=( T * const obj );

#ifdef LINUX
	friend bool operator==<>( SafePtr<T> a, T *b );
	friend bool operator!=<>( SafePtr<T> a, T *b );
	friend bool operator==<>( T *a, SafePtr<T> b );
	friend bool operator!=<>( T *a, SafePtr<T> b );
	friend bool operator==<>( SafePtr<T> a, SafePtr<T> b );
	friend bool operator!=<>( SafePtr<T> a, SafePtr<T> b );
#else
	// The compiler/linker gets confused when the friend functions definition are not templated
	template< class U > friend bool operator==( SafePtr<U> a, U *b );
	template< class U > friend bool operator!=( SafePtr<U> a, U *b );
	template< class U > friend bool operator==( U *a, SafePtr<U> b );
	template< class U > friend bool operator!=( U *a, SafePtr<U> b );
	template< class U > friend bool operator==( SafePtr<U> a, SafePtr<U> b );
	template< class U > friend bool operator!=( SafePtr<U> a, SafePtr<U> b );
#endif

	bool operator !( ) const;
	operator T*( ) const;
	T* operator->( ) const;
	T& operator*( ) const;
};

template<class T>
inline SafePtr<T>::SafePtr( T* objptr )
{
	InitSafePtr( ( Class * )objptr );
}

template<class T>
inline SafePtr<T>::SafePtr( const SafePtr& obj )
{
	InitSafePtr( obj.ptr );
}

template<class T>
inline SafePtr<T>& SafePtr<T>::operator=( const SafePtr& obj )
{
	InitSafePtr( obj.ptr );
	return *this;
}

template<class T>
inline SafePtr<T>& SafePtr<T>::operator=( T * const obj )
{
	InitSafePtr( obj );
	return *this;
}

template<class T>
inline bool operator==( SafePtr<T> a, T *b )
{
	return a.ptr == b;
}

template<class T>
inline bool operator!=( SafePtr<T> a, T* b )
{
	return a.ptr != b;
}

template<class T>
inline bool operator==( T* a, SafePtr<T> b )
{
	return a == b.ptr;
}

template<class T>
inline bool operator!=( T* a, SafePtr<T> b )
{
	return a != b.ptr;
}

template<class T>
inline bool operator==( SafePtr<T> a, SafePtr<T> b )
{
	return a.ptr == b.ptr;
}

template<class T>
inline bool operator!=( SafePtr<T> a, SafePtr<T> b )
{
	return a.ptr != b.ptr;
}

template<class T>
inline bool SafePtr<T>::operator !( ) const
{
	return ptr == NULL;
}

template<class T>
inline SafePtr<T>::operator T*( ) const
{
	return ( T * )ptr;
}

template<class T>
inline T* SafePtr<T>::operator->( ) const
{
	return ( T * )ptr;
}

template<class T>
inline T& SafePtr<T>::operator*( ) const
{
	return *( T * )ptr;
}

#endif
