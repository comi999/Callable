#pragma once
#include <vector>

#include "FunctionTraits.hpp"

// Static storage object for a member function pointer. Use as MemberFunction<&Object::Member>{}.
template < auto _Function >
struct MemberFunction {};

// Helpers for invoker types.
namespace InvokerHelpers
{
    using LambdaTagType = uintptr_t;
    using LambdaProcessorType = void( * )( void*&, bool );
    using LambdaOffsetType = size_t;
    
    static constexpr LambdaTagType LambdaStorageTag = ( LambdaOffsetType )0xABCDEFABCDEFABCD;
    static constexpr LambdaOffsetType LambdaTagOffset = 0u;
    static constexpr LambdaOffsetType LambdaProcessorOffset = LambdaTagOffset + sizeof( LambdaTagOffset );
    static constexpr LambdaOffsetType LambdaOffset = LambdaProcessorOffset + sizeof( LambdaProcessorOffset );

    template < typename T >
    static void LambdaProcessor( void*&, bool );

    template < typename T >
    struct LambdaStorage
    {
        using LambdaType = T;

        LambdaTagType Tag = LambdaStorageTag;
        LambdaProcessorType Processor = LambdaProcessor< T >;
        LambdaType Lambda;

        template < typename... Args >
        LambdaStorage( Args&&... a_Args )
            : Lambda( std::forward< Args >( a_Args )... )
        {}
    };

    template < typename T >
    static void LambdaProcessor( void*& a_Pointer, bool a_Copy )
    {
        if ( a_Copy )
        {
            a_Pointer = new LambdaStorage< T >( ( ( LambdaStorage< T >*& )a_Pointer )->Lambda );
        }
        else
        {
            delete ( LambdaStorage< T >*& )a_Pointer;
        }
    }

    static bool IsLambdaStorage( void* a_Pointer )
    {
        return *( LambdaTagType* )a_Pointer == LambdaStorageTag;
    }

    static void* GetLambda( void* a_Pointer )
    {
        return ( uint8_t* )a_Pointer + LambdaOffset;
    }

    static LambdaProcessorType GetLambdaProcessor( void* a_Pointer )
    {
        return *( LambdaProcessorType* )( ( uint8_t* )a_Pointer + LambdaProcessorOffset );
    }

    static void DestroyLambdaStorage( void* a_Pointer )
    {
        GetLambdaProcessor( a_Pointer )( a_Pointer, false );
    }

    static void CopyLambdaStorage( void*& a_Pointer )
    {
        GetLambdaProcessor( a_Pointer )( a_Pointer, true );
    }
}

//==========================================================================
// An invoker is a function object that can be bound to the following:
// - Static functions
// - Member functions and accompanying objects
// - Static lambda
// - Capture lambda
// - Invocable object.
//==========================================================================
template < typename Return = void, typename... Args >
class Invoker
{
private:

    template < auto _Function, bool _UsingLambdaStorage = false >
    static Return Invocation( void* a_Object, Args... a_Args )
    {
        using ObjectType = FunctionTraits::GetObject< decltype( _Function ) >;

        if constexpr ( _UsingLambdaStorage )
        {
            a_Object = InvokerHelpers::GetLambda( a_Object );
        }

        return ( reinterpret_cast< ObjectType* >( a_Object )->*_Function )( std::forward< Args >( a_Args )... );
    }

public:

    // Create an empty invoker.
    Invoker()
        : m_Object( nullptr )
        , m_Function( nullptr )
    {}

    // Will unbind any bound functions or functors.
    ~Invoker()
    {
        Unbind();
    }

    // Create an invoker from a callable object. This can be a static function, member function or lambda.
    template < typename Object, auto _Function >
    Invoker( Object* a_Object, MemberFunction< _Function > ) : Invoker() { Bind< _Function >( a_Object ); }

    // Create an invoker from a callable object. This can be a static function, or functor type.
    template < typename T >
    Invoker( T&& a_Function ) : Invoker() { Bind( std::forward< T >( a_Function ) ); }

    // Bind a static function to the invoker.
    void Bind( Return( *a_Function )( Args... ) )
    {
        Unbind();

        m_Function = a_Function;
    }

    // Statically binds a member function along with an object instance to the invoker.
    template < auto _Function, typename Object >
    void Bind( Object* a_Object, MemberFunction< _Function > a_Function = MemberFunction< _Function >{} )
    {
        using FunctionType = decltype( _Function );
        using ObjectType = std::remove_const_t< Object >;
        
        static_assert( FunctionTraits::IsDynamic< FunctionType >, "Function must be a member function." );
        static_assert( std::is_same_v< FunctionTraits::GetObject< FunctionType >, ObjectType >, "Member function object type must be the same as the pointer type provided." );
        static_assert( FunctionTraits::IsConst< FunctionType > || !std::is_const_v< Object >, "A non-const member function can not be called on a const object." );
        static_assert( FunctionTraits::IsVolatile< FunctionType > || !std::is_volatile_v< Object >, "A non-volatile member function can not be called on a volatile object." );
        
        Unbind();

        m_Object = const_cast< ObjectType* >( a_Object );
        m_Function = ( void* )Invocation< _Function >;
    }

private:

    template < auto _Function, typename Object >
    void Bind( InvokerHelpers::LambdaStorage< Object >* a_Object, MemberFunction< _Function > a_Function = MemberFunction< _Function >{} )
    {
        using FunctionType = decltype( _Function );
        using ObjectType = Object;

        static_assert( FunctionTraits::IsDynamic< FunctionType >, "Function must be a member function." );
        static_assert( std::is_same_v< FunctionTraits::GetObject< FunctionType >, ObjectType >, "Member function object type must be the same as the pointer type provided." );

        Unbind();

        m_Object = a_Object;
        m_Function = ( void* )Invocation< _Function, true >;
    }

public:

    // Binds a function or functor to the invoker. Static lambdas will be decayed into a static function pointer. If moving from a functor, functor instance
    // will be stored.
    template < typename T >
    void Bind( T&& a_Function )
    {
        using FunctorType = std::remove_reference_t< T >;

        if constexpr ( std::is_same_v< FunctorType, Invoker > )
        {
            if constexpr ( std::is_rvalue_reference_v< decltype( a_Function ) > )
            {
                m_Object = a_Function.m_Object;
                m_Function = a_Function.m_Function;
                a_Function.m_Object = nullptr;
                a_Function.m_Function = nullptr;
            }
            else
            {
                if ( InvokerHelpers::IsLambdaStorage( a_Function.m_Object ) )
                {
                    InvokerHelpers::CopyLambdaStorage( m_Object = a_Function.m_Object );
                    m_Function = a_Function.m_Function;
                }
                else
                {
                    m_Object = a_Function.m_Object;
                    m_Function = a_Function.m_Function;
                }
            }
        }
        else if constexpr ( std::is_convertible_v < FunctorType, Return( * )( Args... ) > )
        {
            Bind( static_cast< Return( * )( Args... ) >( a_Function ) );
        }
        else
        {
            static_assert( FunctionTraits::IsConst< decltype( &FunctorType::operator() ) > || !std::is_const_v< FunctorType >, "A non-const member function can not be called on a const object." );

            if constexpr ( std::is_rvalue_reference_v< decltype( a_Function ) > )
            {
                Bind< &FunctorType::operator() >( new InvokerHelpers::LambdaStorage< FunctorType >( std::move( a_Function ) ) );
            }
            else
            {
                Bind< &FunctorType::operator() >( &a_Function );
            }
        }
    }

    // Clear invoker binding.
    void Unbind()
    {
        if ( m_Object && InvokerHelpers::IsLambdaStorage( m_Object ) )
        {
            InvokerHelpers::DestroyLambdaStorage( m_Object );
        }

        m_Object = nullptr;
        m_Function = nullptr;
    }

    // Is the invoker bound to a functor or function?
    inline bool IsBound() const { return m_Function; }

    // Is the bound function, if any at all, a static function?
    inline bool IsStatic() const { return !m_Object && m_Function; }

    // Is the bound function, if any at all, a member function?
    inline bool IsMember() const { return m_Object && !InvokerHelpers::IsLambdaStorage( m_Object ); }

    // Is the bound function, if any at all, a lambda?
    inline bool IsLambda() const { return m_Object && InvokerHelpers::IsLambdaStorage( m_Object ); }

    // Invoke the stored callable. _Safe set to true will make sure that
    // the invoker is only called if it is bound to a functor or function. Default Return if not.
    template < bool _Safe = false >
    Return Invoke( Args... a_Args ) const
    {
        if constexpr ( _Safe )
        {
            if ( !m_Function )
            {
                return Return();
            }
        }

        return m_Object ?
            reinterpret_cast< Return( * )( void*, Args... ) >( m_Function )( m_Object, std::forward< Args >( a_Args )... ) :
            reinterpret_cast< Return( * )(        Args... ) >( m_Function )(           std::forward< Args >( a_Args )... ) ;
    }

    // Invoke the stored callable. This will fail if invoker is not bound to a functor or function.
    Return operator()( Args... a_Args ) const
    {
        return m_Object ?
            reinterpret_cast< Return( * )( void*, Args... ) >( m_Function )( m_Object, std::forward< Args >( a_Args )... ) :
            reinterpret_cast< Return( * )(        Args... ) >( m_Function )(           std::forward< Args >( a_Args )... ) ;
    }

    // Is the invoker bound to a functor or function?
    inline operator bool() const
    {
        return m_Function;
    }

    // Checks to see if the invoker is bound to the same functor or function and instance as another invoker.
    inline bool operator==( const Invoker& a_Invoker ) const { return m_Function == a_Invoker.m_Function && m_Object == a_Invoker.m_Object; }

    // Checks to see if the invoker is bound to the same functor or function and instance as another invoker.
    inline bool operator==( Invoker&& a_Invoker ) const { return m_Function == a_Invoker.m_Function && m_Object == a_Invoker.m_Object; }

    // Checks to see if the invokers bound function is the same as given static function.
    inline bool operator==( Return( *a_Function )( Args... ) ) const { return m_Function == a_Function; }

    // Checks to see if the invokers bound object is the same as the given object.
    template < typename T >
    inline bool operator==( T* a_Object ) const { return m_Object == a_Object; }

    // Checks to see if the invoker is bound at all. Same as IsBound and operator bool.
    inline bool operator==( std::nullptr_t ) const { return m_Object == nullptr; }

    // Checks to see if the invoker is bound to the same functor or function and instance as the one given.
    template < typename T >
    inline bool operator==( T&& a_Function ) const { return *this == Invoker( std::forward< T >( a_Function ) ); }

    // Checks to see if the invoker is bound to the same member function as the one provided.
    template < auto _Function >
    inline bool operator==( MemberFunction< _Function > ) const { return m_Function == static_cast< void* >( Invocation< _Function > ); }

    // Checks to see if the invoker is bound to a different function or functor than the one given.
    template < typename T >
    inline bool operator!=( const T& a_Function ) const { return !( *this == a_Function ); }

    // Clear invoker binding. Same as Unbind.
    inline Invoker& operator=( std::nullptr_t ) { Unbind(); return *this; }

    // Assign a functor or function to the invoker.
    template < typename T >
    inline Invoker& operator=( T&& a_Functor ) { Bind( std::forward< T >( a_Functor ) ); return *this; }

private:

    void* m_Object;
    void* m_Function;
};

// An Action is an invoker that returns void.
template < typename... Args >
using Action = Invoker< void, Args... >;

// A Predicate is an invoker that returns bool.
template < typename... Args >
using Predicate = Invoker< bool, Args... >;