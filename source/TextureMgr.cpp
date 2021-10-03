#include "VHud.h"
#include "TextureMgr.h"
#include "Utility.h"
#include "MenuNew.h"

CTextureMgr textureMgr;

CTextureMgr::CTextureMgr() {

}

void CTextureMgr::Init() {
	;;
}

void CTextureMgr::Shutdown() {
	;;
}

RwTexture* CTextureMgr::LoadPNGTextureCB(const char* path, const char* name) {
	int w, h, d, f;
	char file[512];
	RwTexture* texture = NULL;

	strcpy_s(file, path);
	strcat_s(file, "\\");
	strcat_s(file, name);
	strcat_s(file, ".png");
	puts(file);

	if (file && FileCheck(file)) {
		if (RwImage* img = RtPNGImageRead(file)) {
			RwImageFindRasterFormat(img, rwRASTERTYPETEXTURE, &w, &h, &d, &f);

			if (RwRaster* raster = RwRasterCreate(w, h, d, f)) {
				RwRasterSetFromImage(raster, img);

				if (texture = RwTextureCreate(raster)) {
					RwTextureSetName(texture, name);
				}
			}

			RwImageDestroy(img);
		}
	}

	return texture;
}

RwTexture* CTextureMgr::LoadPNGTextureCB(const char *path, const char* name, const char* namea) {
	int w, h, d, f;
	char file[512];
	char maskFile[512];
	char colFile[512];
	RwTexture* texture = NULL;

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
		if (RwImage* img = RtPNGImageRead(file)) {
			if (RwImage* mask = RtPNGImageRead(maskFile)) {
				RwImageMakeMask(mask);
				RwImageApplyMask(img, mask);
				RwImageFindRasterFormat(img, rwRASTERTYPETEXTURE, &w, &h, &d, &f);

				if (RwRaster* raster = RwRasterCreate(w, h, d, f)) {
					RwRasterSetFromImage(raster, img);

					if (texture = RwTextureCreate(raster)) {
						RwTextureSetName(texture, name);
					}
				}

				RwImageDestroy(img);
				RwImageDestroy(mask);
			}
		}
	}

	return texture;
}

RwTexture* CTextureMgr::LoadPNGTextureCB(const char *path, const char* name, const char* namea, const char* namec) {
	int w, h, d, f;
	char file[512];
	char maskFile[512];
	char colFile[512];
	RwTexture* texture = NULL;

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
		if (RwImage* img = RtPNGImageRead(file)) {
			RwImageFindRasterFormat(img, rwRASTERTYPETEXTURE, &w, &h, &d, &f);

			if (RwRaster* iraster = RwRasterCreate(w, h, d, f)) {
				RwRasterSetFromImage(iraster, img);

				if (RwImage* col = RtPNGImageRead(colFile)) {
					RwImageFindRasterFormat(col, rwRASTERTYPETEXTURE, &w, &h, &d, &f);

					if (RwRaster* craster = RwRasterCreate(w, h, d, f)) {
						RwRasterSetFromImage(craster, col);

						Combine(iraster, craster);

						if (RwImage* newimg = RwImageCreate(w, h, 32)) {
							RwImageAllocatePixels(newimg);
							RwImageSetFromRaster(newimg, craster);

							if (RwImage* mask = RtPNGImageRead(maskFile)) {
								RwImageMakeMask(mask);
								RwImageApplyMask(newimg, mask);
								RwImageFindRasterFormat(newimg, rwRASTERTYPETEXTURE, &w, &h, &d, &f);

								if (RwRaster* niraster = RwRasterCreate(w, h, d, f)) {
									RwRasterSetFromImage(niraster, newimg);

									if (texture = RwTextureCreate(niraster)) {
										RwTextureSetName(texture, name);
									}
								}
								RwImageDestroy(mask);
							}
							RwImageDestroy(newimg);
						}
						RwRasterDestroy(craster);
					}
					RwImageDestroy(col);
				}
				RwRasterDestroy(iraster);
			}
			RwImageDestroy(img);
		}
	}

	return texture;
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
