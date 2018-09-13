#ifndef STATUS_HPP
#define STATUS_HPP

#include "using_upnp_namespace.hpp"
#include "upnp_global.hpp"

START_DEFINE_UPNP_NAMESPACE

/*! \brief Holds information to simulate a set of 32 bits.
 *
 * It exists to simulate 2 states: exist or not.
 */
class UPNP_API CStatus
{
public:
  /*! Default constructor. */
  inline CStatus () {}

  /*! Constructs with a st value. */
  inline CStatus (unsigned st);

  /*! Copy constructor. */
  inline CStatus (CStatus const & other);

  /*! Equal operator. */
  inline CStatus& operator = (CStatus const & other);

  /*! Equal operator. */
  inline CStatus& operator = (unsigned st);

  /*! Equality operator. */
  inline bool operator == (CStatus const & other) const;

  /*! Inequality operator. */
  inline bool operator != (CStatus const & other) const;

  /*! Adds a set of bits. */
  inline void addStatus (unsigned st);

  /*! Sets a set of bits. */
  inline void setStatus (unsigned st);

  /*! Removes a set of bits. */
  inline void remStatus (unsigned st);

   /*! Has a set of bits. */
  inline bool hasStatus (unsigned st) const;

  /*! Return the set of bits. */
  inline unsigned status () const;

protected :
  unsigned m_status = 0; //!< The 32 bits value of status.
};

CStatus::CStatus (unsigned st) : m_status (st)
{
}

CStatus::CStatus (CStatus const & other) : m_status (other.m_status)
{
}

CStatus& CStatus::operator = (CStatus const & other)
{
  m_status = other.m_status;
  return *this;
}

CStatus& CStatus::operator = (unsigned st)
{
  m_status = st;
  return *this;
}

void CStatus::addStatus (unsigned st)
{
  m_status |= st;
}

void CStatus::setStatus (unsigned st)
{
  m_status = st;
}

void CStatus::remStatus (unsigned st)
{
  m_status &= ~st;
}

bool CStatus::hasStatus (unsigned st) const
{
  return (m_status & st) != 0;
}

unsigned CStatus::status () const
{
  return m_status;
}

bool CStatus::operator == (CStatus const & other) const
{
  return m_status == other.m_status;
}

bool CStatus::operator != (CStatus const & other) const
{
  return m_status != other.m_status;
}

/*! \brief Related non-members equal operator. */
inline bool operator == (CStatus const & s1, CStatus const & s2)
{
  return s1.status () == s2.status ();
}

/*! \brief Related non-members not equal operator. */
inline bool operator != (CStatus const & s1, CStatus const & s2)
{
  return s1.status () != s2.status ();
}

} // Namespace

#endif // STATUS_HPP
