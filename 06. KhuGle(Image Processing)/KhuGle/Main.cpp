//
//	Dept. Software Convergence, Kyung Hee University
//	Prof. Daeho Lee, nize@khu.ac.kr
//
#include "KhuGleWin.h"
#include "KhuGleSignal.h"
#include <iostream>
#include <algorithm>
#include <string>
#include<iostream>
#include<fstream>

#pragma warning(disable:4996)

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif  // _DEBUG

constexpr const unsigned char QuantizationTableY[8][8] =
{
	{ 16, 11, 10, 16,  24,  40,  51,  61, },
	{ 12, 12, 14, 19,  26,  58,  60,  66, },
	{ 14, 13, 16, 24,  40,  57,  69,  57, },
	{ 14, 17, 22, 29,  51,  87,  80,  62, },
	{ 18, 22, 37, 56,  68, 109, 103,  77, },
	{ 24, 36, 55, 64,  81, 104, 113,  92, },
	{ 49, 64, 78, 87, 103, 121, 120, 101, },
	{ 72, 92, 95, 98, 112, 100, 103,  99, },
};

constexpr const unsigned char QuantizationTableCbCr[8][8] =
{
	{ 17, 18, 24, 47, 99, 99, 99, 99, },
	{ 18, 21, 26, 66, 99, 99, 99, 99, },
	{ 24, 26, 56, 99, 99, 99, 99, 99, },
	{ 47, 66, 99, 99, 99, 99, 99, 99, },
	{ 99, 99, 99, 99, 99, 99, 99, 99, },
	{ 99, 99, 99, 99, 99, 99, 99, 99, },
	{ 99, 99, 99, 99, 99, 99, 99, 99, },
	{ 99, 99, 99, 99, 99, 99, 99, 99, },
};


class CKhuGleImageLayer : public CKhuGleLayer {
public:
	CKhuGleSignal m_Image, m_ImageOut;

	CKhuGleImageLayer(int nW, int nH, KgColor24 bgColor, CKgPoint ptPos = CKgPoint(0, 0))
		: CKhuGleLayer(nW, nH, bgColor, ptPos) {}
	void DrawBackgroundImage();
	
};

void CKhuGleImageLayer::DrawBackgroundImage()
{
	for(int y = 0 ; y < m_nH ; y++)
		for(int x = 0 ; x < m_nW ; x++)
		{
			m_ImageBgR[y][x] = KgGetRed(m_bgColor);
			m_ImageBgG[y][x] = KgGetGreen(m_bgColor);
			m_ImageBgB[y][x] = KgGetBlue(m_bgColor);
		}
	// Original Image
	if(m_Image.m_Red && m_Image.m_Green && m_Image.m_Blue)
	{
		for(int y = 0 ; y < m_Image.m_nH && y < m_nH ; ++y)
			for(int x = 0 ; x < m_Image.m_nW && x < m_nW ; ++x)
			{
				m_ImageBgR[y][x] = m_Image.m_Red[y][x];
				m_ImageBgG[y][x] = m_Image.m_Green[y][x];
				m_ImageBgB[y][x] = m_Image.m_Blue[y][x];
			}
	}
	// Output Image
	if(m_ImageOut.m_Red && m_ImageOut.m_Green && m_ImageOut.m_Blue)
	{
		int OffsetX = 300, OffsetY = 0;
		for(int y = 0 ; y < m_ImageOut.m_nH && y + OffsetY < m_nH ; ++y)
			for(int x = 0 ; x < m_ImageOut.m_nW && x + OffsetX < m_nW ; ++x)
			{
				m_ImageBgR[y + OffsetY][x + OffsetX] = m_ImageOut.m_Red[y][x];
				m_ImageBgG[y + OffsetY][x + OffsetX] = m_ImageOut.m_Green[y][x];
				m_ImageBgB[y + OffsetY][x + OffsetX] = m_ImageOut.m_Blue[y][x];
			}
	}
}

class CImageProcessing : public CKhuGleWin {
public:
	CKhuGleImageLayer *m_pImageLayer;

	CImageProcessing(int nW, int nH, char *ImagePath);
	void Update();
};

CImageProcessing::CImageProcessing(int nW, int nH, char *ImagePath) 
	: CKhuGleWin(nW, nH) {
	m_pScene = new CKhuGleScene(640, 480, KG_COLOR_24_RGB(100, 100, 150));

	m_pImageLayer = new CKhuGleImageLayer(600, 420, KG_COLOR_24_RGB(150, 150, 200), CKgPoint(20, 30));
	m_pImageLayer->m_Image.ReadBmp(ImagePath);
	m_pImageLayer->m_ImageOut.ReadBmp(ImagePath);
	m_pImageLayer->DrawBackgroundImage();
	m_pScene->AddChild(m_pImageLayer);
}

void CImageProcessing::Update()
{
	BOOL b_isQuantized = FALSE;

	if (m_bKeyPressed['1'])
	{
		double** InputR = dmatrix(m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);
		double** InputG = dmatrix(m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);
		double** InputB = dmatrix(m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);

		double** Y = dmatrix(m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);
		double** Cb = dmatrix(m_pImageLayer->m_Image.m_nH / 2, m_pImageLayer->m_Image.m_nW / 2);
		double** Cr = dmatrix(m_pImageLayer->m_Image.m_nH / 2, m_pImageLayer->m_Image.m_nW / 2);

		double** OutY = dmatrix(m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);
		double** OutCb = dmatrix(m_pImageLayer->m_Image.m_nH / 2, m_pImageLayer->m_Image.m_nW / 2);
		double** OutCr = dmatrix(m_pImageLayer->m_Image.m_nH / 2, m_pImageLayer->m_Image.m_nW / 2);

		int QuantizationY[8][8] = { 0 };
		int QuantizationCb[8][8] = { 0 };
		int QuantizationCr[8][8] = { 0 };

		// Initialize the RGB input
		for (int y = 0; y < m_pImageLayer->m_Image.m_nH; ++y)
		{
			for (int x = 0; x < m_pImageLayer->m_Image.m_nW; ++x)
			{
				InputR[y][x] = m_pImageLayer->m_Image.m_Red[y][x];
				InputG[y][x] = m_pImageLayer->m_Image.m_Green[y][x];
				InputB[y][x] = m_pImageLayer->m_Image.m_Blue[y][x];
			}
		}

		// RGB to YCbCr
		for (int y = 0; y < m_pImageLayer->m_Image.m_nH; ++y)
		{
			for (int x = 0; x < m_pImageLayer->m_Image.m_nW; ++x)
			{
				Y[y][x] = 0.299 * InputR[y][x] + 0.587 * InputG[y][x] + 0.114 * InputB[y][x] + 0.0;
			}
		}
		double tempCb = 0.0;
		double tempCr = 0.0;
		for (int y = 0; y < m_pImageLayer->m_Image.m_nH; y += 2)
		{
			for (int x = 0; x < m_pImageLayer->m_Image.m_nW; x += 2)
			{
				for (int i = 0; i < 2; ++i)
				{
					for (int j = 0; j < 2; ++j)
					{
						tempCb += -0.169 * InputR[y + i][x + j] + -0.331 * InputG[y + i][x + j] + 0.499 * InputB[y + i][x + j] + 128.0;
						tempCr += 0.499 * InputR[y + i][x + j] + -0.418 * InputG[y + i][x + j] + -0.0813 * InputB[y + i][x + j] + 128.0;
					}
				}
				Cb[y / 2][x / 2] = tempCb / 4;
				Cr[y / 2][x / 2] = tempCr / 4;
				tempCb = 0.0;
				tempCr = 0.0;
			}
		}
		std::cout << "Convert RGB->YCbCr" << std::endl;

		// Block DCT -> 8 by 8
		DCT2D(Y, OutY, m_pImageLayer->m_Image.m_nW, m_pImageLayer->m_Image.m_nH, 8);
		DCT2D(Cb, OutCb, m_pImageLayer->m_Image.m_nW / 2, m_pImageLayer->m_Image.m_nH / 2, 8);
		DCT2D(Cr, OutCr, m_pImageLayer->m_Image.m_nW / 2, m_pImageLayer->m_Image.m_nH / 2, 8);
		std::cout << "Block DCT 8x8" << std::endl;

		
		// Quantization
		for (int i = 0; i < 8; ++i)
		{
			for (int j = 0; j < 8; ++j)
			{
				QuantizationY[i][j] = (int)(OutY[i][j] / QuantizationTableY[i][j]);
				QuantizationCb[i][j] = (int)(OutCb[i][j] / QuantizationTableCbCr[i][j]);
				QuantizationCr[i][j] = (int)(OutCr[i][j] / QuantizationTableCbCr[i][j]);
			}
		}
		std::cout << "Quantization" << std::endl;
		
		int CountZeroY = 0;
		int CountZeroCb = 0;
		int CountZeroCr = 0;

		std::vector<int> HistogramY;
		std::vector<int> HistogramCountY;
		std::vector<float> HistogramProbabilityY;
		std::vector<int> HistogramCb;
		std::vector<int> HistogramCountCb;
		std::vector<float> HistogramProbabilityCb;
		std::vector<int> HistogramCr;
		std::vector<int> HistogramCountCr;
		std::vector<float> HistogramProbabilityCr;

		int EntropyY = 0;
		int EntropyCb = 0;
		int EntropyCr = 0;
		
		// Channel Histogram File
		std::ofstream writeFile; std::ofstream writeFile2; std::ofstream writeFile3;
		std::string str; std::string str2; std::string str3;
		writeFile.open("YChannel.txt");
		writeFile2.open("CbChannel.txt");
		writeFile3.open("CrChannel.txt");
		for (int i = 0; i < 8; ++i)
		{
			for (int j = 0; j < 8; ++j)
			{
				str += std::to_string(QuantizationY[i][j]) + " ";
				str2 += std::to_string(QuantizationCb[i][j]) + " ";
				str3 += std::to_string(QuantizationCr[i][j]) + " ";
			}
			str += '\n';
			str2 += '\n';
			str3 += '\n';
		}
		writeFile.write(str.c_str(), str.size());
		writeFile2.write(str2.c_str(), str2.size());
		writeFile3.write(str3.c_str(), str3.size());

		writeFile.close();
		writeFile2.close();
		writeFile3.close();

		std::cout << "==========(Y) Channel Quantization===========" << std::endl;
		for (int i = 0; i < 8; ++i)
		{
			for (int j = 0; j < 8; ++j)
			{
				if (QuantizationY[i][j] == 0)
					CountZeroY++;
				else if (QuantizationY[i][j] != 0)
				{
					HistogramY.push_back(QuantizationY[i][j]);
				}
			}
		}
		std::cout << "1) Count Zero in Y Channel : " << CountZeroY << std::endl;
		for (int i = 0; i < HistogramY.size(); i++)
		{
			int cnt = std::count(HistogramY.begin(), HistogramY.end(), HistogramY[i]);
			HistogramCountY.push_back(cnt);
		}

		std::vector<int> Saved;
		std::cout << "2) Count NonZero in Y Channel " << std::endl;
		for (int i = 0; i < HistogramY.size(); i++)
		{
			int SaveCnt = std::count(Saved.begin(), Saved.end(), HistogramY[i]);
			if (HistogramY[i] != 0 && SaveCnt == 0)
			{
				std::cout << "NonZero Histogram : " << HistogramY[i] << ", Count : " << HistogramCountY[i] << '\n';
			}
			Saved.push_back(HistogramY[i]);
		}
		
		// Y Probability 
		for (int i = 0; i < HistogramCountY.size(); i++)
		{
			std::cout << std::fixed;
			std::cout.precision(2);
			HistogramProbabilityY.push_back(HistogramCountY[i] / 64.0f);
			
		}
		// Y Entropy Caculate - Non Zero
		for (int i = 0; i < HistogramProbabilityY.size(); i++)
		{
			EntropyY += log(HistogramProbabilityY[i] * HistogramProbabilityY[i]);
		}
		// Y Entropy Caculate - Zero
		EntropyY += log((CountZeroY / 64.0f) * (CountZeroY / 64.0f));
		EntropyY = -EntropyY;
		std::cout << "3) Entropy in Y Channel : " << EntropyY << std::endl << std::endl;

		std::cout << "==========(Cb) Channel Quantization===========" << std::endl;
		for (int i = 0; i < 8; ++i)
		{
			for (int j = 0; j < 8; ++j)
			{
				if (QuantizationCb[i][j] == 0)
					CountZeroCb++;
				else if (QuantizationCb[i][j] != 0)
				{
					HistogramCb.push_back(QuantizationCb[i][j]);
				}
			}
		}
		std::cout << "1) Count Zero in Cb Channel : " << CountZeroCb << std::endl;

		for (int i = 0; i < HistogramCb.size(); i++)
		{
			int cnt = std::count(HistogramCb.begin(), HistogramCb.end(), HistogramCb[i]);
			HistogramCountCb.push_back(cnt);
		}

		std::vector<int> Saved2;
		std::cout << "2) Count NonZero in Cb Channel " << std::endl;
		for (int i = 0; i < HistogramCb.size(); i++)
		{
			int SaveCnt = std::count(Saved2.begin(), Saved2.end(), HistogramCb[i]);
			if (HistogramCb[i] != 0 && SaveCnt == 0)
			{
				std::cout << "NonZero Histogram : " << HistogramCb[i] << ", Count : " << HistogramCountCb[i] << '\n';
			}
			Saved2.push_back(HistogramCb[i]);
		}
		// Cb Probability
		for (int i = 0; i < HistogramCountCb.size(); i++)
		{
			std::cout << std::fixed;
			std::cout.precision(2);
			HistogramProbabilityCb.push_back(HistogramCountCb[i] / 64.0f);

		}
		// Cb Entropy Caculate - Non Zero
		for (int i = 0; i < HistogramProbabilityCb.size(); i++)
		{
			EntropyCb += log(HistogramProbabilityCb[i] * HistogramProbabilityCb[i]);
		}
		// Cb Entropy Caculate - Zero
		EntropyCb += log((CountZeroCb / 64.0f) * (CountZeroCb / 64.0f));
		EntropyCb = -EntropyCb;
		std::cout << "3) Entropy in Cb Channel : " << EntropyCb << std::endl << std::endl;

		std::cout << "==========(Cr) Channel Quantization===========" << std::endl;
		for (int i = 0; i < 8; ++i)
		{
			for (int j = 0; j < 8; ++j)
			{
				if (QuantizationCr[i][j] == 0)
					CountZeroCr++;
				else if (QuantizationCr[i][j] != 0)
				{
					HistogramCr.push_back(QuantizationCr[i][j]);
				}
			}
		}
		std::cout << "1) Count Zero in Cr Channel : " << CountZeroCr << std::endl;

		for (int i = 0; i < HistogramCr.size(); i++)
		{
			int cnt = std::count(HistogramCr.begin(), HistogramCr.end(), HistogramCr[i]);
			HistogramCountCr.push_back(cnt);
		}

		std::vector<int> Saved3;
		std::cout << "2) Count NonZero in Cr Channel " << std::endl;
		for (int i = 0; i < HistogramCr.size(); i++)
		{
			int SaveCnt = std::count(Saved3.begin(), Saved3.end(), HistogramCr[i]);
			if (HistogramCr[i] != 0 && SaveCnt == 0)
			{
				std::cout << "NonZero Histogram : " << HistogramCr[i] << ", Count : " << HistogramCountCr[i] << '\n';
			}
			Saved3.push_back(HistogramCr[i]);
		}
		// Cr Probability
		for (int i = 0; i < HistogramCountCr.size(); i++)
		{
			std::cout << std::fixed;
			std::cout.precision(2);
			HistogramProbabilityCr.push_back(HistogramCountCr[i] / 64.0f);

		}
		// Cr Entropy Caculate - Non Zero
		for (int i = 0; i < HistogramProbabilityCr.size(); i++)
		{
			EntropyCr += log(HistogramProbabilityCr[i] * HistogramProbabilityCr[i]);
		}
		// Cr Entropy Caculate - Zero
		EntropyCr += log((CountZeroCr / 64.0f) * (CountZeroCr / 64.0f));
		EntropyCr = -EntropyCr;
		std::cout << "3) Entropy in Cr Channel : " << EntropyCr << std::endl << std::endl;
	
		double MaxY, MaxCb, MaxCr, MinY, MinCb, MinCr;
		// Dequantization
		for (int i = 0; i < 8; ++i)
		{
			for (int j = 0; j < 8; ++j)
			{
				OutY[i][j] = (double)QuantizationY[i][j] * QuantizationTableY[i][j];
				OutCb[i][j] = (double)QuantizationCb[i][j] * QuantizationTableCbCr[i][j];
				OutCr[i][j] = (double)QuantizationCr[i][j] * QuantizationTableCbCr[i][j];
			}
		}
		std::cout << "Dequantization" << std::endl;

		// IDCT
		IDCT2D(OutY, Y, m_pImageLayer->m_Image.m_nW, m_pImageLayer->m_Image.m_nH, 8);
		IDCT2D(OutCb, Cb, m_pImageLayer->m_Image.m_nW / 2, m_pImageLayer->m_Image.m_nH / 2, 8);
		IDCT2D(OutCr, Cr, m_pImageLayer->m_Image.m_nW / 2, m_pImageLayer->m_Image.m_nH / 2, 8);
		std::cout << "IDCT" << std::endl;
		
		// YCbCr to RGB
		for (int y = 0; y < m_pImageLayer->m_Image.m_nH; ++y)
		{
			for (int x = 0; x < m_pImageLayer->m_Image.m_nW; ++x)
			{
				m_pImageLayer->m_ImageOut.m_Red[y][x] = std::clamp(Y[y][x] + 1.402 * (Cr[y / 2][x / 2] - 128), 0.0, 255.0);
				m_pImageLayer->m_ImageOut.m_Green[y][x] = std::clamp(Y[y][x] - 0.344 * (Cb[y / 2][x / 2] - 128) - 0.714 * (Cr[y / 2][x / 2] - 128), 0.0, 255.0);
				m_pImageLayer->m_ImageOut.m_Blue[y][x] = std::clamp(Y[y][x] + 1.772 * (Cb[y / 2][x / 2] - 128), 0.0, 255.0);
			}
		}
		std::cout << "Convert YCbCr->RGB" << std::endl;

		// Draw
		m_pImageLayer->DrawBackgroundImage();

		// Print PSNR
		double Psnr = GetPsnr(m_pImageLayer->m_Image.m_Red, m_pImageLayer->m_Image.m_Green, m_pImageLayer->m_Image.m_Blue,
			m_pImageLayer->m_ImageOut.m_Red, m_pImageLayer->m_ImageOut.m_Green, m_pImageLayer->m_ImageOut.m_Blue,
			m_pImageLayer->m_Image.m_nW, m_pImageLayer->m_Image.m_nH);
			
		std::cout << "PSNR: " << Psnr << '\n' << std::endl;

		// Release memory
		free_dmatrix(InputR, m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);
		free_dmatrix(InputG, m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);
		free_dmatrix(InputB, m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);

		free_dmatrix(Y, m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);
		free_dmatrix(Cb, m_pImageLayer->m_Image.m_nH / 2, m_pImageLayer->m_Image.m_nW / 2);
		free_dmatrix(Cr, m_pImageLayer->m_Image.m_nH / 2, m_pImageLayer->m_Image.m_nW / 2);

		free_dmatrix(OutY, m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);
		free_dmatrix(OutCb, m_pImageLayer->m_Image.m_nH / 2, m_pImageLayer->m_Image.m_nW / 2);
		free_dmatrix(OutCr, m_pImageLayer->m_Image.m_nH / 2, m_pImageLayer->m_Image.m_nW / 2);

		m_bKeyPressed['1'] = false;
	
	}



	if(m_bKeyPressed['D'] || m_bKeyPressed['I'] || m_bKeyPressed['C']
		|| m_bKeyPressed['E'] || m_bKeyPressed['M'])
	{
		bool bInverse = m_bKeyPressed['I'];
		bool bCompression = m_bKeyPressed['C'];
		bool bEdge = m_bKeyPressed['E'];
		bool bMean = m_bKeyPressed['M'];

		// 영상 처리를 위한 RGB 각각을 저장하기 위한 Matrix 생성
		double **InputR = dmatrix(m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);
		double **InputG = dmatrix(m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);
		double **InputB = dmatrix(m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);

		double **OutR = dmatrix(m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);
		double **OutG = dmatrix(m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);
		double **OutB = dmatrix(m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);


		for(int y = 0 ; y < m_pImageLayer->m_Image.m_nH ; ++y)
			for(int x = 0 ; x < m_pImageLayer->m_Image.m_nW ; ++x)
			{
				InputR[y][x] = m_pImageLayer->m_Image.m_Red[y][x];
				InputG[y][x] = m_pImageLayer->m_Image.m_Green[y][x];
				InputB[y][x] = m_pImageLayer->m_Image.m_Blue[y][x];
			}


		if(bEdge)
		{
			for(int y = 0 ; y < m_pImageLayer->m_ImageOut.m_nH ; ++y)
				for(int x = 0 ; x < m_pImageLayer->m_ImageOut.m_nW ; ++x)
				{
					OutR[y][x] = OutG[y][x] = OutB[y][x] = 0.;
					if(x > 0 && x < m_pImageLayer->m_ImageOut.m_nW-1 && 
						y > 0 && y < m_pImageLayer->m_ImageOut.m_nH-1)
					{
						double Rx = InputR[y-1][x-1] + 2*InputR[y][x-1] + InputR[y+1][x-1]
							- InputR[y-1][x+1] - 2*InputR[y][x+1] - InputR[y+1][x+1];
						double Ry = InputR[y-1][x-1] + 2*InputR[y-1][x] + InputR[y-1][x+1]
							- InputR[y+1][x-1] - 2*InputR[y+1][x] - InputR[y+1][x+1];
						double Gx = InputG[y-1][x-1] + 2*InputG[y][x-1] + InputG[y+1][x-1]
							- InputG[y-1][x+1] - 2*InputG[y][x+1] - InputG[y+1][x+1];
						double Gy = InputG[y-1][x-1] + 2*InputG[y-1][x] + InputG[y-1][x+1]
							- InputG[y+1][x-1] - 2*InputG[y+1][x] - InputG[y+1][x+1];
						double Bx = InputB[y-1][x-1] + 2*InputB[y][x-1] + InputB[y+1][x-1]
							- InputB[y-1][x+1] - 2*InputB[y][x+1] - InputB[y+1][x+1];
						double By = InputB[y-1][x-1] + 2*InputB[y-1][x] + InputB[y-1][x+1]
							- InputB[y+1][x-1] - 2*InputB[y+1][x] - InputB[y+1][x+1];
						
						OutR[y][x] = sqrt(Rx*Rx + Ry*Ry);
						OutG[y][x] = sqrt(Gx*Gx + Gy*Gy);
						OutB[y][x] = sqrt(Bx*Bx + By*By);
					}
				}

			std::cout << "Edge" << std::endl;
		}
		else if(bMean)
		{
			for(int y = 0 ; y < m_pImageLayer->m_ImageOut.m_nH ; ++y)
				for(int x = 0 ; x < m_pImageLayer->m_ImageOut.m_nW ; ++x)
				{
					OutR[y][x] = OutG[y][x] = OutB[y][x] = 0.;
					if(x > 0 && x < m_pImageLayer->m_ImageOut.m_nW-1 && 
						y > 0 && y < m_pImageLayer->m_ImageOut.m_nH-1)
					{
						for(int dy = -1 ; dy < 2 ; ++dy)
							for(int dx = -1 ; dx < 2 ; ++dx)
							{
								OutR[y][x] += InputR[y+dy][x+dx];
								OutG[y][x] += InputG[y+dy][x+dx];
								OutB[y][x] += InputB[y+dy][x+dx];
							}
					}
				}

			std::cout << "Mean filter" << std::endl;
		}
		else
		{
			DCT2D(InputR, OutR, m_pImageLayer->m_Image.m_nW, m_pImageLayer->m_Image.m_nH, 8);
			DCT2D(InputG, OutG, m_pImageLayer->m_Image.m_nW, m_pImageLayer->m_Image.m_nH, 8);
			DCT2D(InputB, OutB, m_pImageLayer->m_Image.m_nW, m_pImageLayer->m_Image.m_nH, 8);

			std::cout << "DCT" << std::endl;
		}

		if(!bInverse && ! bCompression)
		{
			
			double MaxR, MaxG, MaxB, MinR, MinG, MinB;

			for(int y = 0 ; y < m_pImageLayer->m_ImageOut.m_nH ; ++y)
				for(int x = 0 ; x < m_pImageLayer->m_ImageOut.m_nW ; ++x)
				{
					if(x == 0 && y == 0)
					{
						MaxR = MinR = OutR[y][x];
						MaxG = MinG = OutG[y][x];
						MaxB = MinB = OutB[y][x];
					}
					else
					{
						if(OutR[y][x] > MaxR) MaxR = OutR[y][x];
						if(OutG[y][x] > MaxG) MaxG = OutG[y][x];
						if(OutB[y][x] > MaxB) MaxB = OutB[y][x];

						if(OutR[y][x] < MinR) MinR = OutR[y][x];
						if(OutG[y][x] < MinG) MinG = OutG[y][x];
						if(OutB[y][x] < MinB) MinB = OutB[y][x];
					}
				}
			
			for(int y = 0 ; y < m_pImageLayer->m_ImageOut.m_nH ; ++y)
				for(int x = 0 ; x < m_pImageLayer->m_ImageOut.m_nW ; ++x)
				{
					if(MaxR == MinR) m_pImageLayer->m_ImageOut.m_Red[y][x] = 0;
					else m_pImageLayer->m_ImageOut.m_Red[y][x] = (int)((OutR[y][x]-MinR)*255/(MaxR-MinR));
					if(MaxG == MinG) m_pImageLayer->m_ImageOut.m_Green[y][x] = 0;
					else m_pImageLayer->m_ImageOut.m_Green[y][x] = (int)((OutG[y][x]-MinG)*255/(MaxG-MinG));
					if(MaxB == MinB) m_pImageLayer->m_ImageOut.m_Blue[y][x] = 0;
					else m_pImageLayer->m_ImageOut.m_Blue[y][x] = (int)((OutB[y][x]-MinB)*255/(MaxB-MinB));
				}
				
				
		}
		else
		{
			if(bCompression)
			{
				for(int y = 0 ; y < m_pImageLayer->m_ImageOut.m_nH ; ++y)
					for(int x = 0 ; x < m_pImageLayer->m_ImageOut.m_nW ; ++x)
					{
						if(x%8 > 3 || y %8 > 3)
						{
							OutR[y][x] = 0;
							OutG[y][x] = 0;
							OutB[y][x] = 0;
						}
					}

				std::cout << "Compression" << std::endl;
			}
			else
				std::cout << "Non compression" << std::endl;

			IDCT2D(OutR, InputR, m_pImageLayer->m_Image.m_nW, m_pImageLayer->m_Image.m_nH, 8);
			IDCT2D(OutG, InputG, m_pImageLayer->m_Image.m_nW, m_pImageLayer->m_Image.m_nH, 8);
			IDCT2D(OutB, InputB, m_pImageLayer->m_Image.m_nW, m_pImageLayer->m_Image.m_nH, 8);

			double MaxR, MaxG, MaxB, MinR, MinG, MinB;
			for(int y = 0 ; y < m_pImageLayer->m_ImageOut.m_nH ; ++y)
				for(int x = 0 ; x < m_pImageLayer->m_ImageOut.m_nW ; ++x)
				{
					if(x == 0 && y == 0)
					{
						MaxR = MinR = InputR[y][x];
						MaxG = MinG = InputG[y][x];
						MaxB = MinB = InputB[y][x];
					}
					else
					{
						if(InputR[y][x] > MaxR) MaxR = InputR[y][x];
						if(InputG[y][x] > MaxG) MaxG = InputG[y][x];
						if(InputB[y][x] > MaxB) MaxB = InputB[y][x];

						if(InputR[y][x] < MinR) MinR = InputR[y][x];
						if(InputG[y][x] < MinG) MinG = InputG[y][x];
						if(InputB[y][x] < MinB) MinB = InputB[y][x];
					}
				}
				
			
			for(int y = 0 ; y < m_pImageLayer->m_ImageOut.m_nH ; ++y)
				for(int x = 0 ; x < m_pImageLayer->m_ImageOut.m_nW ; ++x)
				{
					/*
					 //Normalization
					if(MaxR == MinR) m_pImageLayer->m_ImageOut.m_Red[y][x] = 0;
					else m_pImageLayer->m_ImageOut.m_Red[y][x] = (int)((InputR[y][x]-MinR)*255/(MaxR-MinR));
					if(MaxG == MinG) m_pImageLayer->m_ImageOut.m_Green[y][x] = 0;
					else m_pImageLayer->m_ImageOut.m_Green[y][x] = (int)((InputG[y][x]-MinG)*255/(MaxG-MinG));
					if(MaxB == MinB) m_pImageLayer->m_ImageOut.m_Blue[y][x] = 0;
					else m_pImageLayer->m_ImageOut.m_Blue[y][x] = (int)((InputB[y][x]-MinB)*255/(MaxB-MinB));
					*/
					
					// Not Normalization
					m_pImageLayer->m_ImageOut.m_Red[y][x] = (std::max)((std::min)((int)(InputR[y][x] + 0.5), 255), 0);
					m_pImageLayer->m_ImageOut.m_Green[y][x] = (std::max)((std::min)((int)(InputG[y][x] + 0.5), 255), 0);
					m_pImageLayer->m_ImageOut.m_Blue[y][x] = (std::max)((std::min)((int)(InputB[y][x] + 0.5), 255), 0);
				}
				
			
		}

		if(bMean || bCompression || bInverse)
		{
			double Psnr = GetPsnr(m_pImageLayer->m_Image.m_Red, m_pImageLayer->m_Image.m_Green, m_pImageLayer->m_Image.m_Blue, 
				m_pImageLayer->m_ImageOut.m_Red, m_pImageLayer->m_ImageOut.m_Green, m_pImageLayer->m_ImageOut.m_Blue, 
				m_pImageLayer->m_Image.m_nW, m_pImageLayer->m_Image.m_nH);

			std::cout << Psnr << std::endl;
		}

		free_dmatrix(InputR, m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);
		free_dmatrix(InputG, m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);
		free_dmatrix(InputB, m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);

		free_dmatrix(OutR, m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);
		free_dmatrix(OutG, m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);
		free_dmatrix(OutB, m_pImageLayer->m_Image.m_nH, m_pImageLayer->m_Image.m_nW);

		m_pImageLayer->DrawBackgroundImage();

		m_bKeyPressed['D'] = m_bKeyPressed['I'] = m_bKeyPressed['C']
			= m_bKeyPressed['E'] = m_bKeyPressed['M'] = false;
	}

	m_pScene->Render();
	DrawSceneTextPos("Image Processing", CKgPoint(0, 0));
	
	CKhuGleWin::Update();
}

int main()
{
	char ExePath[MAX_PATH], ImagePath[MAX_PATH];

	GetModuleFileName(NULL, ExePath, MAX_PATH);

	int i;
	int LastBackSlash = -1;
	int nLen = strlen(ExePath);
	for(i = nLen-1 ; i >= 0 ; i--)
	{
		if(ExePath[i] == '\\') {
			LastBackSlash = i;
			break;
		}
	}

	if(LastBackSlash >= 0)
		ExePath[LastBackSlash] = '\0';

	// Image File Load
	char ImageFile[MAX_PATH];
	std::cout << "Enter the path of the image: ";
	std::cin >> ImageFile;
	sprintf(ImagePath, "%s\\%s", ExePath, ImageFile);

	
	CImageProcessing *pImageProcessing = new CImageProcessing(640, 480, ImagePath);
	KhuGleWinInit(pImageProcessing);

	return 0;
}