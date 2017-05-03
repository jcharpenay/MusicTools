#include "Precompiled.h"
#include "RefCounted.h"

#ifdef _DEBUG
	InterlockedInt RefCounted::s_numObjects;
#endif // _DEBUG

RefCounted::RefCounted() {
#ifdef _DEBUG
	s_numObjects.Increment();
#endif // _DEBUG
}

RefCounted::~RefCounted() {
	DEBUG_ASSERT( m_refCount == 0 );
#ifdef _DEBUG
	s_numObjects.Decrement();
#endif // _DEBUG
}

void RefCounted::AddRef() {
	m_refCount.Increment();
}

void RefCounted::Release() {
	DEBUG_ASSERT( m_refCount > 0 );
	if ( m_refCount.Decrement() == 0 ) {
		delete this;
	}
}
