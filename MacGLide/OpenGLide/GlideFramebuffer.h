//**************************************************************
//*      OpenGLide for Macintosh - Glide to OpenGL Wrapper
//*             http://macglide.sourceforge.net/
//*
//*               Glide framebuffer emulation
//*
//*         OpenGLide is OpenSource under LGPL license
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#pragma once

#include "Framebuffer.h"

#define  GR_LFBWRITEMODE_UNUSED (GR_LFBWRITEMODE_ANY -1)

class GlideFramebuffer : protected Framebuffer
{
public:
	GlideFramebuffer();
	~GlideFramebuffer();
protected:
	bool m_write_opaque;
	bool m_must_write;
	bool m_renderbuffer_changed;
	bool m_renderbuffer_changed_for_read;
	GLfloat m_depth;
	FxU32 m_alpha;
	bool m_chromakeyvalue_changed;
	FxU16 m_chromakeyvalue_new;
	GrLfbWriteMode_t m_grLfbLockWriteMode[6];
	unsigned int m_bufferclearcalls;
	static const tilesize tilesize_table_carmageddon[11];
public:
	void initialise(BufferStruct* framebuffer, BufferStruct* texbuffer);
	void OnBufferLockStartWrite(GrLock_t dwType, GrBuffer_t dwBuffer, GrLfbWriteMode_t dwWriteMode, GrOriginLocation_t dwOrigin, FxBool bPixelPipeline);
	void OnBufferUnlockEndWrite(GrBuffer_t dwBuffer);
	// void OnBufferNumPending();
	void OnBeforeBufferClear();
	void OnLfbReadRegion();
	void OnRenderDrawTriangles();
	void OnBeforeBufferSwap();
	void OnAfterBufferSwap();
	// void OnSetIdle();
	void OnClipWindow();
	void OnChromaKeyValueChanged();
	void Cleanup();
	void SetAlpha(FxU32 alpha);
	void SetDepth(GLfloat depth);
	FxU16 GetChromaKeyValue16(GrColor_t chromakeyvalue);
	inline void SetRenderBufferChanged() {m_renderbuffer_changed = m_renderbuffer_changed_for_read = true;};
	inline bool GetRenderBufferChanged() const {return m_renderbuffer_changed;};
	inline bool GetRenderBufferChangedForRead() const {return m_renderbuffer_changed_for_read;};
	inline void ResetRenderBufferChangedForRead() {m_renderbuffer_changed_for_read = false;};
	void CopyFrameBuffer(FxU16* targetbuffer);
	inline FxU16 GetChromaKeyValue() {return m_ChromaKey;};
protected:
	void WriteFrameBuffer(bool pixelpipline);
	inline bool BackBufferIsLocked() {return m_grLfbLockWriteMode[GR_BUFFER_BACKBUFFER] != GR_LFBWRITEMODE_UNUSED;};
};
