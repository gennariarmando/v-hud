#include "VHud.h"
#include "TextureMgr.h"
#include "Utility.h"

int texturesCount = 0;
CTextureRel* CTextureMgr::pTex[1024];

CTextureMgr textureMgr;

CTextureMgr::CTextureMgr() {

}

void CTextureMgr::Init() {
	texturesCount = 0;
}

void CTextureMgr::Shutdown() {
	for (int i = 0; i < texturesCount; i++) {
		CTextureRel* t = pTex[i];

		if (t) {
			t->texture = NULL;
			delete t;
		}
	}

	texturesCount = 0;
}

void CTextureMgr::ReloadTextures() {
	int c = texturesCount;
	texturesCount = 0;
	for (int i = 0; i < c; i++) {
		CTextureRel* t = pTex[i];
		if (t) {
			if (t->texture) {
				RwTextureDestroy(t->texture);
				t->texture = CTextureMgr::LoadPNGTextureCB(t->path, t->name);
			}
		}
	}
}

void CTextureMgr::LimitTextureSize(int& w, int& h) {
	if (RsGlobal.maximumWidth == 0 || RsGlobal.maximumHeight == 0)
		return;

	float lx = RsGlobal.maximumWidth / 100;
	float ly = RsGlobal.maximumHeight / 100;

	int _w = w;
	int _h = h;

	if (ly > 0.0f && ly <= 5.0f) {
		_w = 512;
		_h = 512;
	}
	else if (ly > 5.0f && ly <= 7.0f) {
		_w = 512;
		_h = 512;
	}
	else if (ly > 7.0f && ly <= 10.0f) {
		_w = 1024;
		_h = 1024;
	}
	else if (ly > 10.0f && ly <= 20.0f) {
		_w = 2048;
		_h = 2048;
	}
	else if (ly > 20.0f) {
		_w = 4096;
		_h = 4096;
	}

	w = clamp(w, 0, _w);
	h = clamp(h, 0, _h);
}

RwTexture* CTextureMgr::LoadPNGTextureCB(const char *path, const char* name) {
	int w, h, d, f;
	char file[512];

	strcpy_s(file, path);
	strcat_s(file, "\\");
	strcat_s(file, name);
	strcat_s(file, ".png");
	puts(file);

	if (file && FileCheck(file)) {
		RwImage* img = RtPNGImageRead(file);
		RwImageFindRasterFormat(img, rwRASTERTYPETEXTURE, &w, &h, &d, &f);

		// Little hack to ensure visibility
		LimitTextureSize(w, h);

		RwRaster* raster = RwRasterCreate(w, h, d, f);
		RwRasterSetFromImage(raster, img);

		RwImageDestroy(img);

		RwTexture* texture = RwTextureCreate(raster);
		strcpy_s(texture->name, 0x20u, name);

		RwTextureSetFilterMode(texture, rwFILTERLINEARMIPLINEAR);

		//pTex[texturesCount] = new CTextureRel(texture, path, name);
		//texturesCount++;
		return texture;
	}
	return NULL;
}

RwTexture* CTextureMgr::LoadPNGTextureCB(const char *path, const char* name, const char* namea) {
	int w, h, d, f;
	char file[512];
	char maskFile[512];
	char colFile[512];

	strcpy_s(file, path);
	strcat_s(file, "\\");
	strcat_s(file, name);
	strcat_s(file, ".png");
	puts(file);

	strcpy_s(maskFile, path);
	strcat_s(maskFile, "\\");
	strcat_s(maskFile, namea);
	strcat_s(maskFile, ".png");
	puts(maskFile);

	if (file && FileCheck(file)) {
		RwImage* img = RtPNGImageRead(file);
		
		RwImage* mask = RtPNGImageRead(maskFile);
		RwImageMakeMask(mask);
		RwImageApplyMask(img, mask);
		RwImageFindRasterFormat(img, rwRASTERTYPETEXTURE, &w, &h, &d, &f);

		RwRaster* raster = RwRasterCreate(w, h, d, f);
		RwRasterSetFromImage(raster, img);
		
		RwImageDestroy(img);
		RwImageDestroy(mask);

		RwTexture* texture = RwTextureCreate(raster);
		strcpy_s(texture->name, 0x20u, name);
		return texture;
	}
	return NULL;
}

RwTexture* CTextureMgr::LoadPNGTextureCB(const char *path, const char* name, const char* namea, const char* namec) {
	int w, h, d, f;
	char file[512];
	char maskFile[512];
	char colFile[512];

	strcpy_s(file, path);
	strcat_s(file, "\\");
	strcat_s(file, name);
	strcat_s(file, ".png");
	puts(file);

	strcpy_s(maskFile, path);
	strcat_s(maskFile, "\\");
	strcat_s(maskFile, namea);
	strcat_s(maskFile, ".png");
	puts(maskFile);

	strcpy_s(colFile, path);
	strcat_s(colFile, "\\");
	strcat_s(colFile, namec);
	strcat_s(colFile, ".png");
	puts(colFile);

	if (file && FileCheck(file)
		&& maskFile && FileCheck(maskFile)
		&& colFile && FileCheck(colFile)) {
		 RwImage* img = RtPNGImageRead(file);
		 RwImageFindRasterFormat(img, rwRASTERTYPETEXTURE, &w, &h, &d, &f);
		 RwRaster* iraster = RwRasterCreate(w, h, d, f);

		 RwRasterSetFromImage(iraster, img);
		 RwImageDestroy(img);
		 
		 RwImage* col = RtPNGImageRead(colFile);
		 RwImageFindRasterFormat(col, rwRASTERTYPETEXTURE, &w, &h, &d, &f);
		 RwRaster* craster = RwRasterCreate(w, h, d, f);
		 RwRasterSetFromImage(craster, col);
		 RwImageDestroy(col);

		 Combine(iraster, craster);

		 RwRasterDestroy(iraster);

		 RwImage* newimg = RwImageCreate(w, h, 32);
		 RwImageAllocatePixels(newimg);
		 RwImageSetFromRaster(newimg, craster);
		 RwRasterDestroy(craster);

		 RwImage* mask = RtPNGImageRead(maskFile);
		 RwImageMakeMask(mask);
		 RwImageApplyMask(newimg, mask);
		 RwImageFindRasterFormat(newimg, rwRASTERTYPETEXTURE, &w, &h, &d, &f);

		 RwRaster* niraster = RwRasterCreate(w, h, d, f);
		 RwRasterSetFromImage(niraster, newimg);
		 RwImageDestroy(newimg);
		 RwImageDestroy(mask);

		 RwTexture* texture = RwTextureCreate(niraster);
		 strcpy_s(texture->name, 0x20u, name);
		return texture;
	}
	return NULL;
}

void CTextureMgr::Delete(RwTexture* t) {
	if (t && t->refCount <= 1) {
		RwTextureDestroy(t);
	}
}

void CTextureMgr::Combine(RwRaster* raster1, RwRaster* raster2) {
	RwUInt8* v3;
	RwUInt8* v4;
	unsigned __int16 *v5;
	int v6;
	double v7;
	unsigned __int16 v8;
	unsigned __int16 v9;
	double v10;
	RwUInt8* v11;
	RwUInt8* v13;
	RwUInt8* v14;
	int v15;
	double v16;
	int v17;
	double v18;
	RwUInt8* v20;
	int v21;
	int v22;
	float v23;
	float v24;

	if (raster1->depth == 16 && raster2->depth == 16) {
		v3 = RwRasterLock(raster1, 0, 3);
		v4 = RwRasterLock(raster2, 0, 3);
		v5 = (unsigned __int16*)v4;

		if (raster1->height * raster1->width > 0) {
			v6 = v3 - v4;
			for (int i = 0; i < raster1->height * raster1->width; i++) {
				v23 = (double)(*(unsigned __int16*)((char*)v5 + v6) >> 12) / 15.0;
				v7 = 1.0 - v23;
				v8 = *v5 ^ ((unsigned __int8)*v5 ^ (unsigned __int8)(int)((double)(*(unsigned __int16*)((char*)v5 + v6) & 0xF)
							* 2.066666603088379
							* v23
							+ (double)(*v5 & 0x1F) * v7)) & 0x1F;
				*v5 = v8;
				v9 = v8 ^ (v8 ^ (32
						   * (int)((double)((*(unsigned __int16*)((char*)v5 + v6) >> 4) & 0xF) * 4.199999809265137 * v23
						   + (double)((v8 >> 5) & 0x3F) * v7))) & 0x7E0;
				*v5 = v9;
				v10 = v23 * ((double)(*((BYTE*)v5++ + v6 + 1) & 0xF) * 2.066666603088379) + v7 * (double)(v9 >> 11);
				*(v5 - 1) = v9 & 0x7FF | ((unsigned __int16)(int)v10 << 11);
			}
		}
	}

	if (raster1->depth == 16)
		return;

	v20 = RwRasterLock(raster1, 0, 3);
	v11 = RwRasterLock(raster2, 0, 3);

	if (raster1->height * raster1->width > 0) {
		v13 = v11 + 2;
		v14 = v20 + 1;
		v15 = v11 - v20 - 1;
		for (int i = 0; i < raster1->height * raster1->width; i++) {
			v16 = (double)*(unsigned __int8 *)(v14 + 2);
			v21 = *(unsigned __int8 *)(v14 - 1);
			v17 = *(unsigned __int8 *)(v15 + v14);
			v14 += 4;
			v13 += 4;
			v24 = v16 / 255.0;
			v18 = 1.0 - v24;
			*(BYTE *)(v15 + v14 - 4) = (int)((double)v21 * v24 + (double)v17 * v18);
			*(BYTE *)(v13 - 5) = (int)(v24 * (double)*(unsigned __int8 *)(v14 - 4)
									   + (double)*(unsigned __int8 *)(v13 - 5) * v18);
			*(BYTE *)(v13 - 4) = (int)(v24 * (double)*(unsigned __int8 *)(v14 - 3)
									   + v18 * (double)*(unsigned __int8 *)(v13 - 4));
		}
	}
	RwRasterUnlock(raster1);
	RwRasterUnlock(raster2);
}

CTextureRel::CTextureRel() {
	texture = NULL;
	path[0] = '\0';
	name[0] = '\0';
}

CTextureRel::CTextureRel(RwTexture* tex, const char* _path, const char* _name) {
	texture = tex;
	strcpy(path,  _path);
	strcpy(name, _name);
}
