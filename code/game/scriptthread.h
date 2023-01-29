#pragma once

#include <listener.h>

class ScriptThread : public Listener
{
	friend class Flag;
	friend class EndOn;
	friend class Listener;
	friend class ScriptMaster;

private:
	ScriptVM* m_ScriptVM;
	SafePtr< ScriptThread > m_WaitingContext;

private:
	void		ScriptExecuteInternal(ScriptVariable* data = NULL, int dataSize = 0);

public:
	CLASS_PROTOTYPE(ScriptThread);

#ifndef _DEBUG_MEM
	void* operator new(size_t size);
	void operator delete(void* ptr);
#endif

	virtual void	Archive(Archiver& arc) override;
	void			ArchiveInternal(Archiver& arc);
	virtual void	StartedWaitFor(void) override;
	virtual void	StoppedNotify(void) override;
	virtual void	StoppedWaitFor(const_str name, bool bDeleting) override;
	virtual	ScriptThread* CreateThreadInternal(const ScriptVariable& label) override;
	virtual ScriptThread* CreateScriptInternal(const ScriptVariable& label) override;

	ScriptThread();
	ScriptThread(ScriptClass* scriptClass, unsigned char* pCodePos);
	virtual ~ScriptThread();

	void			Execute(Event& ev);
	void			Execute(Event* ev = NULL);
	void			Execute(ScriptVariable* data, int dataSize);
	void			DelayExecute(Event& ev);
	void			DelayExecute(Event* ev = NULL);

	void			AllowContextSwitch(bool allow = true);

	ScriptClass* GetScriptClass(void);
	str				ScriptThread::FileName(void);
	int				GetThreadState(void);
	ScriptThread* GetWaitingContext(void);
	void			SetWaitingContext(ScriptThread* thread);

	void			HandleContextSwitch(ScriptThread* childThread);

	void			Pause(void);
	void			ScriptExecute(ScriptVariable* data, int dataSize, ScriptVariable& returnValue);
	void			Stop(void);
	void			Wait(float time);
	void			StartTiming(float time);
	void			StartTiming(void);

	void		CanSwitchTeams(Event* ev);
	bool		CanScriptTracePrint(void);
	void		CharToInt(Event* ev);
	void		Conprintf(Event* ev);
	void		CreateHUD(Event* ev);
	void		Earthquake(Event* ev);
	void		FadeSound(Event* ev);
	void		FileOpen(Event* ev);
	void		FileWrite(Event* ev);
	void		FileRead(Event* ev);
	void		FileClose(Event* ev);
	void		FileEof(Event* ev);
	void		FileSeek(Event* ev);
	void		FileTell(Event* ev);
	void		FileRewind(Event* ev);
	void		FilePutc(Event* ev);
	void		FilePuts(Event* ev);
	void		FileGetc(Event* ev);
	void		FileGets(Event* ev);
	void		FileError(Event* ev);
	void		FileFlush(Event* ev);
	void		FileExists(Event* ev);
	void		FileReadAll(Event* ev);
	void		FileSaveAll(Event* ev);
	void		FileRemove(Event* ev);
	void		FileRename(Event* ev);
	void		FileCopy(Event* ev);
	void		FileReadPak(Event* ev);
	void		FileList(Event* ev);
	void		FileNewDirectory(Event* ev);
	void		FileRemoveDirectory(Event* ev);
	void		FlagClear(Event* ev);
	void		FlagInit(Event* ev);
	void		FlagSet(Event* ev);
	void		FlagWait(Event* ev);
	void		GetAreaEntities(Event* ev);
	void		GetArrayKeys(Event* ev);
	void		GetArrayValues(Event* ev);
	void		GetEntArray(Event* ev);
	void		GetPlayerNetname(Event* ev);
	void		GetPlayerIP(Event* ev);
	void		GetPlayerPing(Event* ev);
	void		GetPlayerClientNum(Event* ev);
	void		GetTime(Event* ev);
	void		GetDate(Event* ev);
	void		GetTimeZone(Event* ev);
	void		PregMatch(Event* ev);
	void		EventHudDraw3d(Event* ev);
	void		EventHudDrawTimer(Event* ev);
	void		EventHudDrawShader(Event* ev);
	void		EventHudDrawAlign(Event* ev);
	void		EventHudDrawRect(Event* ev);
	void		EventHudDrawVirtualSize(Event* ev);
	void		EventHudDrawColor(Event* ev);
	void		EventHudDrawAlpha(Event* ev);
	void		EventHudDrawString(Event* ev);
	void		EventHudDrawFont(Event* ev);
	void		EventIHudDraw3d(Event* ev);
	void		EventIHudDrawShader(Event* ev);
	void		EventIHudDrawAlign(Event* ev);
	void		EventIHudDrawRect(Event* ev);
	void		EventIHudDrawVirtualSize(Event* ev);
	void		EventIHudDrawColor(Event* ev);
	void		EventIHudDrawAlpha(Event* ev);
	void		EventIHudDrawString(Event* ev);
	void		EventIHudDrawFont(Event* ev);
	void		EventIHudDrawTimer(Event* ev);
	void		EventIsArray(Event* ev);
	void		EventIsDefined(Event* ev);
	void		EventIsOnGround(Event* ev);
	void		EventIsOutOfBounds(Event* ev);
	void		GetEntity(Event* ev);
	void		MathCos(Event* ev);
	void		MathSin(Event* ev);
	void		MathTan(Event* ev);
	void		MathACos(Event* ev);
	void		MathASin(Event* ev);
	void		MathATan(Event* ev);
	void		MathATan2(Event* ev);
	void		MathCosH(Event* ev);
	void		MathSinH(Event* ev);
	void		MathTanH(Event* ev);
	void		MathExp(Event* ev);
	void		MathFrexp(Event* ev);
	void		MathLdexp(Event* ev);
	void		MathLog(Event* ev);
	void		MathLog10(Event* ev);
	void		MathModf(Event* ev);
	void		MathPow(Event* ev);
	void		MathSqrt(Event* ev);
	void		MathCeil(Event* ev);
	void		MathFloor(Event* ev);
	void		MathFmod(Event* ev);
	void		StringBytesCopy(Event* ev);
	void		Md5File(Event* ev);
	void		Md5String(Event* ev);
	void		RegisterEvent(Event* ev);
	void		RestoreSound(Event* ev);
	void		RemoveArchivedClass(Event* ev);
	void		ServerStufftext(Event* ev);
	void		SetTimer(Event* ev);
	void		TeamGetScore(Event* ev);
	void		TeamSetScore(Event* ev);
	void		TeamSwitchDelay(Event* ev);
	void		TraceDetails(Event* ev);
	void		TypeOfVariable(Event* ev);
	void		UnregisterEvent(Event* ev);
	void		VisionGetNaked(Event* ev);
	void		VisionSetNaked(Event* ev);
	void		CancelWaiting(Event* ev);

	void		GetAbs(Event* ev);
	void		AddObjective(Event* ev);
	void		AddObjective(int index, int status, str text, Vector location);
	void		ClearObjectiveLocation(Event* ev);
	void		ClearObjectiveLocation(void);
	void		SetObjectiveLocation(Event* ev);
	void		SetObjectiveLocation(Vector vLocation);
	void		SetCurrentObjective(Event* ev);
	void		SetCurrentObjective(int iObjective);
	void		AllAIOff(Event* ev);
	void		AllAIOn(Event* ev);
	void		EventTeamWin(Event* ev);

	void		Angles_PointAt(Event* ev);
	void		Angles_ToForward(Event* ev);
	void		Angles_ToLeft(Event* ev);
	void		Angles_ToUp(Event* ev);

	void		Assert(Event* ev);
	void		Cache(Event* ev);

	void		CastBoolean(Event* ev);
	void		CastEntity(Event* ev);
	void		CastFloat(Event* ev);
	void		CastInt(Event* ev);
	void		CastString(Event* ev);

	void		CreateReturnThread(Event* ev);
	void		CreateThread(Event* ev);
	void		ExecuteReturnScript(Event* ev);
	void		ExecuteScript(Event* ev);

	void		EventCreateListener(Event* ev);
	void		EventDelayThrow(Event* ev);
	void		EventEnd(Event* ev);
	void		EventTimeout(Event* ev);
	void		EventError(Event* ev);
	void		EventGoto(Event* ev);
	void		EventRegisterCommand(Event* ev);
	void		EventGetCvar(Event* ev);
	void		EventSetCvar(Event* ev);
	void		EventSightTrace(Event* ev);
	void		EventTrace(Event* ev);
	void		EventThrow(Event* ev);
	void		EventWait(Event* ev);
	void		EventWaitFrame(Event* ev);

	void		EventIsAlive(Event* ev);

	void		EventEarthquake(Event* ev);
	void		MapEvent(Event* ev);
	void		CueCamera(Event* ev);
	void		CuePlayer(Event* ev);

	void		FreezePlayer(Event* ev);
	void		ReleasePlayer(Event* ev);
	void		EventDrawHud(Event* ev);

	void		EventRadiusDamage(Event* ev);

	void		GetSelf(Event* ev);

	void		IPrintln(Event* ev);
	void		IPrintln_NoLoc(Event* ev);
	void		IPrintlnBold(Event* ev);
	void		IPrintlnBold_NoLoc(Event* ev);
	void		Println(Event* ev);
	void		Print(Event* ev);
	void		MPrintln(Event* ev);
	void		MPrint(Event* ev);
	void		EventPrint3D(Event* ev);

	void		EventBspTransition(Event* ev);
	void		EventLevelTransition(Event* ev);
	void		EventMissionTransition(Event* ev);

	void		EventGetBoundKey1(Event* ev);
	void		EventGetBoundKey2(Event* ev);
	void		EventLocConvertString(Event* ev);

	void		RandomFloat(Event* ev);
	void		RandomInt(Event* ev);

	void		Spawn(Event* ev);
	Listener* SpawnInternal(Event* ev);
	void		SpawnReturn(Event* ev);

	void		ForceMusicEvent(Event* ev);
	void		SoundtrackEvent(Event* ev);
	void		RestoreSoundtrackEvent(Event* ev);

	void		EventVectorAdd(Event* ev);
	void		EventVectorCloser(Event* ev);
	void		EventVectorCross(Event* ev);
	void		EventVectorDot(Event* ev);
	void		EventVectorLength(Event* ev);
	void		EventVectorNormalize(Event* ev);
	void		EventVectorScale(Event* ev);
	void		EventVectorSubtract(Event* ev);
	void		EventVectorToAngles(Event* ev);
	void		EventVectorWithin(Event* ev);


	void		FadeIn(Event* ev);
	void		FadeOut(Event* ev);
	void		ClearFade(Event* ev);
	void		Letterbox(Event* ev);
	void		ClearLetterbox(Event* ev);
	void		MusicEvent(Event* ev);
	void		MusicVolumeEvent(Event* ev);
	void		RestoreMusicVolumeEvent(Event* ev);
	void		SetCinematic(Event* ev);
	void		SetNonCinematic(Event* ev);
	void		StuffCommand(Event* ev);
	void		KillEnt(Event* ev);
	void		RemoveEnt(Event* ev);
	void		KillClass(Event* ev);
	void		RemoveClass(Event* ev);
	void		CameraCommand(Event* ev);
	void		SetLightStyle(Event* ev);
	void		CenterPrint(Event* ev);
	void		LocationPrint(Event* ev);
	void		TriggerEvent(Event* ev);
	void		ServerEvent(Event* ev);
	void		MissionFailed(Event* ev);
};