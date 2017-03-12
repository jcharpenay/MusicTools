#pragma once

class RefCounted {
public:
	// Constructor
	RefCounted() : m_refCount( 1 ) {}

	// Destructor
	virtual ~RefCounted() {}

	// Operations
	void AddRef() { m_refCount++; }
	void Release() {
		if ( --m_refCount == 0 ) {
			delete this;
		}
	}

private:
	unsigned int m_refCount;
};

template< class T > class RefCountedPtr {
public:
	// Constructor
	RefCountedPtr() : m_ptr( NULL ) {}
	explicit RefCountedPtr( T * _ptr ) : m_ptr( _ptr ) {}
	RefCountedPtr( const RefCountedPtr< T > & _other ) : m_ptr( _other.m_ptr ) { AddRefIfNotNull(); }

	// Destructor
	~RefCountedPtr() {
		ReleaseIfNotNull();
	}

	// Operations
	void operator=( T * _ptr ) {
		if ( m_ptr != _ptr ) {
			ReleaseIfNotNull();
			m_ptr = _ptr;
			AddRefIfNotNull();
		}
	}

	void operator=( const RefCountedPtr< T > & _other ) {
		if ( m_ptr != _other.m_ptr ) {
			ReleaseIfNotNull();
			m_ptr = _other.m_ptr;
			AddRefIfNotNull();
		}
	}

	// Accessors
	const T ** operator&() const	{ return &m_ptr; }
	T ** operator&()				{ return &m_ptr; }
	operator const T*() const		{ return m_ptr; }
	operator T*()					{ return m_ptr; }
	const T * operator->() const	{ return m_ptr; }
	T * operator->()				{ return m_ptr; }

protected:
	T * m_ptr;

	void AddRefIfNotNull() {
		if ( m_ptr != NULL ) {
			m_ptr->AddRef();
		}
	}

	void ReleaseIfNotNull() {
		if ( m_ptr != NULL ) {
			m_ptr->Release();
		}
	}
};

template< class T > class ComPtr : public RefCountedPtr< T > {
public:
	// Constructor
	ComPtr() {}
	ComPtr( T * _ptr ) : RefCountedPtr( _ptr ) {}
	ComPtr( const ComPtr< T > & _other ) : RefCountedPtr( _other ) {}

	// Operations
	template< class U > HRESULT As( ComPtr< U > & _other ) const {
		if ( m_ptr != NULL ) {
			_other = NULL;
			return m_ptr->QueryInterface( __uuidof( IPortableDeviceContent2 ), reinterpret_cast< void ** >( &_other ) );
		} else {
			return E_POINTER;
		}
	}
};
