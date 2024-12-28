/* --------------------------------------------------------------------------

   libmusicbrainz5 - Client library to access MusicBrainz

   Copyright (C) 2024 Andrew Hawkins, Mike Moreton

   This file is part of libmusicbrainz5.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   libmusicbrainz5 is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library.  If not, see <http://www.gnu.org/licenses/>.

     $Id$

----------------------------------------------------------------------------*/

#include "config.h"
#include "musicbrainz5/defines.h"

#include "musicbrainz5/Iso3166_1CodeList.h"

#include "musicbrainz5/Iso3166_1Code.h"

class MusicBrainz5::CIso3166_1CodeListPrivate
{
	public:
		CIso3166_1CodeListPrivate()
		{
		}

};

MusicBrainz5::CIso3166_1CodeList::CIso3166_1CodeList(const XMLNode& Node)
:	CListImpl<CIso3166_1Code>(),
	m_d(new CIso3166_1CodeListPrivate)
{
	if (!Node.isEmpty())
	{
		//std::cout << "Iso3166_1CodeList node: " << std::endl << Node.createXMLString(true) << std::endl;
		Parse(Node);
	}
}

MusicBrainz5::CIso3166_1CodeList::CIso3166_1CodeList(const CIso3166_1CodeList& Other)
:	CListImpl<CIso3166_1Code>(),
	m_d(new CIso3166_1CodeListPrivate)
{
	*this=Other;
}

MusicBrainz5::CIso3166_1CodeList& MusicBrainz5::CIso3166_1CodeList::operator =(const CIso3166_1CodeList& Other)
{
	if (this!=&Other)
	{
		CListImpl<CIso3166_1Code>::operator =(Other);
	}

	return *this;
}

MusicBrainz5::CIso3166_1CodeList::~CIso3166_1CodeList()
{
	delete m_d;
}

MusicBrainz5::CIso3166_1CodeList *MusicBrainz5::CIso3166_1CodeList::Clone()
{
	return new CIso3166_1CodeList(*this);
}

void MusicBrainz5::CIso3166_1CodeList::ParseAttribute(const std::string& Name, const std::string& Value)
{
	CListImpl<CIso3166_1Code>::ParseAttribute(Name,Value);
}

void MusicBrainz5::CIso3166_1CodeList::ParseElement(const XMLNode& Node)
{

#ifdef MIKE
	std::string NodeName=Node.getName();
	std::cerr << "CIso3166_1CodeList::ParseElement(" << NodeName << ")" << std::endl;
#endif

	CListImpl<CIso3166_1Code>::ParseElement(Node);
}

std::string MusicBrainz5::CIso3166_1CodeList::GetElementName()
{
	return "iso-3166-1-code-list";
}

std::ostream& MusicBrainz5::CIso3166_1CodeList::Serialise(std::ostream& os) const
{
	os << "ISO 3166-1 code list:" << std::endl;

	CListImpl<CIso3166_1Code>::Serialise(os);

	return os;
}
