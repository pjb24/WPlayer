#ifndef MEMORYLEAKCHECK_H
#define MEMORYLEAKCHECK_H

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DBG_NEW
#endif // !DBG_NEW
#endif // _DEBUG

void MemoryLeakCheckInitialize()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
}

#endif // !MEMORYLEAKCHECK_H
