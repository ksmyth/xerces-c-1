/*
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 1999-2000 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Xerces" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache\@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation, and was
 * originally based on software copyright (c) 1999, International
 * Business Machines, Inc., http://www.ibm.com .  For more information
 * on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

/*
 * $Log$
 * Revision 1.1  2001/02/26 19:44:25  tng
 * Schema: add utility class QName, by Pei Yong Zhang.
 *
  */

#include <util/QName.hpp>

// ---------------------------------------------------------------------------
//  QName: Constructors and Destructor
// ---------------------------------------------------------------------------
QName::QName() :
      fPrefix(0)
    , fPrefixBufSz(0)
    , fLocalPart(0)
    , fLocalPartBufSz(0)
    , fRawName(0)
    , fRawNameBufSz(0)
    , fURIId(0)
{
}

QName::QName(
                 const XMLCh* const        prefix
               , const XMLCh* const        localPart
               , const int                 uriId
            ) :
      fPrefix(0)
    , fPrefixBufSz(0)
    , fLocalPart(0)
    , fLocalPartBufSz(0)
    , fRawName(0)
    , fRawNameBufSz(0)
    , fURIId(0)
{
    try
    {
        //
        //  Just call the local setters to set up everything. Too much
        //  work is required to replicate that functionality here.
        //
        setName(prefix, localPart, uriId);
    }

    catch(...)
    {
        cleanUp();
    }
}

QName::~QName()
{
	cleanUp();
}

// ---------------------------------------------------------------------------
//  QName: Copy Constructors
// ---------------------------------------------------------------------------
QName::QName(const QName& qname)
{
    unsigned int newLen;

    newLen = XMLString::stringLen(qname.getLocalPart());
    fLocalPartBufSz = newLen + 8;
    fLocalPart = new XMLCh[fLocalPartBufSz + 1];
    XMLString::moveChars(fLocalPart, qname.getLocalPart(), newLen + 1);

    newLen = XMLString::stringLen(qname.getPrefix());
    fPrefixBufSz = newLen + 8;
    fPrefix = new XMLCh[fPrefixBufSz + 1];
    XMLString::moveChars(fPrefix, qname.getPrefix(), newLen + 1);

    *fRawName = 0;

    fURIId = qname.getURI();
}

// ---------------------------------------------------------------------------
//  QName: Getter methods
// ---------------------------------------------------------------------------
inline const XMLCh* QName::getPrefix() const
{
	return fPrefix;
}

inline const XMLCh* QName::getLocalPart() const
{
	return fLocalPart;
}

inline const int QName::getURI() const
{
	return fURIId;
}

const XMLCh* QName::getRawName() const
{
    //
    //  If there is no buffer, or if there is but we've not faulted in the
    //  value yet, then we have to do that now.
    //
    if (!fRawName || !*fRawName)
    {
        //
        //  Calculate the worst case size buffer we will need. We use the
        //  current high water marks of the prefix and name buffers, so it
        //  might be a little wasteful of memory but we don't have to do
        //  string len operations on the two strings.
        //
        const unsigned int neededLen = fPrefixBufSz + fLocalPartBufSz + 1;

        //
        //  If no buffer, or the current one is too small, then allocate one
        //  and get rid of any old one.
        //
        if (!fRawName || (neededLen > fRawNameBufSz))
        {
            delete [] fRawName;

            // We have to cast off the const'ness to do this
            ((QName*)this)->fRawNameBufSz = neededLen;
            ((QName*)this)->fRawName = new XMLCh[neededLen + 1];

            // Make sure its initially empty
            *fRawName = 0;
        }

        //
        //  If we have a prefix, then do the prefix:name version. Else, its
        //  just the name.
        //
        if (*fPrefix)
        {
            const XMLCh colonStr[] = { chColon, chNull };
            XMLString::copyString(fRawName, fPrefix);
            XMLString::catString(fRawName, colonStr);
            XMLString::catString(fRawName, fLocalPart);
        }
         else
        {
            XMLString::copyString(fRawName, fLocalPart);
        }
    }
    return fRawName;
}

// ---------------------------------------------------------------------------
//  QName: Setter methods
// ---------------------------------------------------------------------------
void QName::setName(      const XMLCh* const    localPart
                        , const XMLCh* const    prefix
						, const int             uriId)
{
    unsigned int newLen;

    newLen = XMLString::stringLen(localPart);
    if (!fLocalPartBufSz || (newLen > fLocalPartBufSz))
    {
        delete [] fLocalPart;
        fLocalPartBufSz = newLen + 8;
        fLocalPart = new XMLCh[fLocalPartBufSz + 1];
    }
    XMLString::moveChars(fLocalPart, localPart, newLen + 1);

    newLen = XMLString::stringLen(prefix);
    if (!fPrefixBufSz || (newLen > fPrefixBufSz))
    {
        delete [] fPrefix;
        fPrefixBufSz = newLen + 8;
        fPrefix = new XMLCh[fPrefixBufSz + 1];
    }
    XMLString::moveChars(fPrefix, prefix, newLen + 1);

    // And clean up any QName and leave it undone until/if asked for again
    if (fRawName)
        *fRawName = 0;

    // And finally store the URI id parameter
    fURIId = uriId;
}

void QName::setPrefix(const XMLCh* prefix)
{
    unsigned int newLen;

    newLen = XMLString::stringLen(prefix);
    if (!fPrefixBufSz || (newLen > fPrefixBufSz))
    {
        delete [] fPrefix;
        fPrefixBufSz = newLen + 8;
        fPrefix = new XMLCh[fPrefixBufSz + 1];
    }
    XMLString::moveChars(fPrefix, prefix, newLen + 1);
}

void QName::setLocalPart(const XMLCh* localPart)
{
    unsigned int newLen;

    newLen = XMLString::stringLen(localPart);
    if (!fLocalPartBufSz || (newLen > fLocalPartBufSz))
    {
        delete [] fLocalPart;
        fLocalPartBufSz = newLen + 8;
        fLocalPart = new XMLCh[fLocalPartBufSz + 1];
    }
    XMLString::moveChars(fLocalPart, localPart, newLen + 1);
}

inline void QName::setURI(const int uriId)
{
    fURIId = uriId;
}

// -----------------------------------------------------------------------
//  comparison
// -----------------------------------------------------------------------
bool QName::operator==(const QName& qname)
{
return (XMLString::compareString(fPrefix, qname.getPrefix())==0) &&
       (XMLString::compareString(fLocalPart, qname.getLocalPart())==0) &&
       (fURIId == qname.getURI());
}

// -----------------------------------------------------------------------
//  Misc
// -----------------------------------------------------------------------


// ---------------------------------------------------------------------------
//  QName: Private, helper methods
// ---------------------------------------------------------------------------
void QName::cleanUp()
{
    delete [] fLocalPart;
    delete [] fPrefix;
    delete [] fRawName;
}

