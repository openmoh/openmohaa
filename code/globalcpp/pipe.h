#pragma once

#include "listener.h"

class MessageEvent {
private:
	byte* Buffer;
	size_t AllocatedSize;
	size_t BufferSize;
	byte* Position;
	bool bReadMode;

public:
	MessageEvent();
	MessageEvent(byte* Buffer, size_t Size);
	~MessageEvent();

	void Reset();
	byte* SetReadMode(size_t Size);

	bool ReadBool();
	int ReadInteger();
	str ReadString();
	byte ReadByte();

	void WriteBool(bool Value);
	void WriteInteger(int Value);
	void WriteString(const char* Value);

	byte* GetData() const;
	size_t GetDataSize() const;

private:
	bool FinishedReading();
	void EnsureAllocated();
};

class PipeClass {
private:
	void *m_phSourceReadHandle;
	void *m_phSourceWriteHandle;
	void *m_phTargetReadHandle;
	void *m_phTargetWriteHandle;

public:
	PipeClass();
	PipeClass(void* SourceHandle, void* TargetHandle);
	~PipeClass();

	bool IsValid() const;
	bool IsValidForWriting() const;
	void Read(MessageEvent* Msg, bool bWait = false);
	void Send(const MessageEvent* Msg);
	bool HasData();

	void* GetSourceNativeHandle();
	void* GetTargetNativeHandle();

private:
	void ReadPipeData(MessageEvent* Msg, bool bWait = false );
	void WritePipeData(const MessageEvent* Msg);
};
