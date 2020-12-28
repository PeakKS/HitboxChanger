/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod Sample Extension
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#include "extension.h"
#include <studio.h>
#include <ivmodelinfo.h>
//#include <ivdebugoverlay.h> directx weirdchamp
//#include <bone_setup.h>
//#include <bone_utils.cpp> fuck bones

/**
 * @file extension.cpp
 * @brief Implement extension code here.
 */

HitboxChanger	g_HitboxChanger;		/**< Global singleton for extension's main interface */
IVModelInfo		*g_pModelinfo = NULL;
//IVDebugOverlay	*g_pDebugOverlay = NULL;

SMEXT_LINK(&g_HitboxChanger);

cell_t HitboxInfo(IPluginContext *pContext, const cell_t *params)
{
	cell_t ModelIndex = params[1];
	const model_t *test = g_pModelinfo->GetModel(ModelIndex);
	studiohdr_t *mdl = g_pModelinfo->GetStudiomodel(test);
	mstudiohitboxset_t *hitboxset = mdl->pHitboxSet(0);
	
	if (hitboxset)
	{
		for (auto i = 0; i < hitboxset->numhitboxes; i++)
		{
			mstudiobbox_t *hitbox = hitboxset->pHitbox(i);
			if (!hitbox)
				continue;
			//Would print hitbox name as well but they seem to always be empty, so lets just print bone name
			rootconsole->ConsolePrint("HB: %i, Bone: %i, Bone Name: %s, BoneFlags: %i, Group: %i, Min: [ %.2f, %.2f, %.2f ], Max: [ %.2f, %.2f, %.2f ], Angle: [ %.2f, %.2f, %.2f ], Radius: %.2f",
										i, hitbox->bone, mdl->pBone(hitbox->bone)->pszName(), mdl->pBone(hitbox->bone)->flags, hitbox->group,
										hitbox->bbmin.x, hitbox->bbmin.y, hitbox->bbmin.z,
										hitbox->bbmax.x, hitbox->bbmax.y, hitbox->bbmax.z,
										hitbox->angOffsetOrientation.x, hitbox->angOffsetOrientation.y, hitbox->angOffsetOrientation.z,
										hitbox->flCapsuleRadius);
		}
	}
	else
	{
		rootconsole->ConsolePrint("[HitboxChanger]: !!!Hitbox Info Failure!!! Bad HitBoxSet");
		return -1;
	}
	return 1;
}

cell_t BoneInfo(IPluginContext *pContext, const cell_t *params)
{
	cell_t ModelIndex = params[1];
	const model_t *test = g_pModelinfo->GetModel(ModelIndex);
	studiohdr_t *mdl = g_pModelinfo->GetStudiomodel(test);
	
	if (mdl)
	{
		for (auto i = 0; i < mdl->numbones; i++)
		{
			const mstudiobone_t *bone = mdl->pBone(i);
			if (!bone)
				continue;
			rootconsole->ConsolePrint("Bone: %i, Name: %s, Flags: %i", i, bone->pszName(),bone->flags);
		}
	}
	else
	{
		rootconsole->ConsolePrint("[HitboxChanger]: !!!Bone Info Failure!!! Bad StudioHDR");
		return -1;
	}
	return 1;
}

cell_t SetHitbox(IPluginContext *pContext, const cell_t *params)
{	
	//Sourcepawn Native to follow:
	//native int SetHitbox(int modelIndex, int hitboxIndex, int bone, int group, const float min[3], const float max[3], const float angle[3], const float radius);
	cell_t modelIndex = params[1];
	cell_t hitboxIndex = params[2];
	
	const model_t *test = g_pModelinfo->GetModel(modelIndex);
	studiohdr_t *mdl = g_pModelinfo->GetStudiomodel(test);
	mstudiohitboxset_t *hitboxset = mdl->pHitboxSet(0);
	
	if (hitboxset)
	{	
		if(hitboxIndex < 0 || hitboxset->numhitboxes <= hitboxIndex)
		{
			rootconsole->ConsolePrint("[HitboxChanger]: !!!Set Hitbox Failure!!! Invalid Hitbox Index");
			return 0;
		}
		if(params[3] < 0 || mdl->numbones <= params[3])
		{
			rootconsole->ConsolePrint("[HitboxChanger]: !!!Get Hitbox Failure!!! Invalid Bone Index");
			return 0;
		}
		mstudiobbox_t *hitbox = hitboxset->pHitbox(hitboxIndex);
		if(hitbox)
		{
			if((mdl->pBone(params[3])->flags & BONE_USED_BY_HITBOX) == 0)
			{
				rootconsole->ConsolePrint("[HitboxChanger]: !!!Set Hitbox Failure!!! Bone does not have hitbox flag set");
				return 0;
			}
			hitbox->bone = params[3];			
			hitbox->group = params[4];
			
			cell_t *bbmin, *bbmax, *angle;
			pContext->LocalToPhysAddr(params[5], &bbmin);
			pContext->LocalToPhysAddr(params[6], &bbmax);
			pContext->LocalToPhysAddr(params[7], &angle);
			//Set Min Point
			hitbox->bbmin.x = sp_ctof(bbmin[0]);
			hitbox->bbmin.y = sp_ctof(bbmin[1]);
			hitbox->bbmin.z = sp_ctof(bbmin[2]);
			
			//Set Max Point
			hitbox->bbmax.x = sp_ctof(bbmax[0]);
			hitbox->bbmax.y = sp_ctof(bbmax[1]);
			hitbox->bbmax.z = sp_ctof(bbmax[2]);
			
			//Set Angle
			hitbox->angOffsetOrientation.x = sp_ctof(angle[0]);
			hitbox->angOffsetOrientation.y = sp_ctof(angle[1]);
			hitbox->angOffsetOrientation.z = sp_ctof(angle[2]);
			
			//Set Radius (0 for box)
			hitbox->flCapsuleRadius = sp_ctof(params[8]);
		}
		else
		{
			rootconsole->ConsolePrint("[HitboxChanger]: !!!Set Hitbox Failure!!! Bad HitBox %i", hitboxIndex);
			return 0;
		}
	}
	else
	{
		rootconsole->ConsolePrint("[HitboxChanger]: !!!Set Hitbox Failure!!! Bad HitBoxSet");
		return 0;
	}
	return 1;
}

cell_t GetHitbox(IPluginContext *pContext, const cell_t *params)
{	
	//Sourcepawn Native to follow:
	//native int GetHitbox(int modelIndex, int hitboxIndex, int &bone, int &group, float min[3], float max[3], float angle[3], float &radius);
	cell_t modelIndex = params[1];
	cell_t hitboxIndex = params[2];
	
	const model_t *test = g_pModelinfo->GetModel(modelIndex);
	studiohdr_t *mdl = g_pModelinfo->GetStudiomodel(test);
	mstudiohitboxset_t *hitboxset = mdl->pHitboxSet(0);
	
	if (hitboxset)
	{
		if(hitboxIndex < 0 || hitboxset->numhitboxes <= hitboxIndex)
		{
			rootconsole->ConsolePrint("[HitboxChanger]: !!!Get Hitbox Failure!!! Invalid Hitbox Index");
			return 0;
		}
		mstudiobbox_t *hitbox = hitboxset->pHitbox(hitboxIndex);
		if(hitbox)
		{
		
			cell_t *bone;
			pContext->LocalToPhysAddr(params[3], &bone);
			*bone = hitbox->bone;

			cell_t *group;
			pContext->LocalToPhysAddr(params[4], &group);
			*group = hitbox->group;
			
			cell_t *bbmin, *bbmax, *angle;
			pContext->LocalToPhysAddr(params[5], &bbmin);
			pContext->LocalToPhysAddr(params[6], &bbmax);
			pContext->LocalToPhysAddr(params[7], &angle);

			//Get Min Point
			bbmin[0] = sp_ftoc(hitbox->bbmin.x);
			bbmin[1] = sp_ftoc(hitbox->bbmin.y);
			bbmin[2] = sp_ftoc(hitbox->bbmin.z);
			
			//Get Max Point
			
			bbmax[0] = sp_ftoc(hitbox->bbmax.x);
			bbmax[1] = sp_ftoc(hitbox->bbmax.y);
			bbmax[2] = sp_ftoc(hitbox->bbmax.z);
			
			//Get Angle
			angle[0] = sp_ftoc(hitbox->angOffsetOrientation.x);
			angle[1] = sp_ftoc(hitbox->angOffsetOrientation.y);
			angle[2] = sp_ftoc(hitbox->angOffsetOrientation.z);
			
			//Set Radius (0 for box)
			cell_t *radius;
			pContext->LocalToPhysAddr(params[8], &radius);
			*radius = sp_ftoc(hitbox->flCapsuleRadius);
		}
		else
		{
			rootconsole->ConsolePrint("[HitboxChanger]: !!!Get Hitbox Failure!!! Bad HitBox %i", hitboxIndex);
			return 0;
		}
	}
	else
	{
		rootconsole->ConsolePrint("[HitboxChanger]: !!!Get Hitbox Failure!!! Bad HitBoxSet");
		return 0;
	}
	return 1;
}

cell_t SetNumHitboxes(IPluginContext *pContext, const cell_t *params)
{	
	cell_t modelIndex = params[1];
	
	const model_t *model = g_pModelinfo->GetModel(modelIndex);
	studiohdr_t *studiomodel = g_pModelinfo->GetStudiomodel(model);
	mstudiohitboxset_t *hitboxset = studiomodel->pHitboxSet(0);
	
	if (hitboxset)
	{
		hitboxset->numhitboxes = params[2];;
	}
	else
	{
		rootconsole->ConsolePrint("[HitboxChanger]: !!!Set Num Hitboxes Failure!!! Bad HitBoxSet");
		return 0;
	}
	return 1;
}

cell_t GetNumHitboxes(IPluginContext *pContext, const cell_t *params)
{	
	cell_t modelIndex = params[1];
	
	const model_t *model = g_pModelinfo->GetModel(modelIndex);
	studiohdr_t *studiomodel = g_pModelinfo->GetStudiomodel(model);
	mstudiohitboxset_t *hitboxset = studiomodel->pHitboxSet(0);
	
	if (hitboxset)
	{
		return hitboxset->numhitboxes;
	}
	else
	{
		rootconsole->ConsolePrint("[HitboxChanger]: !!!Get Num Hitboxes Failure!!! Bad HitBoxSet");
		return -1;
	}
}

cell_t FindBone(IPluginContext *pContext, const cell_t *params)
{
	char *boneName;
	pContext->LocalToString(params[1], &boneName);
	
	cell_t modelIndex = params[2];
	const model_t *model = g_pModelinfo->GetModel(modelIndex);
	studiohdr_t *studiomodel = g_pModelinfo->GetStudiomodel(model);
	
	//bones are a bitch im stealing the code instead of using the function
	
	// binary search for the bone matching boneName
	int start = 0, end = studiomodel->numbones-1;
	const byte *pBoneTable = studiomodel->GetBoneTableSortedByName();
	const mstudiobone_t *pbones = studiomodel->pBone( 0 );
	while (start <= end)
	{
		int mid = (start + end) >> 1;
		int cmp = Q_stricmp( pbones[pBoneTable[mid]].pszName(), boneName );
		
		if ( cmp < 0 )
		{
			start = mid + 1;
		}
		else if ( cmp > 0 )
		{
			end = mid - 1;
		}
		else
		{
			return pBoneTable[mid];
		}
	}
	return -1;
}

cell_t FindValidBones(IPluginContext *pContext, const cell_t *params)
{
	cell_t ModelIndex = params[1];
	const model_t *test = g_pModelinfo->GetModel(ModelIndex);
	studiohdr_t *mdl = g_pModelinfo->GetStudiomodel(test);
	
	if (mdl)
	{
		for (auto i = 0; i < mdl->numbones; i++)
		{
			const mstudiobone_t *bone = mdl->pBone(i);
			if (!(bone->flags & BONE_USED_BY_HITBOX))
				continue;
			rootconsole->ConsolePrint("ValidHitboxBone: %i, Name: %s, Flags: %i", i, bone->pszName(),bone->flags);
		}
	}
	else
	{
		rootconsole->ConsolePrint("[HitboxChanger]: !!!Find Valid Bones Failure!!! Bad StudioHDR");
		return 0;
	}
	return 1;
}

/* maybe this shit will work on windows but idk
static Vector	hullcolor[8] = 
{
	Vector( 1.0, 1.0, 1.0 ),
	Vector( 1.0, 0.5, 0.5 ),
	Vector( 0.5, 1.0, 0.5 ),
	Vector( 1.0, 1.0, 0.5 ),
	Vector( 0.5, 0.5, 1.0 ),
	Vector( 1.0, 0.5, 1.0 ),
	Vector( 0.5, 1.0, 1.0 ),
	Vector( 1.0, 1.0, 1.0 )
};


cell_t DrawServerHitboxes(IPluginContext *pContext, const cell_t *params)
{	
	cell_t modelIndex = params[1];
	cell_t duration = params[2];
	
	cell_t *origin;
	Vector pOrigin;
	pContext->LocalToPhysAddr(params[3], &origin);
	pOrigin.x = sp_ctof(origin[0]);
	pOrigin.y = sp_ctof(origin[1]);
	pOrigin.z = sp_ctof(origin[2]);
	
	cell_t *oAngles;
	QAngle pAngles;
	pContext->LocalToPhysAddr(params[4], &oAngles);
	pAngles.x = sp_ctof(oAngles[0]);
	pAngles.y = sp_ctof(oAngles[1]);
	pAngles.z = sp_ctof(oAngles[2]);
	
	const model_t *test = g_pModelinfo->GetModel(modelIndex);
	studiohdr_t *mdl = g_pModelinfo->GetStudiomodel(test);
	mstudiohitboxset_t *hitboxset = mdl->pHitboxSet(0);
	
	if (hitboxset)
	{
		Vector position;
		QAngle angles;

		int r = 0;
		int g = 0;
		int b = 255;

		for ( int i = 0; i < hitboxset->numhitboxes; i++ )
		{
			mstudiobbox_t *pbox = hitboxset->pHitbox( i );
			
			int j = (pbox->group % 8);
			
			r = ( int ) ( 255.0f * hullcolor[j][0] );
			g = ( int ) ( 255.0f * hullcolor[j][1] );
			b = ( int ) ( 255.0f * hullcolor[j][2] );

			if ( pbox->flCapsuleRadius > 0 )
			{
				matrix3x4_t temp;
				//GetHitboxBoneTransform( pbox->bone, pbox->angOffsetOrientation, temp );
				{
					matrix3x4_t bonetoworld;
					//GetBoneTransform( iBone, bonetoworld );
					{
						Vector bonePos = mdl->pBone(pbox->bone)->pos;
						Quaternion boneAngle = mdl->pBone(pbox->bone)->quat;
						matrix3x4_t bPos;
						//bPos.InitFromQuaternion(boneAngle, bonePos);
						QuaternionMatrix( boneAngle, bonePos, bPos );
						
						matrix3x4_t entPos;
						//entPos.InitFromQAngles(pAngles, pOrigin);
						AngleMatrix( pAngles, pOrigin, entPos );
						{
							bPos[ 0 ][ 0 ] += entPos[ 0 ][ 0 ]; bPos[ 0 ][ 1 ] += entPos[ 0 ][ 1 ]; bPos[ 0 ][ 2 ] += entPos[ 0 ][ 2 ]; bPos[ 0 ][ 3 ] += entPos[ 0 ][ 3 ];
							bPos[ 1 ][ 0 ] += entPos[ 1 ][ 0 ]; bPos[ 1 ][ 1 ] += entPos[ 1 ][ 1 ]; bPos[ 1 ][ 2 ] += entPos[ 1 ][ 2 ]; bPos[ 1 ][ 3 ] += entPos[ 1 ][ 3 ];
							bPos[ 2 ][ 0 ] += entPos[ 2 ][ 0 ]; bPos[ 2 ][ 1 ] += entPos[ 2 ][ 1 ]; bPos[ 2 ][ 2 ] += entPos[ 2 ][ 2 ]; bPos[ 2 ][ 3 ] += entPos[ 2 ][ 3 ];
						}
						// FIXME
						MatrixCopy( bPos, bonetoworld );
					}
					matrix3x4_t temp2;
					AngleMatrix( pbox->angOffsetOrientation, temp2);
					MatrixMultiply( bonetoworld, temp2, temp );
				}

				Vector vecCapsuleCenters[ 2 ];
				VectorTransform( pbox->bbmin, temp, vecCapsuleCenters[0] );
				VectorTransform( pbox->bbmax, temp, vecCapsuleCenters[1] );
				
				//NDebugOverlay::Capsule( vecCapsuleCenters[0], vecCapsuleCenters[1], pbox->flCapsuleRadius, r, g, b, 255, duration );
				g_pDebugOverlay->AddCapsuleOverlay( vecCapsuleCenters[0], vecCapsuleCenters[1], pbox->flCapsuleRadius, r, g, b, 255, duration );
			}
			else
			{
				//GetHitboxBonePosition( pbox->bone, position, angles, pbox->angOffsetOrientation );
				{
					matrix3x4_t bonetoworld;
					//GetBoneTransform( iBone, bonetoworld );
					{
						Vector bonePos = mdl->pBone(pbox->bone)->pos;
						Quaternion boneAngle = mdl->pBone(pbox->bone)->quat;
						matrix3x4_t bPos;
						//bPos.InitFromQuaternion(boneAngle, bonePos);
						QuaternionMatrix( boneAngle, bonePos, bPos );
						
						matrix3x4_t entPos;
						//entPos.InitFromQAngles(pAngles, pOrigin);
						AngleMatrix( pAngles, pOrigin, entPos );
						{
							bPos[ 0 ][ 0 ] += entPos[ 0 ][ 0 ]; bPos[ 0 ][ 1 ] += entPos[ 0 ][ 1 ]; bPos[ 0 ][ 2 ] += entPos[ 0 ][ 2 ]; bPos[ 0 ][ 3 ] += entPos[ 0 ][ 3 ];
							bPos[ 1 ][ 0 ] += entPos[ 1 ][ 0 ]; bPos[ 1 ][ 1 ] += entPos[ 1 ][ 1 ]; bPos[ 1 ][ 2 ] += entPos[ 1 ][ 2 ]; bPos[ 1 ][ 3 ] += entPos[ 1 ][ 3 ];
							bPos[ 2 ][ 0 ] += entPos[ 2 ][ 0 ]; bPos[ 2 ][ 1 ] += entPos[ 2 ][ 1 ]; bPos[ 2 ][ 2 ] += entPos[ 2 ][ 2 ]; bPos[ 2 ][ 3 ] += entPos[ 2 ][ 3 ];
						}
						// FIXME
						MatrixCopy( bPos, bonetoworld );
					}
					
					matrix3x4_t temp;
					AngleMatrix( pbox->angOffsetOrientation, temp);
					MatrixMultiply( bonetoworld, temp, temp );

					MatrixAngles( temp, angles, position );
				}
				//NDebugOverlay::BoxAngles( position, pbox->bbmin*GetModelHierarchyScale(), pbox->bbmax*GetModelHierarchyScale(), angles, r, g, b, 0 ,duration );
				g_pDebugOverlay->AddBoxOverlay(position, pbox->bbmin, pbox->bbmax, angles, r, g, b, 0, duration);
			}
		}
	}
	else
	{
		rootconsole->ConsolePrint("[HitboxChanger]: !!!Get Hitbox Failure!!! Bad HitBoxSet");
		return 0;
	}
	return 1;
}
*/

const sp_nativeinfo_t MyNatives[] = 
{
	{"HitboxInfo",		HitboxInfo},
	{"BoneInfo",		BoneInfo},
	{"SetHitbox",		SetHitbox},
	{"GetHitbox",		GetHitbox},
	{"SetNumHitboxes",	SetNumHitboxes},
	{"GetNumHitboxes",	GetNumHitboxes},
	{"FindBone",		FindBone},
	{"FindValidBones",	FindValidBones},
	//{"DrawServerHitboxes",	DrawServerHitboxes},
	{NULL,				NULL},
};

bool HitboxChanger::SDK_OnLoad(char *error, size_t maxlen, bool late)
{
	sharesys->AddNatives(myself, MyNatives);
	return true;
}

bool HitboxChanger::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	GET_V_IFACE_CURRENT(GetFileSystemFactory, g_pModelinfo, IVModelInfo, VMODELINFO_SERVER_INTERFACE_VERSION);
	//TODO: Try to see if this works on windows?
	//GET_V_IFACE_CURRENT(GetEngineFactory, g_pDebugOverlay, IVDebugOverlay, VDEBUG_OVERLAY_INTERFACE_VERSION);
	return true;
}








