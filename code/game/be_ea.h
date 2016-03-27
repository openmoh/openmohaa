// Copyright (C) 1999-2000 Id Software, Inc.
//

/*****************************************************************************
 * name:		be_ea.h
 *
 * desc:		elementary actions
 *
 * $Archive: /Code/DLLs/game/be_ea.h $
 * $Author: Jwaters $ 
 * $Revision: 2 $
 * $Modtime: 8/08/02 12:17p $
 * $Date: 8/08/02 1:38p $
 *
 *****************************************************************************/

//ClientCommand elementary actions
void EA_Say(int client, char *str);
void EA_SayTeam(int client, char *str);
void EA_Command(int client, const char *command );

void EA_Action(int client, int action);
void EA_Crouch(int client);
void EA_Walk(int client);
void EA_MoveUp(int client);
void EA_MoveDown(int client);
void EA_MoveForward(int client);
void EA_MoveBack(int client);
void EA_MoveLeft(int client);
void EA_MoveRight(int client);
void EA_ToggleFireState(int client);
void EA_Attack(int client, int primarydangerous, int altdangerous);
void EA_Respawn(int client);
void EA_Talk(int client);
void EA_Gesture(int client);
void EA_Use(int client);

//regular elementary actions
void EA_SelectWeapon(int client, int weapon);
void EA_Jump(int client);
void EA_DelayedJump(int client);
void EA_Move(int client, vec3_t dir, float speed);
void EA_View(int client, vec3_t viewangles);

//send regular input to the server
void EA_EndRegular(int client, float thinktime);
void EA_GetInput(int client, float thinktime, bot_input_t *input);
void EA_ResetInput(int client);
//setup and shutdown routines
int EA_Setup(void);
void EA_Shutdown(void);
