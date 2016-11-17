/*
 * Copyright (C) 2005 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define NDEBUG 0
#define LOG_TAG "MyRefBase"

#ifndef ANDROID_WREF_BASE_H
#define ANDROID_WREF_BASE_H

#include <cutils/atomic.h>

#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <utils/Log.h>
#include <utils/CallStack.h>

#include "WStrongPointer.h"
#include <utils/TypeHelpers.h>

// ---------------------------------------------------------------------------
namespace android {

class TextOutput;
TextOutput& printWeakPointer(TextOutput& to, const void* val);

// ---------------------------------------------------------------------------

#define COMPARE_WEAK(_op_)                                      \
inline bool operator _op_ (const wsp<T>& o) const {              \
    return m_ptr _op_ o.m_ptr;                                  \
}                                                               \
inline bool operator _op_ (const T* o) const {                  \
    return m_ptr _op_ o;                                        \
}                                                               \
template<typename U>                                            \
inline bool operator _op_ (const wsp<U>& o) const {              \
    return m_ptr _op_ o.m_ptr;                                  \
}                                                               \
template<typename U>                                            \
inline bool operator _op_ (const U* o) const {                  \
    return m_ptr _op_ o;                                        \
}

// ---------------------------------------------------------------------------

class WReferenceRenamer {
protected:
    // destructor is purposedly not virtual so we avoid code overhead from
    // subclasses; we have to make it protected to guarantee that it
    // cannot be called from this base class (and to make strict compilers
    // happy).
    ~WReferenceRenamer() { }
public:
    virtual void operator()(size_t i) const = 0;
};

// ---------------------------------------------------------------------------

class WRefBase
{
public:
            void            incStrong(const void* id) const;
            void            decStrong(const void* id) const;
    
            void            forceIncStrong(const void* id) const;

            //! DEBUGGING ONLY: Get current strong ref count.
            int32_t         getStrongCount() const;

    class weakref_type
    {
    public:
        WRefBase*            refBase() const;
        
        void                incWeak(const void* id);
        void                decWeak(const void* id);
        
        // acquires a strong reference if there is already one.
        bool                attemptIncStrong(const void* id);
        
        // acquires a weak reference if there is already one.
        // This is not always safe. see ProcessState.cpp and BpBinder.cpp
        // for proper use.
        bool                attemptIncWeak(const void* id);

        //! DEBUGGING ONLY: Get current weak ref count.
        int32_t             getWeakCount() const;

        //! DEBUGGING ONLY: Print references held on object.
        void                printRefs() const;

        //! DEBUGGING ONLY: Enable tracking for this object.
        // enable -- enable/disable tracking
        // retain -- when tracking is enable, if true, then we save a stack trace
        //           for each reference and dereference; when retain == false, we
        //           match up references and dereferences and keep only the 
        //           outstanding ones.
        
        void                trackMe(bool enable, bool retain);
    };
    
            weakref_type*   createWeak(const void* id) const;
            
            weakref_type*   getWeakRefs() const;

            //! DEBUGGING ONLY: Print references held on object.
    inline  void            printRefs() const { getWeakRefs()->printRefs(); }

            //! DEBUGGING ONLY: Enable tracking of object.
    inline  void            trackMe(bool enable, bool retain)
    { 
        getWeakRefs()->trackMe(enable, retain); 
    }

    typedef WRefBase basetype;

protected:
                            WRefBase();
    virtual                 ~WRefBase();
    
    //! Flags for extendObjectLifetime()
    enum {
        OBJECT_LIFETIME_STRONG  = 0x0000,
        OBJECT_LIFETIME_WEAK    = 0x0001,
        OBJECT_LIFETIME_MASK    = 0x0001
    };
    
            void            extendObjectLifetime(int32_t mode);
            
    //! Flags for onIncStrongAttempted()
    enum {
        FIRST_INC_STRONG = 0x0001
    };
    
    virtual void            onFirstRef();
    virtual void            onLastStrongRef(const void* id);
    virtual bool            onIncStrongAttempted(uint32_t flags, const void* id);
    virtual void            onLastWeakRef(const void* id);

private:
    friend class weakref_type;
    class weakref_impl;
    
                            WRefBase(const WRefBase& o);
            WRefBase&        operator=(const WRefBase& o);

private:
    friend class WWReferenceMover;

    static void renameRefs(size_t n, const WReferenceRenamer& renamer);

    static void renameRefId(weakref_type* ref,
            const void* old_id, const void* new_id);

    static void renameRefId(WRefBase* ref,
            const void* old_id, const void* new_id);

        weakref_impl* const mRefs;
};

// ---------------------------------------------------------------------------

template <class T>
class WLightWRefBase
{
public:
    inline WLightWRefBase() : mCount(0) { }
    inline void incStrong(__attribute__((unused)) const void* id) const {
        android_atomic_inc(&mCount);
    }
    inline void decStrong(__attribute__((unused)) const void* id) const {
        if (android_atomic_dec(&mCount) == 1) {
            delete static_cast<const T*>(this);
        }
    }
    //! DEBUGGING ONLY: Get current strong ref count.
    inline int32_t getStrongCount() const {
        return mCount;
    }

    typedef WLightWRefBase<T> basetype;

protected:
    inline ~WLightWRefBase() { }

private:
    friend class WReferenceMover;
    inline static void renameRefs(size_t n, const WReferenceRenamer& renamer) { }
    inline static void renameRefId(T* ref,
            const void* old_id, const void* new_id) { }

private:
    mutable volatile int32_t mCount;
};

// This is a wrapper around LightWRefBase that simply enforces a virtual
// destructor to eliminate the template requirement of LightWRefBase
class VirtualLightWRefBase : public WLightWRefBase<VirtualLightWRefBase> {
public:
    virtual ~VirtualLightWRefBase() {}
};

// ---------------------------------------------------------------------------

template <typename T>
class wwp
{
public:
    typedef typename WRefBase::weakref_type weakref_type;
    
    inline wwp() : m_ptr(0) { }

    wwp(T* other);
    wwp(const wwp<T>& other);
    wwp(const wsp<T>& other);
    template<typename U> wwp(U* other);
    template<typename U> wwp(const wsp<U>& other);
    template<typename U> wwp(const wwp<U>& other);

    ~wwp();
    
    // Assignment

    wwp& operator = (T* other);
    wwp& operator = (const wwp<T>& other);
    wwp& operator = (const wsp<T>& other);
    
    template<typename U> wwp& operator = (U* other);
    template<typename U> wwp& operator = (const wwp<U>& other);
    template<typename U> wwp& operator = (const wsp<U>& other);
    
    void set_object_and_refs(T* other, weakref_type* refs);

    // promotion to sp
    
    wsp<T> promote() const;

    // Reset
    
    void clear();

    // Accessors
    
    inline  weakref_type* get_refs() const { return m_refs; }
    
    inline  T* unsafe_get() const { return m_ptr; }

    // Operators

    COMPARE_WEAK(==)
    COMPARE_WEAK(!=)
    COMPARE_WEAK(>)
    COMPARE_WEAK(<)
    COMPARE_WEAK(<=)
    COMPARE_WEAK(>=)

    inline bool operator == (const wwp<T>& o) const {
        return (m_ptr == o.m_ptr) && (m_refs == o.m_refs);
    }
    template<typename U>
    inline bool operator == (const wwp<U>& o) const {
        return m_ptr == o.m_ptr;
    }

    inline bool operator > (const wwp<T>& o) const {
        return (m_ptr == o.m_ptr) ? (m_refs > o.m_refs) : (m_ptr > o.m_ptr);
    }
    template<typename U>
    inline bool operator > (const wwp<U>& o) const {
        return (m_ptr == o.m_ptr) ? (m_refs > o.m_refs) : (m_ptr > o.m_ptr);
    }

    inline bool operator < (const wwp<T>& o) const {
        return (m_ptr == o.m_ptr) ? (m_refs < o.m_refs) : (m_ptr < o.m_ptr);
    }
    template<typename U>
    inline bool operator < (const wwp<U>& o) const {
        return (m_ptr == o.m_ptr) ? (m_refs < o.m_refs) : (m_ptr < o.m_ptr);
    }
                         inline bool operator != (const wwp<T>& o) const { return m_refs != o.m_refs; }
    template<typename U> inline bool operator != (const wwp<U>& o) const { return !operator == (o); }
                         inline bool operator <= (const wwp<T>& o) const { return !operator > (o); }
    template<typename U> inline bool operator <= (const wwp<U>& o) const { return !operator > (o); }
                         inline bool operator >= (const wwp<T>& o) const { return !operator < (o); }
    template<typename U> inline bool operator >= (const wwp<U>& o) const { return !operator < (o); }

private:
    template<typename Y> friend class sp;
    template<typename Y> friend class wwp;

    T*              m_ptr;
    weakref_type*   m_refs;
};

template <typename T>
TextOutput& operator<<(TextOutput& to, const wwp<T>& val);

#undef COMPARE_WEAK

// ---------------------------------------------------------------------------

template<typename T>
wwp<T>::wwp(T* other)
    : m_ptr(other)
{
    if (other) m_refs = other->createWeak(this);
}

template<typename T>
wwp<T>::wwp(const wwp<T>& other)
    : m_ptr(other.m_ptr), m_refs(other.m_refs)
{
    if (m_ptr) m_refs->incWeak(this);
}

template<typename T>
wwp<T>::wwp(const wsp<T>& other)
    : m_ptr(other.m_ptr)
{
    if (m_ptr) {
        m_refs = m_ptr->createWeak(this);
    }
}

template<typename T> template<typename U>
wwp<T>::wwp(U* other)
    : m_ptr(other)
{
    if (other) m_refs = other->createWeak(this);
}

template<typename T> template<typename U>
wwp<T>::wwp(const wwp<U>& other)
    : m_ptr(other.m_ptr)
{
    if (m_ptr) {
        m_refs = other.m_refs;
        m_refs->incWeak(this);
    }
}

template<typename T> template<typename U>
wwp<T>::wwp(const wsp<U>& other)
    : m_ptr(other.m_ptr)
{
    if (m_ptr) {
        m_refs = m_ptr->createWeak(this);
    }
}

template<typename T>
wwp<T>::~wwp()
{
    if (m_ptr) m_refs->decWeak(this);
}

template<typename T>
wwp<T>& wwp<T>::operator = (T* other)
{
    CallStack stack(LOG_TAG);
    //stack.update();
    stack.log(LOG_TAG);
    weakref_type* newRefs =
        other ? other->createWeak(this) : 0;
    ALOGD("\n= m_ptr == NULL: %d", (m_ptr == NULL));
    if (m_ptr) m_refs->decWeak(this);
    ALOGD("newRef.getWeakCount: " + newRefs->getWeakCount());
    ALOGD("m_refs.getWeakCount: " + m_refs->getWeakCount());
    m_ptr = other;
    m_refs = newRefs;
    return *this;
}

template<typename T>
wwp<T>& wwp<T>::operator = (const wwp<T>& other)
{
    weakref_type* otherRefs(other.m_refs);
    T* otherPtr(other.m_ptr);
    if (otherPtr) otherRefs->incWeak(this);
    if (m_ptr) m_refs->decWeak(this);
    m_ptr = otherPtr;
    m_refs = otherRefs;
    return *this;
}

template<typename T>
wwp<T>& wwp<T>::operator = (const wsp<T>& other)
{
    weakref_type* newRefs =
        other != NULL ? other->createWeak(this) : 0;
    T* otherPtr(other.m_ptr);
    if (m_ptr) m_refs->decWeak(this);
    m_ptr = otherPtr;
    m_refs = newRefs;
    return *this;
}

template<typename T> template<typename U>
wwp<T>& wwp<T>::operator = (U* other)
{
    weakref_type* newRefs =
        other ? other->createWeak(this) : 0;
    if (m_ptr) m_refs->decWeak(this);
    m_ptr = other;
    m_refs = newRefs;
    return *this;
}

template<typename T> template<typename U>
wwp<T>& wwp<T>::operator = (const wwp<U>& other)
{
    weakref_type* otherRefs(other.m_refs);
    U* otherPtr(other.m_ptr);
    if (otherPtr) otherRefs->incWeak(this);
    if (m_ptr) m_refs->decWeak(this);
    m_ptr = otherPtr;
    m_refs = otherRefs;
    return *this;
}

template<typename T> template<typename U>
wwp<T>& wwp<T>::operator = (const wsp<U>& other)
{
    weakref_type* newRefs =
        other != NULL ? other->createWeak(this) : 0;
    U* otherPtr(other.m_ptr);
    if (m_ptr) m_refs->decWeak(this);
    m_ptr = otherPtr;
    m_refs = newRefs;
    return *this;
}

template<typename T>
void wwp<T>::set_object_and_refs(T* other, weakref_type* refs)
{
    if (other) refs->incWeak(this);
    if (m_ptr) m_refs->decWeak(this);
    m_ptr = other;
    m_refs = refs;
}

template<typename T>
wsp<T> wwp<T>::promote() const
{
    wsp<T> result;
    if (m_ptr && m_refs->attemptIncStrong(&result)) {
        result.set_pointer(m_ptr);
    }
    return result;
}

template<typename T>
void wwp<T>::clear()
{
    if (m_ptr) {
        m_refs->decWeak(this);
        m_ptr = 0;
    }
}

template <typename T>
inline TextOutput& operator<<(TextOutput& to, const wwp<T>& val)
{
    return printWeakPointer(to, val.unsafe_get());
}



// ---------------------------------------------------------------------------

// this class just serves as a namespace so TYPE::moveReferences can stay
// private.
class WReferenceMover {
public:
    // it would be nice if we could make sure no extra code is generated
    // for sp<TYPE> or wp<TYPE> when TYPE is a descendant of WRefBase:
    // Using a sp<WRefBase> override doesn't work; it's a bit like we wanted
    // a template<typename TYPE inherits WRefBase> template...

    template<typename TYPE> static inline
    void move_references(wsp<TYPE>* d, wsp<TYPE> const* s, size_t n) {

        class Renamer : public WReferenceRenamer {
            wsp<TYPE>* d;
            wsp<TYPE> const* s;
            virtual void operator()(size_t i) const {
                // The id are known to be the sp<>'s this pointer
                TYPE::renameRefId(d[i].get(), &s[i], &d[i]);
            }
        public:
            Renamer(wsp<TYPE>* d, wsp<TYPE> const* s) : s(s), d(d) { }
        };

        memmove(d, s, n*sizeof(wsp<TYPE>));
        TYPE::renameRefs(n, Renamer(d, s));
    }


    template<typename TYPE> static inline
    void move_references(wwp<TYPE>* d, wwp<TYPE> const* s, size_t n) {

        class Renamer : public WReferenceRenamer {
            wwp<TYPE>* d;
            wwp<TYPE> const* s;
            virtual void operator()(size_t i) const {
                // The id are known to be the wwp<>'s this pointer
                TYPE::renameRefId(d[i].get_refs(), &s[i], &d[i]);
            }
        public:
            Renamer(wwp<TYPE>* d, wwp<TYPE> const* s) : s(s), d(d) { }
        };

        memmove(d, s, n*sizeof(wwp<TYPE>));
        TYPE::renameRefs(n, Renamer(d, s));
    }
};

// specialization for moving sp<> and wp<> types.
// these are used by the [Sorted|Keyed]Vector<> implementations
// sp<> and wp<> need to be handled specially, because they do not
// have trivial copy operation in the general case (see WRefBase.cpp
// when DEBUG ops are enabled), but can be implemented very
// efficiently in most cases.

template<typename TYPE> inline
void move_forward_type(wsp<TYPE>* d, wsp<TYPE> const* s, size_t n) {
    WReferenceMover::move_references(d, s, n);
}

template<typename TYPE> inline
void move_backward_type(wsp<TYPE>* d, wsp<TYPE> const* s, size_t n) {
    WReferenceMover::move_references(d, s, n);
}

template<typename TYPE> inline
void move_forward_type(wwp<TYPE>* d, wwp<TYPE> const* s, size_t n) {
    WReferenceMover::move_references(d, s, n);
}

template<typename TYPE> inline
void move_backward_type(wwp<TYPE>* d, wwp<TYPE> const* s, size_t n) {
    WReferenceMover::move_references(d, s, n);
}


}; // namespace android

// ---------------------------------------------------------------------------

#endif // ANDROID_WREF_BASE_H
