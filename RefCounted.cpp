#include "Precompiled.h"
#include "RefCounted.h"

#ifdef _DEBUG
	unsigned int RefCounted::s_numObjects = 0;
#endif // _DEBUG

RefCounted::RefCounted() : m_refCount( 0 ) {
#ifdef _DEBUG
	s_numObjects++;
#endif // _DEBUG
}

RefCounted::~RefCounted() {
	DEBUG_ASSERT( m_refCount == 0 );
#ifdef _DEBUG
	s_numObjects--;
#endif // _DEBUG
}

void RefCounted::AddRef() {
	m_refCount++;
}

void RefCounted::Release() {
	DEBUG_ASSERT( m_refCount > 0 );
	if ( --m_refCount == 0 ) {
		delete this;
	}
}
