//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <winnt.h>
#include <Psapi.h>

#pragma pack(push, before_imagehlp, 8)
#include <imagehlp.h>
#pragma pack(pop, before_imagehlp)

//////////////////////////////////////////////////////////////////////

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "dbghelp.lib")

//////////////////////////////////////////////////////////////////////

#if defined(malloc)
#undef malloc
#undef free
#endif

//////////////////////////////////////////////////////////////////////

namespace chs
{

//////////////////////////////////////////////////////////////////////

#ifdef _M_X64
STACKFRAME64 init_stack_frame(CONTEXT c)
{
    STACKFRAME64 s;
    s.AddrPC.Offset = c.Rip;
    s.AddrPC.Mode = AddrModeFlat;
    s.AddrStack.Offset = c.Rsp;
    s.AddrStack.Mode = AddrModeFlat;    
    s.AddrFrame.Offset = c.Rbp;
    s.AddrFrame.Mode = AddrModeFlat;
    return s;
}
#else
STACKFRAME64 init_stack_frame(CONTEXT c)
{
    STACKFRAME64 s;
    s.AddrPC.Offset = c.Eip;
    s.AddrPC.Mode = AddrModeFlat;
    s.AddrStack.Offset = c.Esp;
    s.AddrStack.Mode = AddrModeFlat;    
    s.AddrFrame.Offset = c.Ebp;
    s.AddrFrame.Mode = AddrModeFlat;
    return s;
}
#endif

//////////////////////////////////////////////////////////////////////

#pragma pack(push)
#pragma pack(1)

struct memory_block : list_node<memory_block>
{
	size_t			block_size;
	size_t			frame_count;
	void *			mem_block;
	size_t			alignment;
	DWORD64			stack[64];
};

#pragma pack(pop)

//////////////////////////////////////////////////////////////////////

static void *				base;
static HANDLE				process;
linked_list<memory_block>	memory_block_list;

//////////////////////////////////////////////////////////////////////

void *load_modules_symbols(HANDLE process)
{
	struct module_data
	{
		char image_name[4096];
		char module_name[4096];
		void *base_address;
		DWORD load_size;
	};

	module_data *module_data_array = null;
	DWORD n = 64;						// usually enough
    HMODULE *module_handles = null;
	do
	{
	    DWORD cbNeeded;
		module_handles = (HMODULE *)malloc(sizeof(HMODULE) * n);
	    EnumProcessModules(process, module_handles, n * sizeof(HMODULE), &cbNeeded);
		DWORD required = cbNeeded / sizeof(HMODULE);
		if(required > n)
		{
			n = required;
			free(module_handles);
			module_handles = null;
		}
	}
	while(module_handles == null);

	module_data_array = (module_data *)malloc(sizeof(module_data) * n);
	int w = 0;
	for(uint i=0; i<n; ++i)
	{
		HMODULE hm = module_handles[i];
		module_data &m = module_data_array[i];
		MODULEINFO mi;

		GetModuleInformation(process, hm, &mi, sizeof(mi));
		m.base_address = mi.lpBaseOfDll;
		m.load_size = mi.SizeOfImage;

		GetModuleFileNameExA(process, hm, m.image_name, sizeof(m.image_name));
		GetModuleBaseNameA(process, hm, m.module_name, sizeof(m.module_name));

		DWORD64 addr = SymLoadModule64(process, 0, m.image_name, m.module_name, (DWORD64)m.base_address, m.load_size);
		if(addr == 0)
		{
			DWORD err = GetLastError();
			if(err != ERROR_SUCCESS)
			{
				DebugBreak();
			}
		}
	}
	void *p = module_data_array[0].base_address;
	free(module_handles);
	free(module_data_array);
    return p;
}

//////////////////////////////////////////////////////////////////////

void sym_options(DWORD add, DWORD remove=0)
{
    DWORD symOptions = SymGetOptions();
    symOptions |= add;
    symOptions &= ~remove;
    SymSetOptions(symOptions);
}

//////////////////////////////////////////////////////////////////////

#pragma pack(push)
#pragma pack(1)
template <size_t s> struct IMAGEHLP_SYMBOL64_T : IMAGEHLP_SYMBOL64
{
	enum
	{
		max_size = s
	};
	char buffer[s];

	IMAGEHLP_SYMBOL64_T()
	{
		ZeroMemory(this, sizeof(this));
		MaxNameLength = s;
	}
};
#pragma pack(pop)

typedef IMAGEHLP_SYMBOL64_T<1024> imagehlp_symbol_t;

//////////////////////////////////////////////////////////////////////

void dump_stack(memory_block &b)
{
	char buf[1024];
    DWORD offset_from_symbol=0;
    IMAGEHLP_LINE64 line = {0};
	line.SizeOfStruct = sizeof(line);
    sym_options(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
    IMAGE_NT_HEADERS *h = ImageNtHeader(base);
    DWORD image_type = h->FileHeader.Machine;
	DWORD64 *s = b.stack;
	sprintf_s(buf, "\nLeaked %llu bytes:\n", (uint64)b.block_size);
	OutputDebugStringA(buf);
	for(int n = 0; n < ARRAYSIZE(b.stack); ++n)
	{
		buf[0] = 0;
		DWORD64 addr = *s++;
        if(addr != 0)
		{
            if(SymGetLineFromAddr64( process, addr, &offset_from_symbol, &line ) ) 
			{
				imagehlp_symbol_t sym;
				char name[imagehlp_symbol_t::max_size];
				DWORD64 displacement;
				if(SymGetSymFromAddr64(process, addr, &displacement, &sym))
				{
					UnDecorateSymbolName(sym.Name, name, 1024, UNDNAME_COMPLETE);
					sprintf_s(buf, "%s(%d): %s\n", line.FileName, line.LineNumber, name);
				}
			}
			else if(GetLastError() == ERROR_MOD_NOT_FOUND)
			{
				sprintf_s(buf, "No module for %p\n", addr);
			}
			else
			{
				break;
			}
        }
		else
		{
			break;
		}
		OutputDebugStringA(buf);
	}
}

//////////////////////////////////////////////////////////////////////

void *get_mem(size_t size, size_t align)
{
	size_t aligned_size = size + align - 1 + sizeof(memory_block *);
	HANDLE thread = GetCurrentThread();
	CONTEXT c;
	RtlCaptureContext(&c);
	STACKFRAME64 s = init_stack_frame(c);
	IMAGE_NT_HEADERS *h = ImageNtHeader(base);
    DWORD image_type = h->FileHeader.Machine;
	size_t total_size = aligned_size + sizeof(memory_block);
	memory_block *block = reinterpret_cast<memory_block *>(malloc(total_size));
	char *base = (char *)(block + 1) + sizeof(memory_block *);
	base = (char *)(((intptr_t)base + ((align - 1)) % align));
	((memory_block **)base)[-1] = block;
	block->block_size = size;
	block->frame_count = 0;
	block->alignment = align;
	block->mem_block = base;
	int n = 0;
	do
	{
        if(!StackWalk64(image_type, process, thread, &s, &c, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
		{
            break;
		}
		block->stack[n++] = s.AddrPC.Offset;
        ++block->frame_count;

    } while (s.AddrReturn.Offset != 0);
	if(n < ARRAYSIZE(block->stack))
	{
		block->stack[n++] = 0;
	}
	memory_block_list.push_back(block);
	return block->mem_block;
}

//////////////////////////////////////////////////////////////////////

void __cdecl dump_leaks()
{
	char const *header = "MEMORY LEAKS DETECTED:\n";
	if(memory_block_list.empty())
	{
		TRACE("\n= NO MEMORY LEACKS DETECTED =======================================================================\n");
	}
	else
	{
		TRACE("\n= MEMORY LEAKS DETECTED ===========================================================================\n");
		uint n = 0;
		for(auto &b: reverse(memory_block_list))
		{
			dump_stack(b);
			++n;
		}
		TRACE("Total: %d leaks\n", n);
		TRACE("\n= END OF LEAK REPORT ==============================================================================\n");
	}

	SymCleanup(process);
}

//////////////////////////////////////////////////////////////////////

static size_t amount;

void *memory_alloc(size_t size, size_t align)
{
	if(size > 0)
	{
		// huh, seems to need an extra frame to get the caller!?
		amount += size;
	//	TRACE("memory_alloc(%u,%u) =%d\n", size, align, amount);
		return get_mem(size, align);
	}
	else
	{
		return null;
	}
}

//////////////////////////////////////////////////////////////////////

void memory_free(void *p)
{
	if(p != null)
	{
		memory_block **p1 = (memory_block **)p;
		memory_block *m = p1[-1];
		memory_block_list.remove(m);
		amount -= m->block_size;
	//	TRACE("memory_free(%u) (%u) =%d\n", m->block_size, m->alignment, amount);
		free(m);
	}
}

//////////////////////////////////////////////////////////////////////

void memory_init()
{
	amount = 0;
	memory_block_list.clear();
	process = GetCurrentProcess();
    if(!SymInitialize(process, NULL, FALSE))
	{
		DebugBreak();
	}
	base = load_modules_symbols(process);
}

//////////////////////////////////////////////////////////////////////
// Dodgy way to call it first and last:

#pragma warning(disable: 4073)
#pragma init_seg(lib)

struct memory_leak_tracker
{
	memory_leak_tracker()
	{
		memory_init();
	}

	~memory_leak_tracker()
	{
		dump_leaks();
	}

} memory_leak_tracker;

//////////////////////////////////////////////////////////////////////

} // namespace chs

void *operator new(size_t s)
{
	chs::amount += s;
	return chs::get_mem(s, 1);
}

void operator delete(void *p)
{
	chs::memory_free(p);
}

void operator delete[](void *p)
{
	chs::memory_free(p);
}
