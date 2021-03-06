#ifndef LP_MP_MEMORY_arena_HXX
#define LP_MP_MEMORY_arena_HXX

#pragma once

#include <iostream>
#include <cstring>
#include <mutex>
#include "config.hxx"
#include "spinlock.hxx"

/* 
   allocators using a stack and a more general one using a variable size list of stacks for allocating memory for factors and messages.
   The implementation is taken from Alexander Shekhovtsov's TRW-S code https://gitlab.icg.tugraz.at/shekhovt/part_opt and modified to be compatible with std::allocator

   do zrobienia: make allocator work in multiples of bytes, not in multiples of ints, as malloc also allocates in bytes.
 */

namespace LP_MP {

  using big_size = long long;
  const int sign_block_used = 123456789;
  const int sign_block_unused = 987654321;
  const int sign_malloc = 123123123;
  const static int KB = 1024;
  const static int MB = 1024*KB;
  const static int GB = 1024*MB;

template<typename T>
class stack_arena{
  public:
    using value_type = T;
	private:
		//! memory from which to allocate
		mutable int * _beg;
		mutable int * _end;
		mutable int * _capbeg;
	public:
		constexpr static int overhead = 2;
	public:
		int * cap_beg()const;
		int * beg()const;
		int * end()const;
		//! size in ints
		int size()const;
		//! capacity in ints
		int capacity()const;
		int cap_free()const;
		bool empty()const;
		bool allocated()const;
		bool can_allocate(size_t n, int align)const;
	public:
		stack_arena();
		//stack_arena()
		void attach(int * _Beg, size_t size);
		void detach();
		stack_arena(int * _Beg, size_t size);
	public:
		stack_arena(const stack_arena & x) = default;
		//void operator = (const stack_arena & x) = default;
	public:
		~stack_arena();
	private:
		bool is_top_block(int * P)const;
		static bool is_block_used(int * P);
		static bool is_block_unused(int * P);
		static void mark_block_used(int * P);
		static void mark_block_unused(int * P);
	public:
		static int& block_size(int * P);
		static size_t block_size_bytes(int * P);
		static int& block_sign(int * P);
	public:
    template<class _T> struct rebind {using other = stack_arena<_T>;};
		//char * allocate(int size_bytes, int align);
		//bool deallocate(void * vP);
		T * allocate(std::size_t n, int align);
		void deallocate(T* p, std::size_t n);
		void clean_garbage();
		void load(char * filename);
		void unload(char * filename);
		void check_integrity();
	};

template<typename T1, typename T2>
  bool operator==(const stack_arena<T1>& a1, const stack_arena<T2>& a2)
  {
    return a1._cap == a2._cap && a1._end == a2._end && a1._capbeg == a2._capbeg;
  }
template<typename T1, typename T2>
  bool operator!=(const stack_arena<T1>& a1, const stack_arena<T2>& a2)
  {
    return !(a1==a2);
  }


	//____________________block_arena______________________________
	/*!
	block_arena keeps a list of buffers, and uses stack_arena to allocate from the buffers.
	*/
  template<typename T>
	class block_arena{
	public:
	protected:
		size_t buffer_size;
    mutable std::vector<stack_arena<T>> buffers;
		//mutable dynamic_array1<stack_allocator, mallocator<stack_allocator> > buffers;
		stack_arena<T> spare;//!< when deallocating, save one spare buffer
		size_t peak_reserved;
		size_t current_reserved;
		size_t current_used;
		int alloc_count;
    spinlock lock_;
	private:
		void took_mem(size_t size_bytes);
		void released_mem(size_t size_bytes);
	protected:
		void add_buffer(size_t buffer_size_sp);
		void drop_buffer();
	public:
		//!clean unused blocks in the buffers and drop empty buffers
		void clean_garbage();
	public:
		block_arena(size_t default_buffer_size=16*MB);
		void reserve(size_t reserve_buffer_size);
	private://forbidden
		block_arena(const block_arena & x);
		void operator=(const block_arena & x);
		T* protect_allocate(size_t n, int align);
		void protect_deallocate(void * vP);
	public:
		~block_arena();
	public:
		size_t mem_used()const;
		size_t mem_reserved()const;
		size_t mem_peak_reserved()const;
		static size_t round_up(size_t size_bytes);
		static size_t align_up(size_t size_bytes);
	public:
		T* allocate(size_t n, int aling = sizeof(size_t));
		size_t object_size(void * vP);
		void deallocate(void * vP);
		void* realloc(void * vP, size_t size_bytes);
		void error_allocate(big_size n, const char * caller);
		void check_integrity();
	};

//___________________stack_arena___________________________
template<typename T>
	inline int * stack_arena<T>::cap_beg()const{
		return _capbeg;
   }
template<typename T>
	inline int * stack_arena<T>::beg()const{
		return _beg;
   }
template<typename T>
	inline int * stack_arena<T>::end()const{
		return _end;
   }
	//! size in ints
template<typename T>
	inline int stack_arena<T>::size()const{
		return int(_end - _beg);
   }
	//! capacity in ints
template<typename T>
	inline int stack_arena<T>::capacity()const{
		if (!allocated())return 0;
		return int(_end - _capbeg);
   }
template<typename T>
	inline int stack_arena<T>::cap_free()const{
		return int(_beg - _capbeg);
   }
template<typename T>
	inline bool stack_arena<T>::empty()const{
		return size() == 0;
   }
template<typename T>
	inline bool stack_arena<T>::allocated()const{
		return (_capbeg != 0);
   }
template<typename T>
	inline stack_arena<T>::stack_arena() :_beg(0), _end(0), _capbeg(0){
   }
	//stack_arena()
template<typename T>
	inline void stack_arena<T>::attach(int * _Beg, size_t size){
		if (!empty())throw std::bad_alloc();
		_capbeg = _Beg;
		_end = _Beg + size;
		_beg = _end;
   }
template<typename T>
	inline void stack_arena<T>::detach(){
		_beg = 0;
		_end = 0;
		_capbeg = 0;
   }
template<typename T>
	inline stack_arena<T>::stack_arena(int * _Beg, size_t size) :_capbeg(_Beg){
		_end = _Beg + size;
		_beg = _end;
   }
//template<typename T>
//	inline stack_arena<T>::stack_arena(const stack_arena & x) :_capbeg(x._capbeg), _beg(x._beg), _end(x._end){};
/*
template<typename T>
	inline void stack_arena<T>::operator = (const stack_arena & x){
		if (allocated() || !empty())throw std::runtime_error("buffer is in use");
		_beg = x._beg;
		_end = x._end;
		_capbeg = x._capbeg;
		//x._beg = 0;
		//x._end = 0;
		//x._capbeg = 0;
	};
  */
template<typename T>
	inline stack_arena<T>::~stack_arena(){
		//! all objects must have been destroyed
		assert(empty());
		if (!empty()){
			throw std::runtime_error("buffer is in use");
      }
   }
template<typename T>
	inline bool stack_arena<T>::is_top_block(int * P)const{
		return (P - overhead == beg());
   }
template<typename T>
	inline bool stack_arena<T>::is_block_used(int * P){
		return block_sign(P) == sign_block_used;
   }
template<typename T>
	inline bool stack_arena<T>::is_block_unused(int * P){
		return block_sign(P) == sign_block_unused;
   }
template<typename T>
	inline void stack_arena<T>::mark_block_used(int * P){
		block_sign(P) = sign_block_used;
   }
template<typename T>
	inline void stack_arena<T>::mark_block_unused(int * P){
		block_sign(P) = sign_block_unused;
   }
template<typename T>
	inline int& stack_arena<T>::block_size(int * P){
		return *(P - 2);
   }
template<typename T>
	inline size_t stack_arena<T>::block_size_bytes(int * P){
		return size_t(*(P - 2))*sizeof(int);
   }
template<typename T>
	inline int& stack_arena<T>::block_sign(int * P){
		return *(P - 1);
   }
template<typename T>
	inline bool stack_arena<T>::can_allocate(size_t n, int align)const{
		// assume size_bytes is aligned to sizeof(size_t)
		int size = n*sizeof(T);
		int sz_add_bytes = size_t(beg() - size) % align;
		size = size + sz_add_bytes/sizeof(int);
		return (cap_free() >= size + overhead);
		//return big_size(cap_free())*sizeof(int) >= size_bytes + overhead*sizeof(int);
   }
template<typename T>
	inline T* stack_arena<T>::allocate(std::size_t n, int align){
		//size_bytes is rounded up to multiple of 4
		if (n == 0){
			perror("Allocating 0 bytes?\n"); fflush(stdout);
			abort();
      }
    const int size_bytes = n * sizeof(T);
		// actually assume size_bytes is already aligned, so this just divides
		//int size = (size_bytes + 3) >> 2;
		//int size = size_bytes >> 2;
		// now alignment, must also be multiple of 4
		// how many extra space to add so that (beg()-size) % (align/4) == 0?
		//int sz_add = size_t(beg() - size) % (align / 4);
		//size = size + sz_add;
		int size = size_bytes / sizeof(int);
		int sz_add_bytes = size_t(beg() - size) % align;
		size = size + sz_add_bytes / sizeof(int);
		if (cap_free() < size + overhead){//check if it fits
			throw std::bad_alloc();
      }
		int * P = beg() - size;
		//chech address is aligned
		assert(size_t(P)%(align) == 0);
		block_size(P) = size;
		mark_block_used(P);
		_beg = P - overhead;
    //std::cout << "allocate " << (int*) P << ", this = " << this << "\n";
		return (T*)P;
   }
template<typename T>
	inline void stack_arena<T>::deallocate(T* vP, std::size_t n){
		int* P = (int*)vP;
    //std::cout << "deallocate " << (int*) P << ", this = " << this << "\n";
		if (is_block_used(P) != true){
			perror("Deallocation failed: bad pointer\n"); fflush(stdout);
			abort();
      }
		//mark for detetion
		mark_block_unused(P);
		if (_beg + overhead == P){//this is the top block of this allocator
			//delete it
			_beg = _beg + overhead + block_size(_beg + overhead);
			if (empty() || is_block_unused(_beg + overhead)){//buffer is empty or more unused blocks
				//triger cascade deallocation
        clean_garbage();
        return;
				//return true;
         }
      }
		//return false;
   }

template<typename T>
  inline void stack_arena<T>::clean_garbage(){
		while (!empty() && is_block_unused(_beg + overhead)){
			_beg = _beg + overhead + block_size(_beg + overhead);
			assert(_beg <= _end);
      }
		assert(empty() || is_block_used(_beg + overhead));
  }
template<typename T>
	void stack_arena<T>::load(char * filename){
		//FILE * f = fopen(filename,"rb");
		//int sz_used;
		//fread(&sz_used,sizeof(sz_used),1,f);
		//if(sz_used<capacity())throw std::bad_alloc("cant load");
   }

template<typename T>
	void stack_arena<T>::unload(char * filename){
   }

template<typename T>
	void stack_arena<T>::check_integrity(){
		if (empty())return;
		int * P = _beg + overhead;
		int alive = 0;
		int dead = 0;
		/*
		if(!is_block_used(P)){
		if(!is_block_unused(P)){
		perror("integrity fails, top undefined\n"); fflush(stdout);
		abort();
      }
		//perror("integrity fails, top unused\n"); fflush(stdout);
		//abort();
		}
		*/
		while (P != _end + overhead){
			//check signature
			bool used = is_block_used(P);
			bool unused = is_block_unused(P);
			if (!(used || unused)){
				perror("integrity fails, signatur\n"); fflush(stdout);
				abort();
         }
			if (used){
				++alive;
			} else{
				++dead;
         }
			int sz = block_size(P);
			//check size
			if (P + sz>_end){
				perror("integrity fails, size\n"); fflush(stdout);
				abort();
         }
			// go to next block
			P = P + sz + overhead;
      }
		std::cout << "block: " << alive << " alive and: " << dead << " dead objects\n";
   }


//__________________block_arena________________________
  template<typename T>
	inline void block_arena<T>::took_mem(size_t size_bytes){
		current_reserved += size_bytes;
		peak_reserved = std::max(peak_reserved, current_reserved);
   }
  template<typename T>
	inline void block_arena<T>::released_mem(size_t size_bytes){
		current_reserved -= size_bytes;
   }

  template<typename T>
	void block_arena<T>::error_allocate(big_size n, const char * caller){
		//char s[200];
		printf("Error: memory allocation in %s\n", caller);
		printf("memory requested: %lli\n", big_size(n*sizeof(T)));
		printf("Total reserved: %lli\n", big_size(mem_reserved()));
		fflush(stdout);
		abort();
		throw std::bad_alloc();
	}

  template<typename T>
	inline void block_arena<T>::add_buffer(size_t buffer_size_sp){
		if (spare.allocated() && spare.capacity()*size_t(sizeof(int)) >= buffer_size_sp){//have required ammount in the spare
			buffers.push_back(spare);//steal constructor will make spare empty
		} else{//spare is empty or too small
			//get a new buffer
			int * p = (int*)malloc(buffer_size_sp);
			if (!p)error_allocate(buffer_size_sp, "malloc");
      //stack_arena<T> * buf =
      buffers.push_back({p, buffer_size_sp / sizeof(int)});
			//buf->attach(p, buffer_size_sp / sizeof(int));
			took_mem(buffers.back().capacity()*sizeof(int));
      }
	}

  template<typename T>
	inline void block_arena<T>::drop_buffer(){
		assert(!buffers.empty());
		if (spare.allocated()){
			int * p = spare.cap_beg();
			int cap = spare.capacity()*sizeof(int);
			assert(spare.empty());
			spare.detach();
			free(p);
			released_mem(cap);
      }
		spare = buffers.back();//spare steals the back buffer
    buffers.back().detach();
		buffers.pop_back();//remove the empty buffer from the list
   }

	//!clean unused blocks in the buffers and drop empty buffers
  template<typename T>
	void block_arena<T>::clean_garbage(){
		while (!buffers.empty()){
			buffers.back().clean_garbage();
			if (buffers.back().empty()){//top buffer is empty
				drop_buffer();
			} else{//top buffer still has data
				return;
         }
      }
   }

  template<typename T>
	block_arena<T>::block_arena(size_t default_buffer_size) :buffer_size(default_buffer_size) {
		current_reserved = 0;
		peak_reserved = 0;
		current_used = 0;
		alloc_count = 0;
		buffers.reserve(1000);//Most probably never going to be reallocated
   }

	//inline
  template<typename T>
	void block_arena<T>::reserve(size_t reserve_buffer_size){
    std::lock_guard<spinlock> lock(lock_);
//#pragma omp critical(mem_allocation)
		{
			clean_garbage();
			add_buffer(reserve_buffer_size);
      }
   }

  template<typename T>
	block_arena<T>::block_arena(const block_arena & x) :buffer_size(x.buffer_size){
		current_reserved = 0;
		peak_reserved = 0;
		current_used = 0;
		alloc_count = 0;
   }

  template<typename T>
	void block_arena<T>::operator=(const block_arena & x){
		current_reserved = 0;
		peak_reserved = 0;
		current_used = 0;
   }

  template<typename T>
	block_arena<T>::~block_arena(){
    std::lock_guard<spinlock> lock(lock_);
//#pragma omp critical (mem_allocation)
		{
			clean_garbage();
            //std::cout << "no buffers = " << buffers.size() << "\n";
			//printf("peak mem usage: %lli Mb ", big_size(mem_peak_reserved() / (1 << 20)));
			//printf(" / %i allocations, ", alloc_count);
			//printf("at exit: %lli B\n", big_size(mem_used()));
			//assert(buffers.empty() && spare.empty());
			if (spare.allocated()){
				int * p = spare.cap_beg();
				spare.detach();
				free(p);
         }
			if (!(buffers.empty() && spare.empty())){
				try{
					fprintf(stderr, "Memory leaks detected\n");
				} catch (...){
            }
         }
      }
   }

  template<typename T>
	size_t block_arena<T>::mem_used()const{
		return current_used;
   }

  template<typename T>
	size_t block_arena<T>::mem_reserved()const{
		size_t m = current_reserved - spare.cap_free()*sizeof(int);
		if (!buffers.empty())m -= buffers.back().cap_free()*sizeof(int);
		return m;
   }

  template<typename T>
	size_t block_arena<T>::mem_peak_reserved()const{
		return peak_reserved;
   }


  template<typename T>
	inline size_t block_arena<T>::round_up(size_t size_bytes){
		return (((size_bytes + 3) >> 2) << 2);
   }

  template<typename T>
	inline size_t block_arena<T>::align_up(size_t size_bytes){
		//return ((size_bytes + sizeof(size_t) - 1) / sizeof(size_t))*sizeof(size_t);
		return ((size_bytes + 15) >> 4)<< 4;
   }

  template<typename T>
	T* block_arena<T>::protect_allocate(size_t n, int align){
		T* P;
		int size_bytes = round_up(n*sizeof(T));
		++alloc_count;
		if (!buffers.empty() && buffers.back().can_allocate(n,align)){//fits in the top buffer
			P = buffers.back().allocate(n, align);
			//current_used += round_up(size_bytes);
			current_used += stack_arena<T>::block_size_bytes((int*) P);
			return P;
      }
		if (spare.can_allocate(n, align)){//fits in the spare buffer
			buffers.push_back(spare);
			P = buffers.back().allocate(n, align);
			//current_used += round_up(size_bytes);
			current_used += stack_arena<T>::block_size_bytes((int*) P);
			return P;
      }
		if (n*sizeof(T) < buffer_size / 16){//is small{
			add_buffer(buffer_size);
			P = buffers.back().allocate(n, align);
			//current_used += round_up(size_bytes);
			current_used += stack_arena<T>::block_size_bytes((int*) P);
			return P;
      }
		// is large and does not fit in available buffers
		//use malloc for it (with 2 ints of overhead for signature and size)
    std::cout << "large allocation not fitting into buffers\n";
		big_size cap = round_up(size_bytes);
		big_size size_allocate = cap + sizeof(int) + sizeof(size_t);
		if (size_allocate > (big_size)(std::numeric_limits<std::size_t>::max() / 2)){
			error_allocate(n , "size_check");
      }
		T * Q = (T*)malloc(size_t(size_allocate));
		if (Q == 0)error_allocate(size_allocate, "malloc (2)");
		took_mem(size_t(cap));
		P = (T*)((char*)Q + sizeof(int) + sizeof(size_t));
		*(P - 1) = sign_malloc;
		*((size_t*)(P - 1) - 1) = (size_t)cap;
		current_used += (size_t)cap;
		return P;
      }

  template<typename T>
	T * block_arena<T>::allocate(size_t n, int align){
		T * P;
    std::lock_guard<spinlock> lock(lock_);
//#pragma omp critical (mem_allocation)
		P = protect_allocate(n, align);
		return P;
   }

  template<typename T>
	inline size_t block_arena<T>::object_size(void * vP){
		assert(vP != 0);
		int * P = (int*)vP;
		int sign = stack_arena<T>::block_sign(P);
		if (sign == sign_block_used){
			return stack_arena<T>::block_size(P)*sizeof(int);
		} else if (sign == sign_malloc){
			return *((size_t*)(P - 1) - 1);
		} else{
			printf("Error:unrecognized signature\n");
			throw std::bad_alloc();
      }
   }

	//inline
  template<typename T>
	void block_arena<T>::protect_deallocate(void * vP){
		//if(x==0)throw debug_exception("Invalid pointer.");
		if (vP == 0){
			perror("Deallocation failed: zero pointer\n"); fflush(stdout);
			abort();
      }
		int * P = (int*)vP;
		//check the signature
		int sign = stack_arena<T>::block_sign(P);
		if (sign == sign_block_used){//allocated by stack_arena
			assert(!buffers.empty());
			size_t cap = stack_arena<T>::block_size(P)*sizeof(int);
			current_used -= cap;
			assert(current_used >= 0);
			//does not matter if cP is not from the top buffer (or even from other allocator) -- in that case it will only be marked for deallocation
			buffers.back().deallocate((T*) vP, 1000000000000);
			//if (buffers.back().deallocate((T*) vP, 1000000000000)){//returns 1 when need to clean
			//	clean_garbage();
			//}
			return;
      }
		if (sign == sign_malloc){//was a large separate block
			size_t cap = *((size_t*)(P - 1) - 1);
			stack_arena<T>::block_sign(P) = 321321321;
			current_used -= cap;
			assert(current_used >= 0);
			free((char*)P - sizeof(int) - sizeof(size_t));
			released_mem(cap);
			return;
      }
		if (sign == sign_block_unused){//this isn't good
			printf("Error: Block is already deallocated\n");
			throw std::bad_alloc();
      }
		printf("Unrecognized signature: memory corrupt\n");
		throw std::bad_alloc();
   }

  template<typename T>
	void block_arena<T>::deallocate(void * vP){
    std::lock_guard<spinlock> lock(lock_);
//#pragma omp critical (mem_allocation)
		protect_deallocate(vP);
   }

	//inline
  template<typename T>
	void* block_arena<T>::realloc(void * vP, size_t size_bytes){
		if (!vP)return allocate(size_bytes);
		size_t sz = object_size(vP);
		void * vQ = allocate(size_bytes);
		memcpy(vQ, vP, std::min(sz, size_bytes));
		deallocate(vP);
		//current_used = current_used -sz + round_up(size_bytes);
		return vQ;
   }

  template<typename T>
	void block_arena<T>::check_integrity(){
		for (int b = 0; b<buffers.size(); ++b){
			buffers[b].check_integrity();
      }
		std::cout << "block_arena integrity checked\n";
   }


// stl compliant allocator that allocates memory in a stack like fashion.
template<typename T>
class stack_allocator {
public:
  using value_type = T;
private:
  stack_arena<T>& a_;
public:
  stack_allocator(const stack_allocator&) = default;
  stack_allocator& operator=(stack_allocator&) = delete;
  stack_allocator(stack_arena<T>& a) noexcept : a_(a) {}
  template<typename T2> struct rebind {using other = stack_allocator<T2>;};
  T* allocate(std::size_t n, int align = sizeof(size_t)) { return a_.allocate(n,align); }
  void deallocate(T* p, std::size_t n) { return a_.deallocate(p,n); }
  friend bool operator==(const stack_allocator<T>& a1, const stack_allocator<T> a2) noexcept;
};
template<typename T1, typename T2>
  bool operator==(const stack_allocator<T1>& a1, const stack_allocator<T2>& a2)
  {
    return a1.a_ == a2.a_;
  }
template<typename T1, typename T2>
  bool operator!=(const stack_allocator<T1>& a1, const stack_allocator<T2>& a2)
  {
    return !(a1==a2);
  }

template<typename T>
class block_allocator {
public:
  using value_type = T;
private:
  block_arena<T>& a_;
public:
  block_allocator(const block_allocator<T>&) = default;
  block_allocator& operator=(block_allocator<T>&) = delete;
  block_allocator(block_arena<T>& a) noexcept : a_(a) {}
  template<typename T2> struct rebind {using other = block_allocator<T2>;};
  T* allocate(std::size_t n, int align = sizeof(size_t)) { return a_.allocate(n, align); }
  void deallocate(T* p, std::size_t n) { return a_.deallocate(p); }
  friend bool operator==(const block_allocator<T>& a1, const block_allocator<T> a2) noexcept;
};
template<typename T1, typename T2>
  bool operator==(const block_allocator<T1>& a1, const block_allocator<T2>& a2)
  {
    return a1.a_ == a2.a_;
  }
template<typename T1, typename T2>
  bool operator!=(const block_allocator<T1>& a1, const block_allocator<T2>& a2)
  {
    return !(a1==a2);
  }


// global stack allocator
int stack_arena_mem[100000];
static stack_arena<REAL> global_real_stack_arena(stack_arena_mem,100000);
static stack_allocator<REAL> global_real_stack_allocator(global_real_stack_arena);

// global block allocator
static block_arena<REAL> global_real_block_arena;
static block_allocator<REAL> global_real_block_allocator(global_real_block_arena);

#ifdef LP_MP_PARALLEL
constexpr INDEX no_stack_allocators = 4;
#else
constexpr INDEX no_stack_allocators = 1;
#endif

static std::array<block_arena<REAL>, no_stack_allocators> global_real_block_arena_array;

template <std::size_t... I, typename RandomAccessIterator>
std::array<block_allocator<REAL>, no_stack_allocators> make_block_allocator_array(RandomAccessIterator& first, std::integer_sequence<size_t,I...>) {
  return std::array<block_allocator<REAL>, no_stack_allocators>{ { first[I]... } };
}

static std::array<block_allocator<REAL>, no_stack_allocators> global_real_block_allocator_array ( make_block_allocator_array(global_real_block_arena_array, std::make_integer_sequence<size_t,no_stack_allocators>{} ) ) ;

static thread_local INDEX stack_allocator_index = 0;
// do zrobienia: both above allocators do not destroy their arenas
} // end namespace LP_MP

#endif // LP_MP_MEMORY_arena_HXX

