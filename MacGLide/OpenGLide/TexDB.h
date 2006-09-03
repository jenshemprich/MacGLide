//**************************************************************
//*            OpenGLide - Glide to OpenGL Wrapper
//*             http://openglide.sourceforge.net
//*
//*                  TexDB Class Definition
//*
//*         OpenGLide is OpenSource under LGPL license
//*              Originaly made by Fabio Barros
//*      Modified by Paul for Glidos (http://www.glidos.net)
//*  Mac version and additional features by Jens-Olaf Hemprich
//**************************************************************

#ifndef __TEXDB_H__
#define __TEXDB_H__

	struct SubTexCoord_t
	{
		GLfloat smin;	// min max values rfor s & t
		GLfloat smax;
		GLfloat tmin;
		GLfloat tmax;
		GLfloat width;	// size in OpenGL tex coords
		GLfloat height;
		unsigned int left;	// Pixel origin of the sub texture 
		unsigned int top;
		unsigned int texImageWidth;	 // power of two
		unsigned int texImageHeight;    // power of two
	};

class TexDB  
{
public:
	enum TextureMode
	{
		One = 0,
		Two = 1,
		SubTextureOne = 2,
		SubTextureTwo = 3
	};
	
	struct SubRecordArray;
	struct SubRecord
	{
	protected:
		static SubRecord* s_FreeSubRecords;
		static SubRecordArray* s_SubRecordArrays;
		static void AllocSubRecords();
	public:
		SubRecord *next;
	public:
		GLuint texNum;
		GLuint tex2Num;
		unsigned int left;
		unsigned int top;
		unsigned int texImageWidth;
		unsigned int texImageHeight;
		SubRecord(TexDB::TextureMode texturemode);
		~SubRecord(void);
		inline bool Match(const SubTexCoord_t* subtexcoords) const
		{
			return (subtexcoords->smin >= left &&
			        subtexcoords->tmin >= top &&
			        subtexcoords->smax <= left + texImageWidth && 
			        subtexcoords->tmax <= top + texImageHeight);
		};
		inline bool IsInside(const SubTexCoord_t* subtexcoords) const
		{
			return (subtexcoords->left > left &&
			        subtexcoords->top > top &&
			        subtexcoords->left + subtexcoords->texImageWidth < left + texImageWidth && 
			        subtexcoords->top + subtexcoords->texImageHeight < top + texImageHeight);
		};
		void* operator new(size_t s);
		void operator delete(void* p);
		static void Init();
		static void Cleanup();
		static void initOpenGL();
		static void cleanupOpenGL();
	private:
		void* operator new[](size_t s); // not used so we don't need an implementation
		void operator delete[](void* p);
	};
	struct SubRecordArray
	{
		SubRecordArray* next;
		SubRecord subrecords[1];
	};

	struct RecordArray;
	class Record
	{
	protected:
		static Record* s_FreeRecords;
		static RecordArray* s_RecordArrays;
		static void AllocRecords();
	public:
		FxU32 startAddress;
		FxU32 endAddress;
		Record *next;
	    SubRecord *subrecords;
	public:
		GrTexInfo info;
		FxU32 hash;
		GLuint texNum;
		GLuint tex2Num;
		GLuint SClampMode;
		GLuint TClampMode;
		GLuint MinFilterMode;
		GLuint MagFilterMode;
	public:
		Record(TexDB::TextureMode texturemode);
		~Record(void);
		inline bool Match(FxU32 stt, const GrTexInfo *inf, FxU32 h) const
		{
			 return ((startAddress == stt) && 
			         (inf->largeLod == info.largeLod) && 
			         (inf->aspectRatio == info.aspectRatio) && 
			         (inf->format == info.format) && 
			         ((hash == h) || (h == 0)));
		}
		void WipeInside(const SubTexCoord_t* subtexcoords);
		void* operator new(size_t s);
		void operator delete(void* p);
		static void Init();
		static void Cleanup();
		static void initOpenGL();
		static void cleanupOpenGL();
	private:
		void* operator new[](size_t s); // not used so we don't need an implementation
		void operator delete[](void* p); // not used so we don't need an implementation
	};
	struct RecordArray
	{
		RecordArray* next;
		Record records[1];
	};
	TexDB(unsigned int MemorySize);
	virtual ~TexDB(void);
	void Clear(void);
	void initOpenGL();
	void cleanupOpenGL();
	Record* Add(FxU32 startAddress, FxU32 endAddress, const GrTexInfo *info, FxU32 hash, TexDB::TextureMode texturemode, const SubTexCoord_t* subtexcoords);
	Record* Find(FxU32 startAddress, const GrTexInfo *info, FxU32 hash, bool *pal_change, SubTexCoord_t* subtexcoords) const;
	void WipeRange(FxU32 startAddress, FxU32 endAddress, FxU32 hash);
	static int RecordArraySize;
protected:
	static int textureNamesCount;
private:
	unsigned int numberOfTexSections;
	Record** m_first;
	#ifdef OPTIMISE_TEXTURE_LOOKUP
		Record** m_texmem_2_record;
	#endif
};

#endif
