#pragma once
#include <vector>

#include "Invoker.hpp"

//==========================================================================
// Delegates are a collection of stored invokers.
//==========================================================================
template < typename Return = void, typename... Args >
class Delegate
{
private:

    using ContainerType = std::vector< Invoker< Return, Args... > >;

public:

    // Create an empty delegate.
    Delegate()
        : m_IsBroadcasting( false )
        , m_Index( -1 )
    {}

    // Copies from a provided delegate.
    Delegate( const Delegate& a_Delegate )
        : m_Invokers( a_Delegate.m_Invokers )
        , m_IsBroadcasting( false )
        , m_Index( -1 )
    {}

    // Moves from a provided delegate.
    Delegate( Delegate&& a_Delegate )
        : m_Invokers( std::move( a_Delegate.m_Invokers ) )
        , m_IsBroadcasting( false )
        , m_Index( -1 )
    {
        a_Delegate.m_Index = -1;
    }

    // Add a functor or function to the delegate.
    template < typename T >
    inline void Add( T& a_Function ) { m_Invokers.emplace_back( a_Function ); }

    // Add a functor or function to the delegate.
    template < typename T >
    void Add( T&& a_Function ) { m_Invokers.emplace_back( std::forward< T >( a_Function ) ); }

    // Add an instance and member function to the delegate.
    template < auto _Function, typename Object >
    void Add( Object* a_Object, MemberFunction< _Function > a_Function = MemberFunction< _Function >{} ) { m_Invokers.emplace_back( a_Object, a_Function ); }
    
    // Add a functor or function to the delegate at the given index.
    template < typename T >
    void Add( size_t a_Index, T& a_Function )
    {
        if ( m_IsBroadcasting && a_Index <= m_Index )
        {
            ++m_Index;
        }

        m_Invokers.emplace( m_Invokers.begin() + a_Index, a_Function );
    }

    // Add a functor or function to the delegate at the given index.
    template < typename T >
    void Add( size_t a_Index, T&& a_Function )
    {
        if ( m_IsBroadcasting && a_Index <= m_Index )
        {
            ++m_Index;
        }

        m_Invokers.emplace( m_Invokers.begin() + a_Index, std::forward< T >( a_Function ) );
    }

    // Add an instance and member function to the delegate at the given index.
    template < auto _Function, typename Object >
    void Add( size_t a_Index, Object* a_Object, MemberFunction< _Function > a_Function = MemberFunction< _Function >{} )
    {
        if ( m_IsBroadcasting && a_Index <= m_Index )
        {
            ++m_Index;
        }

        m_Invokers.emplace_back( a_Object, a_Function );
    }

    // Add a functor or function to the delegate if it isn't already added to the delegate.
    template < typename T >
    void AddUnique( T& a_Function )
    {
        if ( std::find( m_Invokers.begin(), m_Invokers.end(), a_Function ) != m_Invokers.end() )
        {
            return;
        }

        m_Invokers.emplace_back( a_Function );
    }

    // Add a functor or function to the delegate if it isn't already added to the delegate.
    template < typename T >
    void AddUnique( T&& a_Function )
    {
        if ( std::find( m_Invokers.begin(), m_Invokers.end(), std::forward< T >( a_Function ) ) != m_Invokers.end() )
        {
            return;
        }

        m_Invokers.emplace_back( std::forward< T >( a_Function ) );
    }

    // Add an instance and member function to the delegate if it isn't already added to the delegate.
    template < auto _Function, typename Object >
    void AddUnique( Object* a_Object, MemberFunction< _Function > a_Function = MemberFunction< _Function >{} )
    {
        if ( std::find( m_Invokers.begin(), m_Invokers.end(), Invoker< Return, Args... >( a_Object, a_Function ) ) != m_Invokers.end() )
        {
            return;
        }

        m_Invokers.emplace_back( a_Object, a_Function );
    }

    // Add a functor or function to the delegate if it isn't already added to the delegate, at the given index.
    template < typename T >
    void AddUnique( size_t a_Index, T& a_Function )
    {
        if ( std::find( m_Invokers.begin(), m_Invokers.end(), a_Function ) != m_Invokers.end() )
        {
            return;
        }

        if ( m_IsBroadcasting && a_Index <= m_Index )
        {
            ++m_Index;
        }

        m_Invokers.emplace( m_Invokers.begin() + a_Index, a_Function );
    }

    // Add a functor or function to the delegate if it isn't already added to the delegate, at the given index.
    template < typename T >
    void AddUnique( size_t a_Index, T&& a_Function )
    {
        if ( std::find( m_Invokers.begin(), m_Invokers.end(), std::forward< T >( a_Function ) ) != m_Invokers.end() )
        {
            return;
        }

        if ( m_IsBroadcasting && a_Index <= m_Index )
        {
            ++m_Index;
        }

        m_Invokers.emplace( m_Invokers.begin() + a_Index, std::forward< T >( a_Function ) );
    }

    // Add an instance and member function to the delegate if it isn't already added to the delegate, at the given index.
    template < auto _Function, typename Object >
    void AddUnique( size_t a_Index, Object* a_Object, MemberFunction< _Function > a_Function = MemberFunction< _Function >{} )
    {
        if ( std::find( m_Invokers.begin(), m_Invokers.end(), Invoker< Return, Args... >( a_Object, a_Function ) ) != m_Invokers.end() )
        {
            return;
        }

        if ( m_IsBroadcasting && a_Index <= m_Index )
        {
            ++m_Index;
        }

        m_Invokers.emplace_back( a_Object, a_Function );
    }

    // Remove a functor or function from the delegate.
    template < typename T >
    void Remove( T& a_Function )
    {
        auto Found = std::find( m_Invokers.begin(), m_Invokers.end(), a_Function );

        if ( Found != m_Invokers.end() )
        {
            if ( m_IsBroadcasting && ( Found - m_Invokers.begin() ) <= m_Index )
            {
                --m_Index;
            }

            *Found = m_Invokers.back();
            m_Invokers.pop_back();
        }
    }

    // Remove a functor or function from the delegate.
    template < typename T >
    void Remove( T&& a_Function )
    {
        auto Found = std::find( m_Invokers.begin(), m_Invokers.end(), std::forward< T >( a_Function ) );

        if ( Found != m_Invokers.end() )
        {
            if ( m_IsBroadcasting && ( Found - m_Invokers.begin() ) <= m_Index )
            {
                --m_Index;
            }

            *Found = m_Invokers.back();
            m_Invokers.pop_back();
        }
    }

    // Remove an instance and member function from the delegate.
    template < auto _Function, typename Object >
    void Remove( Object* a_Object, MemberFunction< _Function > a_Function = MemberFunction< _Function >{} )
    {
        auto Found = std::find( m_Invokers.begin(), m_Invokers.end(), Invoker< Return, Args... >( a_Object, a_Function ) );

        if ( Found != m_Invokers.end() )
        {
            if ( m_IsBroadcasting && ( Found - m_Invokers.begin() ) <= m_Index )
            {
                --m_Index;
            }

            *Found = m_Invokers.back();
            m_Invokers.pop_back();
        }
    }
    
    // Remove an invoker from the delegate at the given index.
    void Remove( size_t a_Index )
    {
        if ( m_IsBroadcasting && a_Index <= m_Index )
        {
            --m_Index;
        }

        m_Invokers[ a_Index ] = m_Invokers.back();
        m_Invokers.pop_back();
    }

    // Remove all invokers from the delegate that match the given functor or function.
    template < typename T >
    void RemoveAll( T& a_Function )
    {
        for ( int32_t i = m_Invokers.size() - 1; i > m_Index; --i )
        {
            if ( m_Invokers[ i ] == a_Function )
            {
                m_Invokers[ i ] = m_Invokers.back();
                m_Invokers.pop_back();
            }
        }

        for ( int32_t i = m_Index; i >= 0; --i )
        {
            if ( m_Invokers[ i ] == a_Function )
            {
                m_Invokers[ i ] = m_Invokers.back();
                m_Invokers.pop_back();
                --m_Index;
            }
        }
    }

    // Remove all invokers from the delegate that match the given functor or function.
    template < typename T >
    void RemoveAll( T&& a_Function )
    {
        for ( int32_t i = m_Invokers.size() - 1; i > m_Index; --i )
        {
            if ( m_Invokers[ i ] == std::forward< T >( a_Function ) )
            {
                m_Invokers[ i ] = m_Invokers.back();
                m_Invokers.pop_back();
            }
        }

        for ( int32_t i = m_Index; i >= 0; --i )
        {
            if ( m_Invokers[ i ] == std::forward< T >( a_Function ) )
            {
                m_Invokers[ i ] = m_Invokers.back();
                m_Invokers.pop_back();
                --m_Index;
            }
        }
    }

    // Remove all invokers from the delegate that match the given instance and member function.
    template < auto _Function, typename Object >
    void RemoveAll( Object* a_Object, MemberFunction< _Function > a_Function = MemberFunction< _Function >{} )
    {
        Invoker< Return, Args... > Check( a_Object, a_Function );
        for ( uint32_t i = m_Invokers.size() - 1; i > m_Index; --i )
        {
            if ( m_Invokers[ i ] == Check )
            {
                m_Invokers[ i ] = m_Invokers.back();
                m_Invokers.pop_back();
            }
        }

        for ( uint32_t i = m_Index; i >= 0; --i )
        {
            if ( m_Invokers[ i ] == Check )
            {
                m_Invokers[ i ] = m_Invokers.back();
                m_Invokers.pop_back();
                --m_Index;
            }
        }
    }

    // Call all contained invokers with the given arguments. _Safe set to true will call invokers safely.
    template < bool _Safe = false >
    void Broadcast( Args... a_Args ) const
    {
        if ( m_IsBroadcasting )
        {
            return;
        }

        const_cast< Delegate* >( this )->m_IsBroadcasting = true;
        const_cast< Delegate* >( this )->m_Index = 0;

        for ( ; m_Index < m_Invokers.size(); ++const_cast< Delegate* >( this )->m_Index )
        {
            m_Invokers[ m_Index ].Invoke< _Safe >( std::forward< Args >( a_Args )... );
        }

        const_cast< Delegate* >( this )->m_IsBroadcasting = false;
        const_cast< Delegate* >( this )->m_Index = -1;
    }

    // Call all contained invokers with the given arguments. Invokers will be called unsafely.
    void operator()( Args... a_Args ) const
    {
        if ( m_IsBroadcasting )
        {
            return;
        }

        const_cast< Delegate* >( this )->m_IsBroadcasting = true;
        const_cast< Delegate* >( this )->m_Index = 0;

        for ( ; m_Index < m_Invokers.size(); ++const_cast< Delegate* >( this )->m_Index )
        {
            m_Invokers[ m_Index ].Invoke< false >( std::forward< Args >( a_Args )... );
        }

        const_cast< Delegate* >( this )->m_IsBroadcasting = false;
        const_cast< Delegate* >( this )->m_Index = -1;
    }

    // Clear the delegate.
    inline void Clear() { m_Invokers.clear(); m_Index = -1; }

    // Is the delegate currently broadcasting.
    inline bool IsBroadcasting() const { return m_IsBroadcasting; }

    // The count of stored invokers.
    inline size_t Size() const { return m_Invokers.size(); }

    // Copy from another delegate.
    Delegate& operator=( const Delegate& a_Delegate )
    {
        m_Invokers = a_Delegate.m_Invokers;
        m_IsBroadcasting = false;
        m_Index = -1;
        return *this;
    }

    // Move from another delegate.
    Delegate& operator=( Delegate&& a_Delegate )
    {
        m_Invokers = std::move( a_Delegate.m_Invokers );
        m_IsBroadcasting = false;
        m_Index = -1;
        a_Delegate.m_Index = -1;
        return *this;
    }

    // Add a functor or function object to the delegate.
    template < typename T >
    inline Delegate& operator+=( T& a_Function ) { Add( a_Function ); return *this; }

    // Add a functor or function object to the delegate.
    template < typename T >
    inline Delegate& operator+=( T&& a_Function ) { Add( std::forward< T >( a_Function ) ); return *this; }

    // Remove a functor or function object to the delegate.
    template < typename T >
    inline Delegate& operator-=( T& a_Function ) { Remove( a_Function ); return *this; }

    // Remove a functor or function object to the delegate.
    template < typename T >
    inline Delegate& operator-=( T&& a_Function ) { Remove( std::forward< T >( a_Function ) ); return *this; }

    // Get the stored invoker at a given index.
    template < typename T >
    inline Invoker< Return, Args... >& operator[]( size_t a_Index ) { return m_Invokers[ a_Index ]; }

    // Get the stored invoker at a given index.
    template < typename T >
    inline const Invoker< Return, Args... >& operator[]( size_t a_Index ) const { return m_Invokers[ a_Index ]; }

private:

    ContainerType m_Invokers;
    bool          m_IsBroadcasting;
    int32_t       m_Index;
};