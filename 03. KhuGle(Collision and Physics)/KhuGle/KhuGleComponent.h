//
//	Dept. Software Convergence, Kyung Hee University
//	Prof. Daeho Lee, nize@khu.ac.kr
//
#pragma once

#include <vector>
#include <algorithm>

class CKhuGleComponent
{
public:
	std::vector<CKhuGleComponent*> m_Children;

	CKhuGleComponent *m_Parent;

	CKhuGleComponent();
	virtual ~CKhuGleComponent();

	void AddChild(CKhuGleComponent *pChild);
	void DeleteChild(CKhuGleComponent* pChild);
	virtual void Render() = 0;
};

