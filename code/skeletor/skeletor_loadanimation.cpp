/*
===========================================================================
Copyright (C) 2024 the OpenMoHAA team

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

// skeletor_loadanimation.cpp : Responsible of loading animations

#include "q_shared.h"
#include "qcommon.h"
#include "skeletor.h"

qboolean Compress(skelAnimGameFrame_t *current, skelAnimGameFrame_t *last, int channelIndex, int numChannels)
{
    float tolerance;
    float difference;
    int   i;

    // high-end PC don't need to compress...
    return false;

    for (i = 0; i < numChannels; i++) {
        tolerance = current->pChannels[channelIndex][i];
        if (tolerance > -0.000001 && tolerance < 0.000001) {
            current->pChannels[channelIndex][i] = 0.0f;
        }
    }

    if (last) {
        for (i = 0; i < numChannels; i++) {
            difference = last->pChannels[channelIndex][i] - current->pChannels[channelIndex][i];
            if (difference < -0.001f || difference >= 0.001f) {
                return false;
            }
        }
    }

    return true;
}

skelAnimDataGameHeader_t *EncodeFrames(
    skelAnimDataGameHeader_t *enAnim,
    skelAnimGameFrame_t      *m_frame,
    qboolean                  bLog,
    skelChannelList_c        *channelList,
    ChannelNameTable         *channelNames
)
{
    int                  endFrameCap;
    skanChannelHdr      *pChannel;
    int                  i, j;
    int                  frameCnt;
    skelAnimGameFrame_t *pCurrFrame;
    skelAnimGameFrame_t *pLastFrame;
    skanGameFrame       *pFrame;
    size_t               frameSize;
    int                  indexLastFrameAdded;
    channelType_t        channelType;

    pChannel    = enAnim->ary_channels;
    endFrameCap = enAnim->numFrames - 2;

    for (i = 0; i < enAnim->nTotalChannels; i++, pChannel++) {
        pLastFrame = NULL;
        pCurrFrame = m_frame;

        channelType = GetBoneChannelType(enAnim->channelList.ChannelName(channelNames, i));
        switch (channelType) {
        case CHANNEL_ROTATION:
            frameSize = (sizeof(skanGameFrame) - sizeof(skanGameFrame::pChannelData) + sizeof(vec4_t));
            frameCnt  = 0;
            for (j = 0; j < enAnim->numFrames; j++) {
                if (!Compress(pCurrFrame, pLastFrame, i, 4) || j >= endFrameCap) {
                    frameCnt++;
                    pLastFrame = pCurrFrame;
                }

                pCurrFrame++;
            }

            pChannel->ary_frames       = (skanGameFrame *)Skel_Alloc(frameSize * frameCnt);
            pChannel->nFramesInChannel = frameCnt;
            enAnim->nBytesUsed += frameSize * frameCnt;

            pLastFrame          = NULL;
            indexLastFrameAdded = 0;

            pCurrFrame = m_frame;
            pFrame     = pChannel->ary_frames;

            for (j = 0; j < enAnim->numFrames; j++) {
                if (!Compress(pCurrFrame, pLastFrame, i, 4) || j >= endFrameCap) {
                    pFrame                  = (skanGameFrame *)((byte *)pChannel->ary_frames + j * frameSize);
                    pFrame->nFrameNum       = j;
                    pFrame->nPrevFrameIndex = indexLastFrameAdded;

                    if (j > 0) {
                        indexLastFrameAdded++;
                    }

                    pFrame->pChannelData[0] = pCurrFrame->pChannels[i][0];
                    pFrame->pChannelData[1] = pCurrFrame->pChannels[i][1];
                    pFrame->pChannelData[2] = pCurrFrame->pChannels[i][2];
                    pFrame->pChannelData[3] = pCurrFrame->pChannels[i][3];

                    pLastFrame = pCurrFrame;
                }

                pCurrFrame++;
            }
            break;
        case CHANNEL_POSITION:
            frameSize = (sizeof(skanGameFrame) - sizeof(skanGameFrame::pChannelData) + sizeof(vec3_t));
            frameCnt  = 0;
            for (j = 0; j < enAnim->numFrames; j++) {
                if (!Compress(pCurrFrame, pLastFrame, i, 3) || j >= endFrameCap) {
                    frameCnt++;
                    pLastFrame = pCurrFrame;
                }

                pCurrFrame++;
            }

            pChannel->ary_frames       = (skanGameFrame *)Skel_Alloc(frameSize * frameCnt);
            pChannel->nFramesInChannel = frameCnt;
            enAnim->nBytesUsed += frameSize * frameCnt;

            pLastFrame          = NULL;
            indexLastFrameAdded = 0;

            pCurrFrame = m_frame;
            pFrame     = pChannel->ary_frames;

            for (j = 0; j < enAnim->numFrames; j++) {
                if (!Compress(pCurrFrame, pLastFrame, i, 1) || j >= endFrameCap) {
                    pFrame                  = (skanGameFrame *)((byte *)pChannel->ary_frames + j * frameSize);
                    pFrame->nFrameNum       = j;
                    pFrame->nPrevFrameIndex = indexLastFrameAdded;

                    if (j > 0) {
                        indexLastFrameAdded++;
                    }

                    pFrame->pChannelData[0] = pCurrFrame->pChannels[i][0];
                    pFrame->pChannelData[1] = pCurrFrame->pChannels[i][1];
                    pFrame->pChannelData[2] = pCurrFrame->pChannels[i][2];

                    pLastFrame = pCurrFrame;
                }

                pCurrFrame++;
            }
            break;
        case CHANNEL_NONE:
            pChannel->ary_frames       = NULL;
            pChannel->nFramesInChannel = 0;
            break;
        case CHANNEL_VALUE:
        default:
            frameSize = (sizeof(skanGameFrame) - sizeof(skanGameFrame::pChannelData) + sizeof(float));
            frameCnt  = 0;
            for (j = 0; j < enAnim->numFrames; j++) {
                if (!Compress(pCurrFrame, pLastFrame, i, 1) || j >= endFrameCap) {
                    frameCnt++;
                    pLastFrame = pCurrFrame;
                }

                pCurrFrame++;
            }

            pChannel->ary_frames       = (skanGameFrame *)Skel_Alloc(frameSize * frameCnt);
            pChannel->nFramesInChannel = frameCnt;
            enAnim->nBytesUsed += frameSize * frameCnt;

            pLastFrame          = NULL;
            indexLastFrameAdded = 0;

            pCurrFrame = m_frame;
            pFrame     = pChannel->ary_frames;

            for (j = 0; j < enAnim->numFrames; j++) {
                if (!Compress(pCurrFrame, pLastFrame, i, 3) || j >= endFrameCap) {
                    pFrame                  = (skanGameFrame *)((byte *)pChannel->ary_frames + j * frameSize);
                    pFrame->nFrameNum       = j;
                    pFrame->nPrevFrameIndex = indexLastFrameAdded;

                    if (j > 0) {
                        indexLastFrameAdded++;
                    }

                    pFrame->pChannelData[0] = pCurrFrame->pChannels[i][0];

                    pLastFrame = pCurrFrame;
                }

                pCurrFrame++;
            }
            break;
        }
    }

    return enAnim;
}

skelAnimDataGameHeader_t *
skeletor_c::ConvertSkelFileToGame(skelAnimDataFileHeader_t *pHeader, int iBuffLength, const char *path)
{
    int                       i, j;
    skelAnimFileFrame_t      *pFileFrame;
    skelAnimGameFrame_t      *pGameFrame;
    skelChannelName_t        *pChannelNames;
    skelAnimDataGameHeader_t *enAnim;
    skelAnimGameFrame_t      *oldFrame;
    skelAnimGameFrame_t      *newFrame;
    int                       channelIndex;

    static_assert(sizeof(skelAnimDataFileHeader_t) == 96, "Skeletor anim header size doesn't match!");
    static_assert(sizeof(skelAnimFileFrame_t) == 48, "Skeletor anim file frame size doesn't match!");

    if (pHeader->numFrames <= 0) {
        return NULL;
    }

    pGameFrame = new skelAnimGameFrame_t[pHeader->numFrames];
    pFileFrame = pHeader->frame;
    newFrame   = pGameFrame;

    for (i = 0; i < pHeader->numFrames; i++) {
        vec4_t *pChannels;

        newFrame->bounds[0][0] = LittleFloat(pFileFrame->bounds[0][0]);
        newFrame->bounds[0][1] = LittleFloat(pFileFrame->bounds[0][1]);
        newFrame->bounds[0][2] = LittleFloat(pFileFrame->bounds[0][2]);
        newFrame->bounds[1][0] = LittleFloat(pFileFrame->bounds[1][0]);
        newFrame->bounds[1][1] = LittleFloat(pFileFrame->bounds[1][1]);
        newFrame->bounds[1][2] = LittleFloat(pFileFrame->bounds[1][2]);
        newFrame->radius       = LittleFloat(pFileFrame->radius);
        newFrame->delta[0]     = LittleFloat(pFileFrame->delta[0]);
        newFrame->delta[1]     = LittleFloat(pFileFrame->delta[1]);
        newFrame->delta[2]     = LittleFloat(pFileFrame->delta[2]);
        newFrame->angleDelta   = LittleFloat(pFileFrame->angleDelta);
        //
        // Load channels
        //
        newFrame->pChannels = new vec4_t[pHeader->numChannels];
        pChannels =
            (vec4_t *)((byte *)pHeader
                       + (sizeof(skelAnimDataFileHeader_t) + sizeof(skelAnimFileFrame_t) * (pHeader->numFrames - 1)
                          + sizeof(vec4_t) * pHeader->numChannels * i));
        for (j = 0; j < pHeader->numChannels; j++) {
            newFrame->pChannels[j][0] = LittleFloat(pChannels[j][0]);
            newFrame->pChannels[j][1] = LittleFloat(pChannels[j][1]);
            newFrame->pChannels[j][2] = LittleFloat(pChannels[j][2]);
            newFrame->pChannels[j][3] = LittleFloat(pChannels[j][3]);
        }

        AddToBounds(newFrame->bounds, pFileFrame->bounds);

        pFileFrame++;
        newFrame++;
    }

    enAnim                  = skelAnimDataGameHeader_t::AllocRLEChannelData(pHeader->numChannels);
    enAnim->flags           = pHeader->flags;
    enAnim->frameTime       = pHeader->frameTime;
    enAnim->totalDelta      = pHeader->totalDelta;
    enAnim->totalAngleDelta = pHeader->totalAngleDelta;
    enAnim->numFrames       = pHeader->numFrames;
    enAnim->nTotalChannels  = pHeader->numChannels;
    ClearBounds(enAnim->bounds[0], enAnim->bounds[1]);
    enAnim->m_frame = (skelAnimGameFrame_t *)Skel_Alloc(pHeader->numFrames * sizeof(skelAnimGameFrame_t));

    oldFrame = pGameFrame;
    newFrame = enAnim->m_frame;

    for (i = 0; i < pHeader->numFrames; i++) {
        newFrame->bounds[0]  = oldFrame->bounds[0];
        newFrame->bounds[1]  = oldFrame->bounds[1];
        newFrame->radius     = oldFrame->radius;
        newFrame->delta      = oldFrame->delta;
        newFrame->angleDelta = oldFrame->angleDelta;
        newFrame->pChannels  = NULL;
        AddToBounds(enAnim->bounds, oldFrame->bounds);

        oldFrame++;
        newFrame++;
    }

    enAnim->channelList.ZeroChannels();

    pChannelNames = (skelChannelName_t *)((char *)pHeader + pHeader->ofsChannelNames);

    for (i = 0; i < pHeader->numChannels; i++) {
        channelIndex = m_channelNames.RegisterChannel(*pChannelNames);
        enAnim->channelList.AddChannel(channelIndex);
        pChannelNames++;
    }

    enAnim->channelList.PackChannels();
    EncodeFrames(enAnim, pGameFrame, qfalse, &enAnim->channelList, &m_channelNames);

    if (enAnim->channelList.HasChannel(&m_channelNames, "Bip01 pos")
        && enAnim->channelList.HasChannel(&m_channelNames, "Bip01 R Foot pos")
        && enAnim->channelList.HasChannel(&m_channelNames, "Bip01 L Foot pos")) {
        enAnim->bHasDelta = true;
    } else {
        enAnim->bHasDelta = false;
    }

    if (enAnim->channelList.HasChannel(&m_channelNames, "Bip01 Spine rot")
        && enAnim->channelList.HasChannel(&m_channelNames, "Bip01 Spine1 rot")) {
        enAnim->bHasUpper = true;
    } else {
        enAnim->bHasUpper = false;
    }

    if (enAnim->channelList.HasChannel(&m_channelNames, "VISEME_Bump")
        || enAnim->channelList.HasChannel(&m_channelNames, "VISME_Cage_")
        || enAnim->channelList.HasChannel(&m_channelNames, "VISME_Earth")
        || enAnim->channelList.HasChannel(&m_channelNames, "VISME_Fave")
        || enAnim->channelList.HasChannel(&m_channelNames, "VISME_If")
        || enAnim->channelList.HasChannel(&m_channelNames, "VISME_New")
        || enAnim->channelList.HasChannel(&m_channelNames, "VISME_Ox")
        || enAnim->channelList.HasChannel(&m_channelNames, "VISME_Roar")
        || enAnim->channelList.HasChannel(&m_channelNames, "VISME_Size")
        || enAnim->channelList.HasChannel(&m_channelNames, "VISME_Though")
        || enAnim->channelList.HasChannel(&m_channelNames, "VISME_Told")
        || enAnim->channelList.HasChannel(&m_channelNames, "VISME_Wet")
        || enAnim->channelList.HasChannel(&m_channelNames, "BROW_frown")
        || enAnim->channelList.HasChannel(&m_channelNames, "BROW_R_lift")
        || enAnim->channelList.HasChannel(&m_channelNames, "BROW_lift")
        || enAnim->channelList.HasChannel(&m_channelNames, "BROW_worry")
        || enAnim->channelList.HasChannel(&m_channelNames, "EYE_blink")
        || enAnim->channelList.HasChannel(&m_channelNames, "EYES_Excited__")
        || enAnim->channelList.HasChannel(&m_channelNames, "EYES_L_squint")
        || enAnim->channelList.HasChannel(&m_channelNames, "EYES_narrow__")
        || enAnim->channelList.HasChannel(&m_channelNames, "EYES_down")
        || enAnim->channelList.HasChannel(&m_channelNames, "EYES_left")
        || enAnim->channelList.HasChannel(&m_channelNames, "EYES_right")
        || enAnim->channelList.HasChannel(&m_channelNames, "EYES_smile")
        || enAnim->channelList.HasChannel(&m_channelNames, "EYES_up__")
        || enAnim->channelList.HasChannel(&m_channelNames, "JAW_open-closed")
        || enAnim->channelList.HasChannel(&m_channelNames, "JAW_open-open")
        || enAnim->channelList.HasChannel(&m_channelNames, "LIPS_compressed")
        || enAnim->channelList.HasChannel(&m_channelNames, "MOUTH_L_smile_closed")
        || enAnim->channelList.HasChannel(&m_channelNames, "MOUTH_L_smile_open")
        || enAnim->channelList.HasChannel(&m_channelNames, "MOUTH_L_snarl_closed_")
        || enAnim->channelList.HasChannel(&m_channelNames, "MOUTH_L_snarl_open")
        || enAnim->channelList.HasChannel(&m_channelNames, "MOUTH_grimace")
        || enAnim->channelList.HasChannel(&m_channelNames, "MOUTH_smile_closed")
        || enAnim->channelList.HasChannel(&m_channelNames, "MOUTH_smile_open")
        || enAnim->channelList.HasChannel(&m_channelNames, "MOUTH_Snarl_closed")
        || enAnim->channelList.HasChannel(&m_channelNames, "MOUTH_Snarl_open")) {
        enAnim->bHasMorph = true;
    } else {
        enAnim->bHasMorph = false;
    }

    for (i = 0; i < pHeader->numFrames; i++) {
        if (pGameFrame[i].pChannels) {
            delete[] pGameFrame[i].pChannels;
        }
    }

    delete[] pGameFrame;
    return enAnim;
}

void WriteEncodedFrames(msg_t *msg, skelAnimDataGameHeader_t *enAnim)
{
    skanChannelHdr *pChannel;
    skanGameFrame  *pFrame;
    int             i, j;

    MSG_WriteLong(msg, enAnim->numFrames);
    MSG_WriteShort(msg, enAnim->nTotalChannels);

    for (i = 0; i < enAnim->nTotalChannels; i++) {
        pChannel = &enAnim->ary_channels[i];
        MSG_WriteShort(msg, pChannel->nFramesInChannel);

        for (j = 0; j < pChannel->nFramesInChannel; j++) {
            vec4_t channelData;

            pFrame = &pChannel->ary_frames[i];
            MSG_WriteShort(msg, pFrame->nFrameNum);
            MSG_WriteShort(msg, pFrame->nPrevFrameIndex);

            channelData[0] = LittleFloat(pFrame->pChannelData[0]);
            channelData[1] = LittleFloat(pFrame->pChannelData[1]);
            channelData[2] = LittleFloat(pFrame->pChannelData[2]);
            channelData[3] = LittleFloat(pFrame->pChannelData[3]);
            MSG_WriteData(msg, channelData, sizeof(vec4_t));
        }
    }

    MSG_WriteLong(msg, enAnim->nBytesUsed);
}

void skeletor_c::SaveProcessedAnim(
    skelAnimDataGameHeader_t *enAnim, const char *path, skelAnimDataFileHeader_t *pHeader
)
{
    /*
	int i;
	skelChannelName_t *pChannelNames;
	msg_t msg;
	skelAnimGameFrame_t *newFrame;
	char npath[ 128 ];
	unsigned char buf[ 2000000 ];

	// FIXME:
	// Write animation
*/
}

void ReadEncodedFrames(msg_t *msg, skelAnimDataGameHeader_t *enAnim)
{
    skanChannelHdr *pChannel;
    skanGameFrame  *pFrame;
    size_t          frameSize;
    int             frameCnt;
    int             i, j;
    channelType_t   channelType;
    vec4_t          channelData;

    for (i = 0; i < enAnim->nTotalChannels; i++) {
        pChannel = &enAnim->ary_channels[i];

        channelType = GetBoneChannelType(enAnim->channelList.ChannelName(&skeletor_c::m_channelNames, i));
        frameCnt    = MSG_ReadShort(msg);

        switch (channelType) {
        case CHANNEL_ROTATION:
            frameSize = (sizeof(skanGameFrame) - sizeof(skanGameFrame::pChannelData) + sizeof(vec4_t));

            pFrame                     = (skanGameFrame *)Skel_Alloc(frameSize * frameCnt);
            pChannel->ary_frames       = pFrame;
            pChannel->nFramesInChannel = frameCnt;

            for (j = 0; j < frameCnt; j++) {
                pFrame                  = (skanGameFrame *)((byte *)pChannel->ary_frames + j * frameSize);
                pFrame->nFrameNum       = MSG_ReadShort(msg);
                pFrame->nPrevFrameIndex = MSG_ReadShort(msg);
                MSG_ReadData(msg, channelData, sizeof(vec4_t));

                pFrame->pChannelData[0] = LittleFloat(channelData[0]);
                pFrame->pChannelData[1] = LittleFloat(channelData[1]);
                pFrame->pChannelData[2] = LittleFloat(channelData[2]);
                pFrame->pChannelData[3] = LittleFloat(channelData[3]);
            }
            break;
        case CHANNEL_POSITION:
            frameSize = (sizeof(skanGameFrame) - sizeof(skanGameFrame::pChannelData) + sizeof(vec3_t));

            pFrame                     = (skanGameFrame *)Skel_Alloc(frameSize * frameCnt);
            pChannel->ary_frames       = pFrame;
            pChannel->nFramesInChannel = frameCnt;

            for (j = 0; j < frameCnt; j++) {
                pFrame                  = (skanGameFrame *)((byte *)pChannel->ary_frames + j * frameSize);
                pFrame->nFrameNum       = MSG_ReadShort(msg);
                pFrame->nPrevFrameIndex = MSG_ReadShort(msg);
                MSG_ReadData(msg, channelData, sizeof(vec4_t));

                pFrame->pChannelData[0] = LittleFloat(channelData[0]);
                pFrame->pChannelData[1] = LittleFloat(channelData[1]);
                pFrame->pChannelData[2] = LittleFloat(channelData[2]);
            }
            break;
        case CHANNEL_NONE:
            pChannel->ary_frames       = NULL;
            pChannel->nFramesInChannel = 0;

            for (j = 0; j < frameCnt; j++) {
                MSG_ReadShort(msg);
                MSG_ReadShort(msg);
                MSG_ReadData(msg, channelData, sizeof(vec4_t));
            }
            break;
        case CHANNEL_VALUE:
        default:
            frameSize = (sizeof(skanGameFrame) - sizeof(skanGameFrame::pChannelData) + sizeof(float));

            pFrame                     = (skanGameFrame *)Skel_Alloc(frameSize * frameCnt);
            pChannel->ary_frames       = pFrame;
            pChannel->nFramesInChannel = frameCnt;

            for (j = 0; j < frameCnt; j++) {
                pFrame                  = (skanGameFrame *)((byte *)pChannel->ary_frames + j * frameSize);
                pFrame->nFrameNum       = MSG_ReadShort(msg);
                pFrame->nPrevFrameIndex = MSG_ReadShort(msg);
                MSG_ReadData(msg, channelData, sizeof(vec4_t));

                pFrame->pChannelData[0] = LittleFloat(channelData[0]);
            }
            break;
        }
    }

    enAnim->nBytesUsed = MSG_ReadLong(msg);
}

void ReadEncodedFramesEx(msg_t *msg, skelAnimDataGameHeader_t *enAnim)
{
    skanChannelHdr *pChannel;
    skanGameFrame  *pFrame;
    size_t          frameSize;
    int             frameCnt;
    int             i, j;
    const char     *name;
    channelType_t   type;

    for (i = 0; i < enAnim->nTotalChannels; i++) {
        pChannel = &enAnim->ary_channels[i];

        name     = enAnim->channelList.ChannelName(&skeletor_c::m_channelNames, i);
        type     = GetBoneChannelType(name);
        frameCnt = MSG_ReadShort(msg);

        switch (type) {
        //
        // 4 channels (Quat)
        //
        case CHANNEL_ROTATION:
            frameSize                  = (sizeof(skanGameFrame) - sizeof(skanGameFrame::pChannelData) + sizeof(vec4_t));
            pFrame                     = (skanGameFrame *)Skel_Alloc(frameSize * frameCnt);
            pChannel->ary_frames       = pFrame;
            pChannel->nFramesInChannel = frameCnt;

            for (j = 0; j < pChannel->nFramesInChannel; j++) {
                pFrame                  = (skanGameFrame *)((byte *)pChannel->ary_frames + j * frameSize);
                pFrame->nFrameNum       = MSG_ReadShort(msg);
                pFrame->nPrevFrameIndex = MSG_ReadShort(msg);
                pFrame->pChannelData[0] = MSG_ReadFloat(msg);
                pFrame->pChannelData[1] = MSG_ReadFloat(msg);
                pFrame->pChannelData[2] = MSG_ReadFloat(msg);
                pFrame->pChannelData[3] = MSG_ReadFloat(msg);
            }
            break;
        //
        // 3 channels (Position)
        //
        case CHANNEL_POSITION:
            frameSize                  = (sizeof(skanGameFrame) - sizeof(skanGameFrame::pChannelData) + sizeof(vec3_t));
            pFrame                     = (skanGameFrame *)Skel_Alloc(frameSize * frameCnt);
            pChannel->ary_frames       = pFrame;
            pChannel->nFramesInChannel = frameCnt;

            for (j = 0; j < pChannel->nFramesInChannel; j++) {
                pFrame                  = (skanGameFrame *)((byte *)pChannel->ary_frames + j * frameSize);
                pFrame->nFrameNum       = MSG_ReadShort(msg);
                pFrame->nPrevFrameIndex = MSG_ReadShort(msg);
                pFrame->pChannelData[0] = MSG_ReadFloat(msg);
                pFrame->pChannelData[1] = MSG_ReadFloat(msg);
                pFrame->pChannelData[2] = MSG_ReadFloat(msg);
            }
            break;
        //
        // 1 channel (frame)
        //
        case CHANNEL_VALUE:
            frameSize            = (sizeof(skanGameFrame) - sizeof(skanGameFrame::pChannelData) + sizeof(float) * 1);
            pFrame               = (skanGameFrame *)Skel_Alloc(frameSize * frameCnt);
            pChannel->ary_frames = pFrame;
            pChannel->nFramesInChannel = frameCnt;

            for (j = 0; j < pChannel->nFramesInChannel; j++) {
                pFrame                  = (skanGameFrame *)((byte *)pChannel->ary_frames + j * frameSize);
                pFrame->nFrameNum       = MSG_ReadShort(msg);
                pFrame->nPrevFrameIndex = MSG_ReadShort(msg);
                pFrame->pChannelData[0] = MSG_ReadFloat(msg);
            }
            break;
        case CHANNEL_NONE:
            // Nothing to do
            break;
        }
    }
}

skelAnimDataGameHeader_t *skeletor_c::LoadProcessedAnim(const char *path, void *buffer, int len, const char *name)
{
    skelAnimDataGameHeader_t *enAnim;
    int                       i, j;
    msg_t                     msg;
    msg_t                     msgForAnim;
    int                       numChannels;
    skelAnimGameFrame_t      *newFrame;

    MSG_Init(&msg, (byte *)buffer, len);
    msg.cursize = len;
    MSG_BeginReading(&msg);

    numChannels             = MSG_ReadLong(&msg);
    enAnim                  = skelAnimDataGameHeader_t::AllocRLEChannelData(numChannels);
    enAnim->flags           = MSG_ReadLong(&msg);
    enAnim->frameTime       = MSG_ReadFloat(&msg);
    enAnim->totalDelta[0]   = MSG_ReadFloat(&msg);
    enAnim->totalDelta[1]   = MSG_ReadFloat(&msg);
    enAnim->totalDelta[2]   = MSG_ReadFloat(&msg);
    enAnim->totalAngleDelta = MSG_ReadFloat(&msg);
    enAnim->numFrames       = MSG_ReadLong(&msg);
    enAnim->nTotalChannels  = MSG_ReadShort(&msg);
    enAnim->bHasDelta       = MSG_ReadByte(&msg) != 0;
    enAnim->bHasUpper       = MSG_ReadByte(&msg) != 0;
    enAnim->bHasMorph       = MSG_ReadByte(&msg) != 0;

    newFrame        = (skelAnimGameFrame_t *)Skel_Alloc(enAnim->numFrames * sizeof(skelAnimGameFrame_t));
    enAnim->m_frame = newFrame;

    for (i = 0; i < enAnim->numFrames; i++) {
        newFrame->bounds[0][0] = MSG_ReadFloat(&msg);
        newFrame->bounds[0][1] = MSG_ReadFloat(&msg);
        newFrame->bounds[0][2] = MSG_ReadFloat(&msg);
        newFrame->bounds[1][0] = MSG_ReadFloat(&msg);
        newFrame->bounds[1][1] = MSG_ReadFloat(&msg);
        newFrame->bounds[1][2] = MSG_ReadFloat(&msg);
        newFrame->radius       = MSG_ReadFloat(&msg);
        newFrame->delta[0]     = MSG_ReadFloat(&msg);
        newFrame->delta[1]     = MSG_ReadFloat(&msg);
        newFrame->delta[2]     = MSG_ReadFloat(&msg);
        newFrame->angleDelta   = MSG_ReadFloat(&msg);
        newFrame->pChannels    = NULL;
        newFrame++;
    }

    enAnim->bounds[0][0] = MSG_ReadFloat(&msg);
    enAnim->bounds[0][1] = MSG_ReadFloat(&msg);
    enAnim->bounds[0][2] = MSG_ReadFloat(&msg);
    enAnim->bounds[1][0] = MSG_ReadFloat(&msg);
    enAnim->bounds[1][1] = MSG_ReadFloat(&msg);
    enAnim->bounds[1][2] = MSG_ReadFloat(&msg);

    enAnim->numFrames      = MSG_ReadLong(&msg);
    enAnim->nTotalChannels = MSG_ReadShort(&msg);

    msgForAnim = msg;

    for (i = 0; i < enAnim->nTotalChannels; i++) {
        int    frameCnt = MSG_ReadShort(&msgForAnim);
        vec4_t channelData;

        for (j = 0; j < frameCnt; j++) {
            MSG_ReadShort(&msgForAnim);
            MSG_ReadShort(&msgForAnim);
            MSG_ReadData(&msgForAnim, channelData, sizeof(vec4_t));
        }
    }

    enAnim->nBytesUsed = MSG_ReadLong(&msgForAnim);

    numChannels = MSG_ReadLong(&msgForAnim);
    enAnim->channelList.ZeroChannels();

    for (i = 0; i < numChannels; i++) {
        const char *name = MSG_ReadString(&msgForAnim);

        if (enAnim->channelList.AddChannel(m_channelNames.RegisterChannel(name)) != i) {
            Com_Printf("^~^~^ Animation '%s' has duplicate channel '%s'\n", path, name);
        }
    }

    ReadEncodedFrames(&msg, enAnim);
    return enAnim;
}

skelAnimDataGameHeader_t *skeletor_c::LoadProcessedAnimEx(const char *path, void *buffer, int len, const char *name)
{
    skelAnimDataGameHeader_t *enAnim;
    int                       i;
    msg_t                     msg;
    int                       numChannels;
    skelAnimGameFrame_t      *newFrame;

    MSG_Init(&msg, (byte *)buffer, len);
    msg.cursize = len;
    MSG_BeginReading(&msg);

    numChannels = MSG_ReadShort(&msg);
    enAnim      = skelAnimDataGameHeader_t::AllocRLEChannelData(numChannels);
    enAnim->channelList.ZeroChannels();
    enAnim->flags           = MSG_ReadLong(&msg);
    enAnim->frameTime       = MSG_ReadFloat(&msg);
    enAnim->totalDelta[0]   = MSG_ReadFloat(&msg);
    enAnim->totalDelta[1]   = MSG_ReadFloat(&msg);
    enAnim->totalDelta[2]   = MSG_ReadFloat(&msg);
    enAnim->totalAngleDelta = MSG_ReadFloat(&msg);
    enAnim->numFrames       = MSG_ReadLong(&msg);
    enAnim->bHasDelta       = MSG_ReadByte(&msg) != 0;
    enAnim->bHasUpper       = MSG_ReadByte(&msg) != 0;
    enAnim->bHasMorph       = MSG_ReadByte(&msg) != 0;

    for (i = 0; i < enAnim->nTotalChannels; i++) {
        enAnim->channelList.AddChannel(m_channelNames.RegisterChannel(MSG_ReadString(&msg)));
    }

    newFrame        = (skelAnimGameFrame_t *)Skel_Alloc(enAnim->numFrames * sizeof(skelAnimGameFrame_t));
    enAnim->m_frame = newFrame;

    for (i = 0; i < enAnim->numFrames; i++) {
        newFrame->bounds[0][0] = MSG_ReadFloat(&msg);
        newFrame->bounds[0][1] = MSG_ReadFloat(&msg);
        newFrame->bounds[0][2] = MSG_ReadFloat(&msg);
        newFrame->bounds[1][0] = MSG_ReadFloat(&msg);
        newFrame->bounds[1][1] = MSG_ReadFloat(&msg);
        newFrame->bounds[1][2] = MSG_ReadFloat(&msg);
        newFrame->radius       = MSG_ReadFloat(&msg);
        newFrame->delta[0]     = MSG_ReadFloat(&msg);
        newFrame->delta[1]     = MSG_ReadFloat(&msg);
        newFrame->delta[2]     = MSG_ReadFloat(&msg);
        newFrame->angleDelta   = MSG_ReadFloat(&msg);
        newFrame->pChannels    = NULL;
        newFrame++;
    }

    enAnim->bounds[0][0] = MSG_ReadFloat(&msg);
    enAnim->bounds[0][1] = MSG_ReadFloat(&msg);
    enAnim->bounds[0][2] = MSG_ReadFloat(&msg);
    enAnim->bounds[1][0] = MSG_ReadFloat(&msg);
    enAnim->bounds[1][1] = MSG_ReadFloat(&msg);
    enAnim->bounds[1][2] = MSG_ReadFloat(&msg);
    ReadEncodedFramesEx(&msg, enAnim);

    return enAnim;
}
