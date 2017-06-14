#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <new>
#ifndef WIN32
#include <unistd.h>
#endif

#include <vapor/BlkMemMgr.h>

using namespace Wasp;
using namespace VAPoR;

//
//	Static member initialization
//
bool BlkMemMgr::_page_aligned_req = true;
size_t BlkMemMgr::_mem_size_max_req = 32768;
size_t BlkMemMgr::_blk_size_req = 32*32*32;

bool BlkMemMgr::_page_aligned = false;
size_t BlkMemMgr::_mem_size_max = 0;
size_t BlkMemMgr::_blk_size = 0;

vector <size_t>	BlkMemMgr::_mem_region_sizes;
vector <unsigned char *> BlkMemMgr::_blks;
vector < vector <BlkMemMgr::_mem_allocation_t > > BlkMemMgr::_mem_regions;
#ifdef	DEAD
#endif

int	BlkMemMgr::_ref_count = 0;

int	BlkMemMgr::_Reinit(size_t n)
{
	long page_size = 0;
	size_t size = 0;

	if (_mem_size_max_req == 0 || _blk_size_req == 0) return(false);

	_page_aligned = _page_aligned_req;
	_mem_size_max = _mem_size_max_req;
	_blk_size = _blk_size_req;

	//
	// Calculate starting region size
	//
	size_t mem_size = n;

	//
	// How much total memory already allocated
	//
	size_t total_size = 0;
	int r;
	for (r=0; r<_mem_regions.size(); r++) total_size += _mem_region_sizes[r];

	//
	// New region size is double preceding one
	//
	if (r>0) mem_size = _mem_region_sizes[r-1] << 1;

	// Make sure region size will be large enough, and not too large
	//
	if (mem_size < n) mem_size = n;
	if ((mem_size + total_size) > _mem_size_max) mem_size = _mem_size_max - total_size;

	if (mem_size < n) return(false);


	if (_page_aligned) {
#ifdef WIN32
		page_size = 4096;
#else
		page_size = sysconf(_SC_PAGESIZE);
		if (page_size < 0) page_size = 0;
#endif
	}

	unsigned char *blks;
	do {
		size = (size_t) _blk_size * (size_t) mem_size;
		size += (size_t) page_size;

		blks = new(nothrow) unsigned char[size];
		if (! blks) {
			SetDiagMsg(
				"BlkMemMgr::_Reinit() : failed to allocate %d blocks, retrying",
				 mem_size
			);
			mem_size = mem_size >> 1;
		}
	} while (blks == NULL && mem_size > 0 && _blk_size > 0);

	if (! blks && mem_size > 0 && _blk_size > 0) {
		SetDiagMsg("Memory allocation of %lu bytes failed", size);
		return(false);
	}
	else {
		SetDiagMsg("BlkMemMgr() : allocated %lu bytes", size);
	}

	unsigned char *blkptr = blks;

	if (page_size) {
		blkptr += page_size - (((size_t) blks) % page_size);
	}

	_mem_allocation_t m;
	vector <_mem_allocation_t> mem_region;
	m._nfree = mem_size;
	m._nused = 0;
	m._blk = blkptr;
	mem_region.push_back(m);

	_mem_regions.push_back(mem_region);
	_blks.push_back(blks);
	_mem_region_sizes.push_back(mem_size);

	return(true);
}

int	BlkMemMgr::RequestMemSize(
	size_t blk_size, 
	size_t num_blks, 
	bool page_aligned
) {

	SetDiagMsg(
		"BlkMemMgr::RequestMemSize(%u,%u,%d)", blk_size, num_blks, page_aligned
	);

	//
	// If there are no instances of this object, re-initialized
	// the static memory pool if needed
	//
	if (blk_size == 0 || num_blks == 0) {
		SetErrMsg("Invalid request");
		return(-1);
	}

	_blk_size_req = blk_size;
	_mem_size_max_req = num_blks;
	_page_aligned_req = page_aligned;

	return(0);
}

BlkMemMgr::BlkMemMgr(
) {

	SetDiagMsg("BlkMemMgr::BlkMemMgr()");


	//
	// If there are no other instances of this object, re-initialized
	// the static memory pool if needed
	//
	if (_ref_count != 0) {
		_ref_count++;
		return;
	}

	for (int i=0; i<_blks.size(); i++) {
		if (_blks[i]) delete [] _blks[i];
	}
	_mem_regions.clear();
	_mem_region_sizes.clear();
	_blks.clear();

	_page_aligned = _page_aligned_req;
	_mem_size_max = _mem_size_max_req;
	_blk_size = _blk_size_req;

	_ref_count = 1;

}

BlkMemMgr::~BlkMemMgr() {
	SetDiagMsg("BlkMemMgr::~BlkMemMgr()");

	if (_ref_count > 0) _ref_count--;

	if (_ref_count != 0) return;

	for (int i=0; i<_blks.size(); i++) {
		if (_blks[i]) delete [] _blks[i];
	}
	_blks.clear();
	_mem_regions.clear();
	_mem_region_sizes.clear();

}

void	*BlkMemMgr::Alloc(
	size_t n,
	bool fill
) {
	SetDiagMsg("BlkMemMgr::Alloc(%d)", n);

	//
	// Check each region, find the first run of blocks large enough
	// to satisfy the request
	//
	void *blk = NULL;
	for (int r=0; r<_mem_regions.size() && ! blk; r++) {
		vector <_mem_allocation_t> &mem_region = _mem_regions[r];
		for (int i=0; i<mem_region.size() && ! blk; i++) {

			if (n<=mem_region[i]._nfree) {	// Found a run of blocks
				blk = mem_region[i]._blk;
				mem_region[i]._nused = n;

				//
				// If run is strictly larger than request split it
				//
				if (n<mem_region[i]._nfree) {
					_mem_allocation_t m;
					m._nfree = mem_region[i]._nfree - n;
					m._nused = 0;
					m._blk = (unsigned char *) mem_region[i]._blk + (_blk_size * n);
					mem_region.insert(mem_region.begin()+i+1, m);
				}
				mem_region[i]._nfree = 0;
			}
		}
	}
				
	
	if (! blk) {
		// Couldn't find space in existing memory pool.
		// Try to allocate more memory.
		//
		if (! BlkMemMgr::_Reinit(n)) 
			return(NULL);

		return(Alloc(n,fill));
	}
				
	if (fill) {
		unsigned char *ptr = (unsigned char *) blk;
		for (size_t i=0; i<n*_blk_size; i++) ptr[i] = 0;
	}

	return(blk);
}

void	BlkMemMgr::FreeMem(
	void *ptr
) {
	SetDiagMsg("BlkMemMgr::FreeMem()");


	bool found = false;
	for (int r=0; r<_mem_regions.size() && ! found; r++) {
		vector <_mem_allocation_t> &mem_region = _mem_regions[r];
		for (int i=0; i<mem_region.size() && ! found; i++) {
			if (ptr == mem_region[i]._blk) {
				found = true;
				mem_region[i]._nfree = mem_region[i]._nused;
				mem_region[i]._nused = 0;

			}
		}
	}
	if (! found) cerr << "Failed to free block " << ptr << endl;

	//
	// Collapse any two adjacent runs of they're both free
	//
	bool collapse;
	do {
		collapse = false;
		for (int r=0; r<_mem_regions.size(); r++) {
		vector <_mem_allocation_t> &mem_region = _mem_regions[r];
			for (int i=0; i<mem_region.size()-1; i++) {
				if (mem_region[i]._nfree && mem_region[i+1]._nfree) {
					mem_region[i]._nfree += mem_region[i+1]._nfree;
					mem_region.erase(mem_region.begin() + i+1);
					collapse = true;
					break;
				}
			}
		}
	} while (collapse);

}
