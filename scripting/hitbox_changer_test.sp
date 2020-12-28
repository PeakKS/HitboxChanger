#include <sourcemod>
#include <cstrike>
#include <sdktools>

#define AUTOLOAD_EXTENSIONS
#define REQUIRE_EXTENSIONS
#include <hitboxchanger>

#pragma semicolon 1
#pragma newdecls required

#define PLUGIN_VERSION "1.00"

public Plugin myinfo = 
{
	name = "Hitbox Changer Test",
	author = "PikaJew",
	description = "Test plugin for Hitbox Changer Extension",
	version = PLUGIN_VERSION,
	url = "https://cascadeservers.net/"
};

Hitbox g_hbLeftFoot;
Hitbox g_hbRightFoot;
Hitbox g_hbTemp;

public APLRes AskPluginLoad2(Handle myself, bool late, char[] error, int err_max)
{
	// No need for the old GetGameFolderName setup.
	EngineVersion g_engineversion = GetEngineVersion();
	if (g_engineversion != Engine_CSGO)
	{
		SetFailState("This plugin was made for use with Counter-Strike: Global Offensive only.");
	}
} 

public void OnPluginStart()
{
	/**
	 * @note For the love of god, please stop using FCVAR_PLUGIN.
	 * Console.inc even explains this above the entry for the FCVAR_PLUGIN define.
	 * "No logic using this flag ever existed in a released game. It only ever appeared in the first hl2sdk."
	 */
	CreateConVar("sm_hitbox_changer_test_version", PLUGIN_VERSION, "Standard plugin version ConVar. Please don't change me!", FCVAR_REPLICATED|FCVAR_NOTIFY|FCVAR_DONTRECORD);
	
	RegAdminCmd("sm_feet", CMD_Feet, ADMFLAG_ROOT, "Remove all hitboxes except feet for equipped model");
	RegAdminCmd("sm_hbinfo", CMD_PrintHB, ADMFLAG_ROOT, "Print info on hitboxes to server console");
	RegAdminCmd("sm_boneinfo", CMD_PrintBones, ADMFLAG_ROOT, "Print info on bones to server console");
	RegAdminCmd("sm_sethbnum", CMD_NumHB, ADMFLAG_ROOT, "Set HB set numhitboxes to passed amount");
	RegAdminCmd("sm_copyhb", CMD_CopyHB, ADMFLAG_ROOT, "Get the Hitbox and save it to the global temp");
	RegAdminCmd("sm_findbone", CMD_FindBone, ADMFLAG_ROOT, "Find the index of a bone");
	RegAdminCmd("sm_assigntobone", CMD_AssignHB, ADMFLAG_ROOT, "Assign one of our premade hitboxes to specified bone");
	RegAdminCmd("sm_validbones", CMD_ValidBones, ADMFLAG_ROOT, "Print a list of bones with hitbox flag set");
	//RegAdminCmd("sm_drawhitbox", CMD_DrawHBox, ADMFLAG_ROOT, "Draw server side hitboxes"); Can maybe work on windows?
	/**
	 * Default Left Foot:	bone: "ankle_L", group: 6, bbmin: [ -2.5, -3.8, -2.8 ], bbmax: [ 10.4, -0.2, 2.0 ], angle: [ 0.0, 24.1, 0.0 ], radius: -1
 	 * Default Right Foot:	bone: "ankle_R", group: 7, bbmin: [ -10.4, 0.3, -2.0 ], bbmax: [ 2.6,   4.0, 2.8 ], angle: [ 0.0, 24.5, 0.0 ], radius: -1
 	 */
	g_hbLeftFoot.group = 6;
	g_hbLeftFoot.bbmin[0] = -2.5;
	g_hbLeftFoot.bbmin[1] = -3.8;
	g_hbLeftFoot.bbmin[2] = -2.8;
	g_hbLeftFoot.bbmax[0] = 10.4;
	g_hbLeftFoot.bbmax[1] = -0.2;
	g_hbLeftFoot.bbmax[2] = 2.0;
	g_hbLeftFoot.angle[0] = 0.0;
	g_hbLeftFoot.angle[1] = 24.1;
	g_hbLeftFoot.angle[2] = 0.0;
	g_hbLeftFoot.radius = -1.0;
	
	g_hbRightFoot.group = 7;
	g_hbRightFoot.bbmin[0] = -10.4;
	g_hbRightFoot.bbmin[1] = 0.3;
	g_hbRightFoot.bbmin[2] = -2.0;
	g_hbRightFoot.bbmax[0] = 2.6;
	g_hbRightFoot.bbmax[1] = 4.0;
	g_hbRightFoot.bbmax[2] = 2.8;
	g_hbRightFoot.angle[0] = 0.0;
	g_hbRightFoot.angle[1] = 24.5;
	g_hbRightFoot.angle[2] = 0.0;
	g_hbRightFoot.radius = -1.0;
}

public Action CMD_Feet(int iClient, int iArgs)
{
	char model[PLATFORM_MAX_PATH];
	GetClientModel(iClient, model, sizeof(model));
	
	int modelIndex = PrecacheModel(model);
	int ankle_L = FindBone("ankle_L", modelIndex);
	int ankle_R = FindBone("ankle_R", modelIndex);
	PrintToServer("Left Ankle: %i, Right Ankle: %i", ankle_L, ankle_R);
	g_hbLeftFoot.CopyToModel(modelIndex, 0, ankle_L);
	g_hbRightFoot.CopyToModel(modelIndex, 1, ankle_R);
	PrintToChat(iClient, "Feet Only: %s", (HitboxInfo(modelIndex))? "Success" : "Failure");
}

public Action CMD_NumHB(int iClient, int iArgs)
{
	if(iArgs != 1)
	{
		PrintToChat(iClient, "Usage: sm_sethbnum <amount>");
		return;
	}
	char buffer[8];
	GetCmdArg(1, buffer, sizeof(buffer));
	int num = StringToInt(buffer);
	
	char model[PLATFORM_MAX_PATH];
	GetClientModel(iClient, model, sizeof(model));
	
	int modelIndex = PrecacheModel(model);

	SetNumHitboxes(modelIndex, num);
	PrintToChat(iClient, "Printout: %s", (HitboxInfo(modelIndex))? "Success" : "Failure");
}

public Action CMD_CopyHB(int iClient, int iArgs)
{
	if(iArgs != 1)
	{
		PrintToChat(iClient, "Usage: sm_copyhb <hitbox>");
		return;
	}
	char buffer[8];
	GetCmdArg(1, buffer, sizeof(buffer));
	int num = StringToInt(buffer);
	
	char model[PLATFORM_MAX_PATH];
	GetClientModel(iClient, model, sizeof(model));
	
	int modelIndex = PrecacheModel(model);

	int bone;
	g_hbTemp.CopyFromModel(modelIndex, num, bone);
	PrintToServer("Bone: %i", bone);
	g_hbTemp.Print();
}

public Action CMD_AssignHB(int iClient, int iArgs)
{
	if(iArgs != 2)
	{
		PrintToChat(iClient, "Usage: sm_assigntobone <hitbox> <bone>");
		return;
	}
	char sHB[8];
	GetCmdArg(1, sHB, sizeof(sHB));
	int hbi = StringToInt(sHB);
	
	char sB[8];
	GetCmdArg(2, sB, sizeof(sB));
	int bi = StringToInt(sB);
	
	char model[PLATFORM_MAX_PATH];
	GetClientModel(iClient, model, sizeof(model));
	
	int modelIndex = PrecacheModel(model);

	g_hbLeftFoot.CopyToModel(modelIndex, hbi, bi);
	PrintToChat(iClient, "Feet Only: %s", (HitboxInfo(modelIndex))? "Success" : "Failure");
}

public Action CMD_PrintHB(int iClient, int iArgs)
{
	char model[256];
	GetClientModel(iClient, model, sizeof(model));
	int modelIndex = PrecacheModel(model);
	PrintToChat(iClient, "Printout: %s", (HitboxInfo(modelIndex))? "Success" : "Failure");
}

public Action CMD_PrintBones(int iClient, int iArgs)
{
	char model[256];
	GetClientModel(iClient, model, sizeof(model));
	int modelIndex = PrecacheModel(model);
	PrintToChat(iClient, "Printout: %s", (BoneInfo(modelIndex))? "Success" : "Failure");
}

public Action CMD_FindBone(int iClient, int iArgs)
{
	
	if(iArgs != 1)
	{
		PrintToChat(iClient, "Usage: sm_sethbnum <amount>");
		return;
	}
	char buffer[32];
	GetCmdArg(1, buffer, sizeof(buffer));

	
	char model[256];
	GetClientModel(iClient, model, sizeof(model));
	int modelIndex = PrecacheModel(model);
	
	int bone = FindBone(buffer, modelIndex);
	if(bone != -1)
		PrintToChat(iClient, "%s bone found: %i", buffer, bone);
	else
		PrintToChat(iClient, "%s bone not found", buffer);
}

public Action CMD_GetNumHB(int iClient, int iArgs)
{	
	char model[256];
	GetClientModel(iClient, model, sizeof(model));
	int modelIndex = PrecacheModel(model);
	
	int hbCount = GetNumHitboxes(modelIndex);
	if(hbCount != -1)
		PrintToChat(iClient, "There are %i hitboxes",  hbCount);
	else
		PrintToChat(iClient, "Error, couldnt get count");
}

public Action CMD_ValidBones(int iClient, int iArgs)
{	
	char model[256];
	GetClientModel(iClient, model, sizeof(model));
	int modelIndex = PrecacheModel(model);
	
	PrintToChat(iClient, "Find Valid Bones: %s", (FindValidBones(modelIndex))? "Success" : "Failure");
}

/* This might work on windows
public Action CMD_DrawHBox(int iClient, int iArgs)
{	
	char model[256];
	GetClientModel(iClient, model, sizeof(model));
	int modelIndex = PrecacheModel(model);

	PrintToChat(iClient, "Feet Only: %s", (DrawServerHitboxes(modelIndex))? "Success" : "Failure");
}
*/