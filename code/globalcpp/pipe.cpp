#include "pipe.h"
#include <Windows.h>

MessageEvent::MessageEvent()
{
	Buffer = NULL;
	BufferSize = 0;
	AllocatedSize = 0;
	Position = NULL;
	bReadMode = false;
}

MessageEvent::~MessageEvent()
{
	if (Buffer)
	{
		free(Buffer);
	}
}

void MessageEvent::Reset()
{
	if (Buffer)
	{
		free(Buffer);
	}

	Buffer = NULL;
	BufferSize = 0;
	AllocatedSize = 0;
	Position = NULL;
	bReadMode = false;
}

byte* MessageEvent::SetReadMode(size_t size)
{
	Reset();

	Buffer = (byte*)malloc(size);
	BufferSize = size;
	AllocatedSize = size;
	Position = Buffer;
	bReadMode = true;

	return Buffer;
}

bool MessageEvent::ReadBool()
{
	return !FinishedReading() ? *Position++ : false;
}

str MessageEvent::ReadString()
{
	str Value;

	while (1)
	{
		char CharValue = ReadByte();
		if (CharValue <= 0)
		{
			break;
		}

		Value += CharValue;
	}

	return Value;
}

byte MessageEvent::ReadByte()
{
	if (FinishedReading())
	{
		return -1;
	}

	return *Position++;
}

int MessageEvent::ReadInteger()
{
	if (FinishedReading())
	{
		return -1;
	}

	int Value = *(int*)Position;
	Position += 4;
	return Value;
}

void MessageEvent::WriteBool(bool Value)
{
	if (bReadMode)
	{
		return;
	}

	BufferSize += sizeof(bool);
	EnsureAllocated();

	*Position++ = Value;
}

void MessageEvent::WriteInteger(int Value)
{
	if (bReadMode)
	{
		return;
	}

	BufferSize += sizeof(int);
	EnsureAllocated();

	*(int*)Position = Value;
	Position += 4;
}

void MessageEvent::WriteString(const char* Value)
{
	if (bReadMode)
	{
		return;
	}

	size_t length = strlen(Value) + 1;

	BufferSize += length;
	EnsureAllocated();

	for (size_t i = 0; i < length; i++)
	{
		*Position++ = Value[i];
	}
}

byte* MessageEvent::GetData() const
{
	return Buffer;
}

size_t MessageEvent::GetDataSize() const
{
	return BufferSize;
}

bool MessageEvent::FinishedReading()
{
	return !bReadMode || (Position - Buffer >= BufferSize);
}

void MessageEvent::EnsureAllocated()
{
	if (!bReadMode && AllocatedSize < BufferSize)
	{
		AllocatedSize = BufferSize + 20;

		if (Buffer)
		{
			size_t p = Position - Buffer;

			Buffer = (byte*)realloc(Buffer, AllocatedSize);
			Position = Buffer + p;
		}
		else
		{
			Buffer = (byte*)malloc(AllocatedSize);
			Position = Buffer;
		}
	}
}

PipeClass::PipeClass()
{
	m_phSourceReadHandle = NULL;
	m_phSourceWriteHandle = NULL;
	m_phTargetReadHandle = NULL;
	m_phTargetWriteHandle = NULL;

	SECURITY_ATTRIBUTES PipeAttributes;
	PipeAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	PipeAttributes.bInheritHandle = TRUE;
	PipeAttributes.lpSecurityDescriptor = NULL;

	CreatePipe(&m_phSourceReadHandle, &m_phSourceWriteHandle, &PipeAttributes, 65535);
	CreatePipe(&m_phTargetReadHandle, &m_phTargetWriteHandle, &PipeAttributes, 65535);

	SetHandleInformation(m_phSourceWriteHandle, HANDLE_FLAG_INHERIT, 1);
	SetHandleInformation(m_phTargetReadHandle, HANDLE_FLAG_INHERIT, 1);
}

PipeClass::PipeClass(void* SourceHandle, void* TargetHandle)
{
	m_phSourceReadHandle = SourceHandle;
	m_phTargetWriteHandle = TargetHandle;

	m_phSourceWriteHandle = NULL;
	m_phTargetReadHandle = NULL;
}

PipeClass::~PipeClass()
{
	if (m_phSourceReadHandle)
	{
		CloseHandle(m_phSourceReadHandle);
	}

	if (m_phSourceWriteHandle)
	{
		CloseHandle(m_phSourceWriteHandle);
	}

	if (m_phTargetReadHandle)
	{
		CloseHandle(m_phTargetReadHandle);
	}

	if (m_phTargetWriteHandle)
	{
		CloseHandle(m_phTargetWriteHandle);
	}
}

/*
void PipeClass::ProcessPipe( bool bWait )
{
	Container<Event*> ConEvent = Read( bWait );

	for( int i = 1; i <= ConEvent.NumObjects(); i++ )
	{
		ProcessEvent( ConEvent.ObjectAt( i ) );
	}
}
*/

bool PipeClass::IsValid() const
{
	LARGE_INTEGER FileSize;

	return GetFileSizeEx(m_phSourceReadHandle, &FileSize);
}

bool PipeClass::IsValidForWriting() const
{
	LARGE_INTEGER FileSize;

	return GetFileSizeEx(m_phTargetWriteHandle, &FileSize);
}

void PipeClass::Read(MessageEvent* Msg, bool bWait)
{
	ReadPipeData(Msg, bWait);
}

/*
Container<Event*> PipeClass::Read( bool bWait )
{
	char *buffer;
	const char *com_token;
	str sCommand;
	Container<Event*> ConEvent;

	str data = ReadPipeData( bWait );

	buffer = ( char * )malloc( data.length() + 1 );
	strcpy( buffer, data.c_str() );

	char *b = buffer;

	while( 1 )
	{
		com_token = COM_Parse( &b );

		if( !com_token || !com_token[ 0 ] )
		{
			break;
		}

		sCommand = com_token;

		Event *ev = new Event( sCommand );

		while( 1 )
		{
			com_token = COM_GetToken( ( const char ** )&b, false );

			if( !com_token[ 0 ] )
				break;

			ev->AddString( com_token );
		}

		ConEvent.AddObject( ev );
	}

	free( buffer );
	return ConEvent;
}

void PipeClass::Send( const str& data )
{
	WritePipeData( data );
}
*/

void PipeClass::Send(const MessageEvent* Msg)
{
	WritePipeData(Msg);
}

bool PipeClass::HasData()
{
	LARGE_INTEGER FileSize;

	BOOL bSuccess = GetFileSizeEx(m_phSourceReadHandle, &FileSize);
	return bSuccess && FileSize.LowPart;
}

void* PipeClass::GetSourceNativeHandle()
{
	return m_phSourceWriteHandle;
}

void* PipeClass::GetTargetNativeHandle()
{
	return m_phTargetReadHandle;
}

void PipeClass::ReadPipeData(MessageEvent* Msg, bool bWait)
{
	LARGE_INTEGER FileSize;

	BOOL bSuccess = GetFileSizeEx(m_phSourceReadHandle, &FileSize);
	if (bSuccess && (FileSize.LowPart || bWait))
	{
		DWORD size;

		if (ReadFile(m_phSourceReadHandle, &size, sizeof(DWORD), NULL, NULL))
		{
			byte* buffer = Msg->SetReadMode(size);
			ReadFile(m_phSourceReadHandle, buffer, size, NULL, NULL);
		}
	}
}

void PipeClass::WritePipeData(const MessageEvent* Msg)
{
	DWORD size = (DWORD)Msg->GetDataSize();

	// Write the size of the message
	WriteFile(m_phTargetWriteHandle, &size, sizeof(DWORD), NULL, NULL);

	// Write the message
	WriteFile(m_phTargetWriteHandle, Msg->GetData(), size, NULL, NULL);
}
