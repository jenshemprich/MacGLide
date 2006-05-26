//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*            implementation of the TexDB class
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#include "Glide.h"
#include "TexDB.h"

TexDB::Record* TexDB::Record::s_FreeRecords = NULL;
TexDB::RecordArray* TexDB::Record::s_RecordArrays = NULL;
TexDB::SubRecord* TexDB::SubRecord::s_FreeSubRecords = NULL;
int TexDB::RecordArraySize = 16;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TexDB::TexDB(unsigned int MemorySize)
{
	numberOfTexSections = MemorySize >> 15; // ( 32 * 1024 );
	m_first = new Record*[ numberOfTexSections ];
	for ( unsigned int i = 0; i < numberOfTexSections; i++ )
	{
		m_first[ i ] = NULL;
	}
	#ifdef OPTIMISE_TEXTURE_LOOKUP
		// Allocate MemorySize / 2 record pointers, because
		// a Glide texture can only be downloaded to 8-byte boundaries
		m_texmem_2_record = static_cast<Record**>(AllocBuffer(MemorySize >> 3, sizeof(Record*)));
		memset(m_texmem_2_record, 0, (MemorySize >> 3) * sizeof(Record*));
	#endif
	// Prealloc Reocrds proportional to the amount of tex memory
	RecordArraySize = MemorySize / (1024 * 1024) * 256; // preallocate 256 records per mb  
	// Prealloc some texdb records to avoid out of memory problems later
	TexDB::Record::Init();
}

TexDB::~TexDB( void )
{
	Clear();
	delete[] m_first;
	TexDB::SubRecord::Cleanup();
	#ifdef OPTIMISE_TEXTURE_LOOKUP
		if (m_texmem_2_record) FreeBuffer(m_texmem_2_record);
	#endif
}

TexDB::Record* TexDB::Find(FxU32 startAddress, const GrTexInfo *info, FxU32 hash, bool *pal_change, SubTexCoord_t* subtexcoords) const
{
	// Perform normal search
	FxU32 sect = startAddress >> 15; // ( 32 * 1024 );
	Record* r;
#ifdef OPTIMISE_TEXTURE_LOOKUP
	// We can start searching with the record found via the lookup,
	// because it's the one added last to this address,
	// and records are added to the front of the linked list
	r = m_texmem_2_record[startAddress >> 3];
	// If the record has a palette hash,
	// then we must find the right paletted texture
	if (r == NULL || hash != r->hash)
	{
		if (r)
		{
			// Can start from the next record, because we've
			// just checked that the current doesn't match
			r = r->next;
		}
		else
		{
			// lookup empty, search from the beginning
			r = m_first[sect];
		}
#else
		r = m_first[sect];
#endif
		for (; r; r = r->next)
		{
			if (r->Match(startAddress, info, (pal_change == NULL) ? hash : 0))
			{
				#ifdef OGL_UTEX
					GlideMsg("Found tex %d\n", r->texNum);
				#endif
				break;
			}
		}
#ifdef OPTIMISE_TEXTURE_LOOKUP
	}
#ifdef OGL_UTEX
	else if (r)
	{
		GlideMsg("Found texmem_2_record tex %d\n", r->texNum);
	}
#endif
#endif
	if (r)
	{
		if (subtexcoords)
		{
#ifdef OGL_UTEX
	GlideMsg("Looking for subtexture: ");
	GlideMsg("min/max(%g,%g)-(%g,%g), ",
           subtexcoords->smin,
           subtexcoords->smax,
           subtexcoords->tmin,
           subtexcoords->tmax);
	GlideMsg("w/h(%g,%g), texture(%d,%d)-(%d,%d)\n",
           subtexcoords->width,
           subtexcoords->height,
           subtexcoords->left,
           subtexcoords->top,
           subtexcoords->texImageWidth,
           subtexcoords->texImageHeight);
#endif
			// Lookup subrecord
			SubRecord* s;
			for (s = r->subrecords; s != NULL; s = s->next)
			{
				if (s->Match(subtexcoords))
				{
					// bind textures
					r->texNum = s->texNum;
					r->tex2Num = s->tex2Num;
					// The one we've found might have a greater texture than the one
					//  we're looking for so we have to update the size
					subtexcoords->width = s->left;
					subtexcoords->height = s->top;
					subtexcoords->smin = s->left;
					subtexcoords->tmin = s->top;
					subtexcoords->texImageWidth = s->texImageWidth;
					subtexcoords->texImageHeight = s->texImageHeight;
					#ifdef OGL_UTEX
						GlideMsg("Found subtex %d\n", s->texNum);
					#endif
					break;
				}
			}
			if (s == NULL)
			{
#ifdef OGL_UTEX
	GlideMsg("Removing smaller subtextures in tex_record %d\n", r->texNum);
#endif
				// remove any smaller version of the subtextures to avoid artefacts on snow tiles
				// This is ok because we're about to create a new texture that contains all those
				// smaller textures we're going to wipe
				r->WipeInside(subtexcoords);
				r = NULL;
			}
		}
		// Palette changed?
		if (( pal_change != NULL) && (r->hash != hash ))
		{
			r->hash = hash;
			*pal_change = true;
		}
	}
	return r;
}

void TexDB::WipeRange(FxU32 startAddress, FxU32 endAddress, FxU32 hash)
{
	Record** p;
	FxU32 stt_sect = startAddress >> 15; // ( 32 * 1024 );
	/*
	* Textures can be as large as 128K, so
	* one that starts 3 sections back can
	* extend into this one.
	*/
	if (stt_sect < 4)
	{
		stt_sect = 0;
	}
	else
	{
		stt_sect -= 4;
	}
	FxU32 end_sect = endAddress >> 15; // ( 32 * 1024 );
	if (end_sect >= numberOfTexSections)
	{
		end_sect = numberOfTexSections - 1;
	}
	for(FxU32 i = stt_sect; i <= end_sect; i++)
	{
		p = &(m_first[i]);
		while (*p != NULL)
		{
			Record* r = *p;
			if ((startAddress < r->endAddress) && 
			    (r->startAddress < endAddress) && 
			    ((hash == 0) || (r->hash == hash)))
			{
				*p = r->next;
#ifdef OPTIMISE_TEXTURE_LOOKUP
					m_texmem_2_record[(r->startAddress) >> 3] = NULL;
#endif
				delete r;
#ifdef OGL_UTEX
				if (hash)
				{
					GlideMsg("Wipe tex %d, hash 0x%x\n", r->texNum, r->hash);
				}
				else
				{
					GlideMsg("Wipe tex %d\n", r->texNum);
				}
#endif
			}
			else
			{
				p = &(r->next);
			}
		}
	}
}

TexDB::Record* TexDB::Add(FxU32 startAddress, FxU32 endAddress, const GrTexInfo *info, FxU32 hash, TexDB::TextureMode texturemode, const SubTexCoord_t* subtexcoords)
{
	Record  *r = NULL;
	// Check for existing record
	if (subtexcoords)
	{
#ifdef OPTIMISE_TEXTURE_LOOKUP
		// lookup record
		r = m_texmem_2_record[startAddress >> 3];
		// needed by TR 1 Gold to work correctly
		if (r == NULL || r->Match( startAddress, info, 0) == false)
		{
#endif
			// @todo: don't do it twice (here and in Find())
			FxU32 sect = startAddress >> 15; // ( 32 * 1024 );
			for (r = m_first[ sect ]; r != NULL; r = r->next )
			{
				if ( r->Match( startAddress, info, 0 /*( pal_change == NULL ) ? hash : 0 )*/ ))
				{
					break;
				}
			}
#ifdef OPTIMISE_TEXTURE_LOOKUP
		}
#endif
	}
	else
	{
#ifdef OGL_UTEX
#ifdef OPTIMISE_TEXTURE_LOOKUP
		// lookup record
		r = m_texmem_2_record[startAddress >> 3];
		if (r)
		{
			GlideMsg("Warning: Existing TexRecord must be wiped first%d\n", r->texNum);
			return NULL;
		}
#endif
#endif
	}
	// Create new record?
	if (r == NULL)
	{
		r = new Record(texturemode);
		FxU32 sect = startAddress >> 15; // 32 * 1024
		r->startAddress = startAddress;
		r->endAddress = endAddress;
		r->info = *info;
		r->hash = hash;
		r->next = m_first[sect];
		m_first[sect] = r;
		r->subrecords = NULL;
#ifdef OPTIMISE_TEXTURE_LOOKUP
		// lookup record 
		m_texmem_2_record[startAddress >> 3] = r;
#endif
#ifdef OGL_UTEX
		GlideMsg("Add tex %d\n", r->texNum);
#endif
	}
	if (subtexcoords)
	{
		SubRecord* s = new SubRecord(texturemode);
		s->left = subtexcoords->left;
		s->top = subtexcoords->top;
		s->texImageWidth = subtexcoords->texImageWidth;
		s->texImageHeight = subtexcoords->texImageHeight;
		s->next = r->subrecords;
		r->subrecords = s;
		// The caller always queries r for texture names, thus we have to copy
		// Note: For debugging the caching efficency, comment out the assignment
		r->texNum = s->texNum;
		r->tex2Num = s->tex2Num;
#ifdef OGL_UTEX
	    GlideMsg("Add subtex %d\n", s->texNum);
#endif
	}

	return r;
}

void TexDB::Clear( void )
{
	Record* r;
	for ( unsigned int i = 0; i < numberOfTexSections; i++ )
	{
		r = m_first[ i ];
		while ( r != NULL )
		{
			Record *tmp = r;
			r = r->next;
			#ifdef OPTIMISE_TEXTURE_LOOKUP
				// lookup record
				m_texmem_2_record[(tmp->startAddress) >> 3] = NULL;
			#endif
			delete tmp;
		}
		m_first[ i ] = NULL;
	}
}

///////////////////////////////////////////////////////////////////
// TexDB::Record Class implementation
///////////////////////////////////////////////////////////////////

void TexDB::Record::Init()
{
	// Alloc a couple of records in advance
	AllocRecords();
}

void TexDB::Record::Cleanup()
{
	RecordArray* ra = s_RecordArrays;
	while (ra)
	{
		RecordArray* next = ra->next;
		FreeObject(ra);
#ifdef OGL_UTEX_MEM
	GlideMsg("Freed TexDB record array at 0x%x\n", ra);
#endif
		ra = next;
	}
	s_RecordArrays = NULL;
	s_FreeRecords = NULL;
	return;	
}

void TexDB::Record::AllocRecords()
{
#ifdef OGL_UTEX_MEM
	GlideMsg("Allocating TexDB record array with %d entries\n", RecordArraySize);
#endif
	RecordArray* next = s_RecordArrays ? s_RecordArrays->next : NULL;
	s_RecordArrays = static_cast<RecordArray*>(AllocObject(sizeof(RecordArray) + (RecordArraySize -1) * sizeof(Record)));
	s_RecordArrays->next = next;
	for(int i = 0; i < 	RecordArraySize; i++)
	{
		s_RecordArrays->records[i].next = s_FreeRecords;
		s_FreeRecords = &s_RecordArrays->records[i];
	}
}

void* TexDB::Record::operator new(size_t s)
{
	Record* p;
	if (s_FreeRecords == NULL)
	{
		AllocRecords();
	}
	p = s_FreeRecords;
	s_FreeRecords = p->next;
#ifdef OGL_UTEX_MEM
	GlideMsg("Using TexDB record at 0x%x\n", p);
#endif
	return p;
}

void TexDB::Record::operator delete(void* p)
{
#ifdef OGL_UTEX_MEM
	GlideMsg("Moving TexDB record at 0x%x to free list\n", p);
#endif
	reinterpret_cast<TexDB::Record*>(p)->next = s_FreeRecords;
	s_FreeRecords = reinterpret_cast<TexDB::Record*>(p);
}

TexDB::Record::Record(TexDB::TextureMode texturemode)
{
	if (texturemode <= Two)
	{
		GLfloat one; 
		glGenTextures(1, &texNum);
		glPrioritizeTextures(1, &texNum, &one);
		if (texturemode == TexDB::Two )
		{
			glGenTextures(1, &tex2Num);
			glPrioritizeTextures(1, &tex2Num, &one);
		}
		else
		{
			tex2Num = 0;
		}
	}
}

TexDB::Record::~Record(void)
{
	if (subrecords == NULL)
	{
		glDeleteTextures(1, &texNum);
		if (tex2Num != 0)
		{
			glDeleteTextures(1, &tex2Num);
		}
	}
	else
	{
		SubRecord* next = this->subrecords;
		for (SubRecord* s = next; next != NULL; s = next)
		{
			next = s->next;
			delete s;
		}
	}   
}

void TexDB::Record::WipeInside(const SubTexCoord_t* subtexcoords)
{
	SubRecord** head = &subrecords;
	SubRecord* s = *head;
	while (s)
	{
		if (s->IsInside(subtexcoords))
		{
#ifdef OGL_UTEX
	GlideMsg("Removing subtexture %d: ", s->texNum);
	GlideMsg("texture(%d,%d)-(%d,%d)\n",
	         s->left,
	         s->top,
	         s->texImageWidth,
	         s->texImageHeight);
#endif
			*head = s->next;
			delete s;
			s = *head;
		}
		else
		{
			head = &s->next;
			s = s->next;
		}
	}
}

///////////////////////////////////////////////////////////////////
// TexDB::SubRecord Class implementation
///////////////////////////////////////////////////////////////////

TexDB::SubRecord::SubRecord(TexDB::TextureMode texturemode)
{
	glGenTextures(1, &texNum);
	if (texturemode == TexDB::SubTextureTwo)
	{
		glGenTextures(1, &tex2Num);
	}
	else
	{
		 tex2Num = 0;
	}
}

TexDB::SubRecord::~SubRecord( void )
{
	glDeleteTextures(1, &texNum);
	if (tex2Num != 0)
	{
		glDeleteTextures(1, &tex2Num);
	}
}

void* TexDB::SubRecord::operator new(size_t s)
{
	SubRecord* p;
	if (s_FreeSubRecords == NULL)
	{
		p = reinterpret_cast<SubRecord*>(NewPtrSys(s));
#ifdef OGL_UTEX_MEM
	GlideMsg("Allocated TexDB subrecord at 0x%x\n", p);
#endif
	}
	else
	{
		p = s_FreeSubRecords;
		s_FreeSubRecords = p->next;
#ifdef OGL_UTEX_MEM
	GlideMsg("Using TexDB subrecord at 0x%x\n", p);
#endif
	}
	return p;
}

void TexDB::SubRecord::operator delete(void* p)
{
#ifdef OGL_UTEX_MEM
	GlideMsg("Moving TexDB subrecord at 0x%x to free-list\n", p);
#endif
//	reinterpret_cast<TexDB::SubRecord*>(p)->next = s_FreeSubRecords;
	SubRecord*  s = reinterpret_cast<TexDB::SubRecord*>(p);
	s->next = s_FreeSubRecords;
	s_FreeSubRecords = s;
}

void TexDB::SubRecord::Cleanup()
{
	SubRecord* s = s_FreeSubRecords;
	while (s)
	{
#ifdef OGL_UTEX_MEM
	GlideMsg("Deleting TexDB subrecord at 0x%x\n", s);
#endif
		SubRecord* next = s->next;
		DisposePtr(reinterpret_cast<Ptr>(s));
		s = next;
	}
	s_FreeSubRecords = NULL;
	return;	
}
